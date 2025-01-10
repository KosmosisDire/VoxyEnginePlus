#include "clay-ui.hpp"
#include "../elements/element.hpp"
#include "clay-imgui-renderer.hpp"
#include "clay.h"
#include <fonts.hpp>
#include <stdarg.h>
#include <stdio.h>

// Static member definitions
int ClayUI::elementDepth = 0;
Element *ClayUI::currentElement = nullptr;
uint32_t ClayUI::lastHoveredElement = 0;
uint32_t ClayUI::capturedElement = 0;
bool ClayUI::initialized = false;
UIInputs ClayUI::inputs = {};
std::unordered_map<std::string, string_alloc> ClayUI::currentStrings;
std::unordered_map<uint32_t, ComputedProps> ClayUI::computedProperties;
std::vector<Element *> ClayUI::elementStack;
float ClayUI::lastPointerX = 0;
float ClayUI::lastPointerY = 0;
float ClayUI::pointerDeltaX = 0;
float ClayUI::pointerDeltaY = 0;

double ClayUI::uiFrameTime;
std::chrono::time_point<std::chrono::high_resolution_clock> ClayUI::frameStart;

void ClayUI::IndentPrintf(const char *format, ...)
{
    if (!UI_DEBUG)
    {
        return;
    }

    for (size_t i = 0; i < elementDepth; i++)
    {
        printf("|    ");
    }
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    fflush(stdout);
}

Clay_ElementId ClayUI::HashId(const std::string &id)
{
    return Clay__HashString(Clay_String{
                                static_cast<int32_t>(id.length()),
                                ClayUI::AllocateString(id.c_str()),
                            },
                            0, 0);
}

void ClayUI::HandleClayErrors(Clay_ErrorData errorData)
{
    printf("%s", errorData.errorText.chars);
}

void ClayUI::Initialize(UIInputs input)
{
    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, (char *)malloc(totalMemorySize));
    Clay_Initialize(clayMemory,
                    Clay_Dimensions{(float)input.screenWidth, (float)input.screenHeight},
                    Clay_ErrorHandler{HandleClayErrors});
    Clay_SetMeasureTextFunction(Imgui_MeasureText);
    Clay_SetDebugModeEnabled(true);

    Clay_SetFont(FontManager::GetFont("Roboto"));

    lastPointerX = input.pointerX;
    lastPointerY = input.pointerY;

    SetInputs(input);

    initialized = true;
}

void ClayUI::SetInputs(UIInputs input)
{
    inputs = input;
    pointerDeltaX = input.pointerX - lastPointerX;
    pointerDeltaY = input.pointerY - lastPointerY;
    lastPointerX = input.pointerX;
    lastPointerY = input.pointerY;

    Clay_SetLayoutDimensions(Clay_Dimensions{(float)input.screenWidth, (float)input.screenHeight});
    Clay_SetPointerState(Clay_Vector2{input.pointerX, input.pointerY}, input.pointerDown);
    Clay_UpdateScrollContainers(true, Clay_Vector2{input.scrollDeltaX, input.scrollDeltaY * 10}, input.deltaTime);
}

void ClayUI::Render()
{
    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    FillComputedProperties(renderCommands);
    clay_imgui_render(renderCommands);
}

void ClayUI::SetComputedProps(ComputedProps *props, Clay_RenderCommand *command)
{
    auto bb = command->boundingBox;
    props->width = bb.width;
    props->height = bb.height;
    props->x = bb.x;
    props->y = bb.y;
    props->parent = &ClayUI::computedProperties[props->parentId];
}

void ClayUI::FillComputedProperties(Clay_RenderCommandArray commands)
{
    ClayUI::computedProperties.clear();

    for (int i = 0; i < commands.length; ++i)
    {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);
        SetComputedProps(&ClayUI::computedProperties[command->id], command);
    }
}

void ClayUI::BeginFrame(UIInputs input)
{
    if (!initialized)
    {
        Initialize(input);
    }

    FreeStrings();
    SetInputs(input);

    elementStack.clear();
    Clay_BeginLayout();
}

void ClayUI::PushStack(Element &element)
{
    elementDepth++;
    element.props.computed.elementDepth = elementDepth;
    currentElement = &element;
    elementStack.push_back(&element);
}

void ClayUI::PopStack()
{
    elementDepth--;
    elementStack.pop_back();
}

Element *ClayUI::PeekStack()
{
    return elementStack.size() > 0 ? elementStack.back() : nullptr;
}

Element *ClayUI::GetCurrentElement()
{
    return ClayUI::currentElement;
}

uint32_t ClayUI::GetCurrentOpenId()
{
    return Clay__GetOpenLayoutElement()->id;
}

ComputedProps ClayUI::GetComputedProperties(Element &element)
{
    return computedProperties[element.props.clay.clayId.id];
}

float ClayUI::GetPointerDeltaX()
{
    return pointerDeltaX;
}

float ClayUI::GetPointerDeltaY()
{
    return pointerDeltaY;
}

bool ClayUI::IsHovered()
{
    if (Clay__booleanWarnings.maxElementsExceeded)
    {
        return false;
    }

    if (IsOtherCaptured())
        return false;

    uint32_t openElement = ClayUI::GetCurrentElement()->props.clay.clayId.id;

    for (int32_t i = 0; i < Clay__pointerOverIds.length; ++i)
    {
        auto element = Clay__ElementIdArray_Get(&Clay__pointerOverIds, i);
        if (element->id == openElement)
        {
            ClayUI::lastHoveredElement = element->id;
            return true;
        }
    }

    if (ClayUI::lastHoveredElement != 0 && ClayUI::lastHoveredElement == openElement)
    {
        ClayUI::lastHoveredElement = 0;
    }

    return false;
}

bool ClayUI::IsPressed()
{
    return ClayUI::IsHovered() && Clay__pointerInfo.state == CLAY_POINTER_DATA_PRESSED;
}

bool ClayUI::ClickedThisFrame()
{
    return ClayUI::IsHovered() && Clay__pointerInfo.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME;
}

bool ClayUI::HoveredThisFrame()
{
    return ClayUI::IsHovered() && (ClayUI::lastHoveredElement != 0 && ClayUI::lastHoveredElement == ClayUI::GetCurrentElement()->props.clay.clayId.id);
}

bool ClayUI::MouseMovedThisFrame()
{
    return ClayUI::IsHovered() && (ClayUI::GetPointerDeltaX() != 0 || ClayUI::GetPointerDeltaY() != 0);
}

bool ClayUI::MouseScrolledThisFrame()
{
    return ClayUI::IsHovered() && ClayUI::inputs.scrollDeltaX != 0 || ClayUI::inputs.scrollDeltaY != 0;
}

bool ClayUI::IsSelfCaptured()
{
    if (!ClayUI::IsAnyCaptured() || ClayUI::GetCurrentElement() == nullptr)
    {
        return false;
    }

    return ClayUI::capturedElement == ClayUI::GetCurrentElement()->props.clay.clayId.id;
}

bool ClayUI::IsAnyCaptured()
{
    return ClayUI::capturedElement != 0;
}

bool ClayUI::IsOtherCaptured()
{
    return ClayUI::IsAnyCaptured() && !ClayUI::IsSelfCaptured();
}

bool ClayUI::MouseDraggedThisFrame()
{
    bool selfCap = ClayUI::IsSelfCaptured();
    bool pressed = ClayUI::IsPressed();
    bool result = selfCap || (pressed && (ClayUI::GetPointerDeltaX() != 0 || ClayUI::GetPointerDeltaY() != 0));

    // Release capture if mouse is not pressed
    if (selfCap && !ClayUI::inputs.pointerDown)
    {
        ClayUI::capturedElement = 0;
        return false;
    }

    return result;
}

char *ClayUI::AllocateString(const std::string &str)
{
    if (currentStrings.find(str) != currentStrings.end())
    {
        currentStrings[str].access_count += 2;
        return currentStrings[str].str;
    }

    char *newStr = (char *)malloc(str.length() + 1);
    strcpy(newStr, str.c_str());
    currentStrings[str] = {newStr, 10};
    return newStr;
}

void ClayUI::FreeStrings()
{
    for (const auto &[key, value] : currentStrings)
    {
        if (value.access_count < -10)
        {
            free(value.str);
            currentStrings.erase(key);
            break;
        }
        else
        {
            currentStrings[key].access_count--;
        }
    }
}

void ClayUI::FreeAllStrings()
{
    for (const auto &[key, value] : currentStrings)
    {
        free(value.str);
    }
    currentStrings.clear();
}
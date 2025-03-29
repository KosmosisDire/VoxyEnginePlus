#include <angelscript.h>
#include <engine/apis/Input.hpp>

void registerKeyEnum(asIScriptEngine* engine)
{
    engine->RegisterEnum("Key");
    engine->RegisterEnumValue("Key", "Space", GLFW_KEY_SPACE);
    engine->RegisterEnumValue("Key", "Apostrophe", GLFW_KEY_APOSTROPHE);
    engine->RegisterEnumValue("Key", "Comma", GLFW_KEY_COMMA);
    engine->RegisterEnumValue("Key", "Minus", GLFW_KEY_MINUS);
    engine->RegisterEnumValue("Key", "Period", GLFW_KEY_PERIOD);
    engine->RegisterEnumValue("Key", "Slash", GLFW_KEY_SLASH);
    engine->RegisterEnumValue("Key", "Num0", GLFW_KEY_0);
    engine->RegisterEnumValue("Key", "Num1", GLFW_KEY_1);
    engine->RegisterEnumValue("Key", "Num2", GLFW_KEY_2);
    engine->RegisterEnumValue("Key", "Num3", GLFW_KEY_3);
    engine->RegisterEnumValue("Key", "Num4", GLFW_KEY_4);
    engine->RegisterEnumValue("Key", "Num5", GLFW_KEY_5);
    engine->RegisterEnumValue("Key", "Num6", GLFW_KEY_6);
    engine->RegisterEnumValue("Key", "Num7", GLFW_KEY_7);
    engine->RegisterEnumValue("Key", "Num8", GLFW_KEY_8);
    engine->RegisterEnumValue("Key", "Num9", GLFW_KEY_9);
    engine->RegisterEnumValue("Key", "A", GLFW_KEY_A);
    engine->RegisterEnumValue("Key", "B", GLFW_KEY_B);
    engine->RegisterEnumValue("Key", "C", GLFW_KEY_C);
    engine->RegisterEnumValue("Key", "D", GLFW_KEY_D);
    engine->RegisterEnumValue("Key", "E", GLFW_KEY_E);
    engine->RegisterEnumValue("Key", "F", GLFW_KEY_F);
    engine->RegisterEnumValue("Key", "G", GLFW_KEY_G);
    engine->RegisterEnumValue("Key", "H", GLFW_KEY_H);
    engine->RegisterEnumValue("Key", "I", GLFW_KEY_I);
    engine->RegisterEnumValue("Key", "J", GLFW_KEY_J);
    engine->RegisterEnumValue("Key", "K", GLFW_KEY_K);
    engine->RegisterEnumValue("Key", "L", GLFW_KEY_L);
    engine->RegisterEnumValue("Key", "M", GLFW_KEY_M);
    engine->RegisterEnumValue("Key", "N", GLFW_KEY_N);
    engine->RegisterEnumValue("Key", "O", GLFW_KEY_O);
    engine->RegisterEnumValue("Key", "P", GLFW_KEY_P);
    engine->RegisterEnumValue("Key", "Q", GLFW_KEY_Q);
    engine->RegisterEnumValue("Key", "R", GLFW_KEY_R);
    engine->RegisterEnumValue("Key", "S", GLFW_KEY_S);
    engine->RegisterEnumValue("Key", "T", GLFW_KEY_T);
    engine->RegisterEnumValue("Key", "U", GLFW_KEY_U);
    engine->RegisterEnumValue("Key", "V", GLFW_KEY_V);
    engine->RegisterEnumValue("Key", "W", GLFW_KEY_W);
    engine->RegisterEnumValue("Key", "X", GLFW_KEY_X);
    engine->RegisterEnumValue("Key", "Y", GLFW_KEY_Y);
    engine->RegisterEnumValue("Key", "Z", GLFW_KEY_Z);
    engine->RegisterEnumValue("Key", "F1", GLFW_KEY_F1);
    engine->RegisterEnumValue("Key", "F2", GLFW_KEY_F2);
    engine->RegisterEnumValue("Key", "F3", GLFW_KEY_F3);
    engine->RegisterEnumValue("Key", "F4", GLFW_KEY_F4);
    engine->RegisterEnumValue("Key", "F5", GLFW_KEY_F5);
    engine->RegisterEnumValue("Key", "F6", GLFW_KEY_F6);
    engine->RegisterEnumValue("Key", "F7", GLFW_KEY_F7);
    engine->RegisterEnumValue("Key", "F8", GLFW_KEY_F8);
    engine->RegisterEnumValue("Key", "F9", GLFW_KEY_F9);
    engine->RegisterEnumValue("Key", "F10", GLFW_KEY_F10);
    engine->RegisterEnumValue("Key", "F11", GLFW_KEY_F11);
    engine->RegisterEnumValue("Key", "F12", GLFW_KEY_F12);
    engine->RegisterEnumValue("Key", "Escape", GLFW_KEY_ESCAPE);
    engine->RegisterEnumValue("Key", "Enter", GLFW_KEY_ENTER);
    engine->RegisterEnumValue("Key", "Tab", GLFW_KEY_TAB);
    engine->RegisterEnumValue("Key", "Backspace", GLFW_KEY_BACKSPACE);
    engine->RegisterEnumValue("Key", "Insert", GLFW_KEY_INSERT);
    engine->RegisterEnumValue("Key", "Delete", GLFW_KEY_DELETE);
    engine->RegisterEnumValue("Key", "Right", GLFW_KEY_RIGHT);
    engine->RegisterEnumValue("Key", "Left", GLFW_KEY_LEFT);
    engine->RegisterEnumValue("Key", "Down", GLFW_KEY_DOWN);
    engine->RegisterEnumValue("Key", "Up", GLFW_KEY_UP);
    engine->RegisterEnumValue("Key", "PageUp", GLFW_KEY_PAGE_UP);
    engine->RegisterEnumValue("Key", "PageDown", GLFW_KEY_PAGE_DOWN);
    engine->RegisterEnumValue("Key", "Home", GLFW_KEY_HOME);
    engine->RegisterEnumValue("Key", "End", GLFW_KEY_END);
    engine->RegisterEnumValue("Key", "CapsLock", GLFW_KEY_CAPS_LOCK);
    engine->RegisterEnumValue("Key", "ScrollLock", GLFW_KEY_SCROLL_LOCK);
    engine->RegisterEnumValue("Key", "NumLock", GLFW_KEY_NUM_LOCK);
    engine->RegisterEnumValue("Key", "PrintScreen", GLFW_KEY_PRINT_SCREEN);
    engine->RegisterEnumValue("Key", "Pause", GLFW_KEY_PAUSE);
    engine->RegisterEnumValue("Key", "LeftShift", GLFW_KEY_LEFT_SHIFT);
    engine->RegisterEnumValue("Key", "LeftControl", GLFW_KEY_LEFT_CONTROL);
    engine->RegisterEnumValue("Key", "LeftAlt", GLFW_KEY_LEFT_ALT);
    engine->RegisterEnumValue("Key", "LeftSuper", GLFW_KEY_LEFT_SUPER);
    engine->RegisterEnumValue("Key", "RightShift", GLFW_KEY_RIGHT_SHIFT);
    engine->RegisterEnumValue("Key", "RightControl", GLFW_KEY_RIGHT_CONTROL);
    engine->RegisterEnumValue("Key", "RightAlt", GLFW_KEY_RIGHT_ALT);
    engine->RegisterEnumValue("Key", "RightSuper", GLFW_KEY_RIGHT_SUPER);
    engine->RegisterEnumValue("Key", "Menu", GLFW_KEY_MENU);
}

void registerMouseButtonEnum(asIScriptEngine* engine)
{
    engine->RegisterEnum("MouseButton");
    engine->RegisterEnumValue("MouseButton", "Left", GLFW_MOUSE_BUTTON_LEFT);
    engine->RegisterEnumValue("MouseButton", "Right", GLFW_MOUSE_BUTTON_RIGHT);
    engine->RegisterEnumValue("MouseButton", "Middle", GLFW_MOUSE_BUTTON_MIDDLE);
}

void registerInputFunctions(asIScriptEngine* engine)
{
    engine->SetDefaultNamespace("Input");
    engine->RegisterGlobalFunction("bool IsKeyPressed(Key key)", asFUNCTION(Input::IsKeyPressed), asCALL_CDECL);
    engine->RegisterGlobalFunction("bool WasKeyPressed(Key key)", asFUNCTION(Input::WasKeyPressed), asCALL_CDECL);
    engine->RegisterGlobalFunction("bool WasKeyReleased(Key key)", asFUNCTION(Input::WasKeyReleased), asCALL_CDECL);
    engine->RegisterGlobalFunction("bool IsMouseButtonPressed(MouseButton button)", asFUNCTION(Input::IsMouseButtonPressed), asCALL_CDECL);
    engine->RegisterGlobalFunction("bool WasMouseButtonPressed(MouseButton button)", asFUNCTION(Input::WasMouseButtonPressed), asCALL_CDECL);
    engine->RegisterGlobalFunction("bool WasMouseButtonReleased(MouseButton button)", asFUNCTION(Input::WasMouseButtonReleased), asCALL_CDECL);
    engine->RegisterGlobalFunction("Vector2 GetMousePosition()", asFUNCTION(Input::GetMousePosition), asCALL_CDECL);
    engine->RegisterGlobalFunction("Vector2 GetMouseDelta()", asFUNCTION(Input::GetMouseDelta), asCALL_CDECL);
    engine->RegisterGlobalFunction("Vector2 GetMouseScrollDelta()", asFUNCTION(Input::GetMouseScrollDelta), asCALL_CDECL);

    engine->RegisterGlobalFunction("void SetMousePosition(float x, float y)", asFUNCTION(Input::SetMousePosition), asCALL_CDECL);
    engine->RegisterGlobalFunction("void ResetMouseDelta()", asFUNCTION(Input::ResetMouseDelta), asCALL_CDECL);
    engine->RegisterGlobalFunction("void CaptureMouse(bool capture)", asFUNCTION(Input::CaptureMouse), asCALL_CDECL);
    engine->RegisterGlobalFunction("void CaptureMouseResetDelta(bool capture)", asFUNCTION(Input::CaptureMouseResetDelta), asCALL_CDECL);
    engine->RegisterGlobalFunction("bool IsMouseCaptured()", asFUNCTION(Input::IsMouseCaptured), asCALL_CDECL);



    engine->SetDefaultNamespace("");
}
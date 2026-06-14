#pragma once

#include <clay.h>
#include <cmath>
#include <imgui_impl_glfw.h>
#include <stdio.h>
#include <string>

ImFont *clay_font = nullptr;
void Clay_SetFont(ImFont *font)
{
    clay_font = font;
}

#define IM_PI 3.14159265358979323846f

ImColor clayToImGuiColor(Clay_Color color)
{
    return ImColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

static inline Clay_Dimensions Imgui_MeasureText(Clay_String *text, Clay_TextElementConfig *config)
{
    Clay_Dimensions textSize = {0};

    ImFont *font = clay_font;
    IM_ASSERT(font != NULL);
    float scale = config->fontSize / font->FontSize;

    ImVec2 textSizeVec = font->CalcTextSizeA(config->fontSize, FLT_MAX, 0.0f, text->chars, text->chars + text->length);
    textSize.width = textSizeVec.x * scale;
    textSize.height = textSizeVec.y * scale;

    return textSize;
}

void draw_clay(Clay_RenderCommand &command, ImDrawList *drawList)
{
    Clay_BoundingBox boundingBox = command.boundingBox;

    switch (command.commandType)
    {
    case CLAY_RENDER_COMMAND_TYPE_TEXT:
    {
        Clay_String text = command.text;
        Clay_TextElementConfig *config = command.config.textElementConfig;

        ImColor textColor = clayToImGuiColor(config->textColor);
        float fontSize = static_cast<float>(config->fontSize);

        drawList->AddText(
            clay_font,
            config->fontSize,
            ImVec2(boundingBox.x, boundingBox.y),
            textColor,
            text.chars,
            text.chars + text.length,
            command.boundingBox.width + 1);
        break;
    }

    case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
    {
        Clay_RectangleElementConfig *config = command.config.rectangleElementConfig;
        ImColor color = clayToImGuiColor(config->color);

        // If all corners have same radius
        if (config->cornerRadius.topLeft == config->cornerRadius.topRight &&
            config->cornerRadius.topLeft == config->cornerRadius.bottomLeft &&
            config->cornerRadius.topLeft == config->cornerRadius.bottomRight)
        {
            drawList->AddRectFilled(
                ImVec2(boundingBox.x, boundingBox.y),
                ImVec2(boundingBox.x + boundingBox.width, boundingBox.y + boundingBox.height),
                color,
                config->cornerRadius.topLeft);
        }
        else
        {
            // Draw rectangle with different corner radii using paths
            drawList->PathClear();

            // Top-left corner
            if (config->cornerRadius.topLeft > 0)
            {
                drawList->PathArcTo(
                    ImVec2(boundingBox.x + config->cornerRadius.topLeft,
                           boundingBox.y + config->cornerRadius.topLeft),
                    config->cornerRadius.topLeft,
                    IM_PI,
                    IM_PI * 1.5f);
            }
            else
            {
                drawList->PathLineTo(ImVec2(boundingBox.x, boundingBox.y));
            }

            // Top-right corner
            if (config->cornerRadius.topRight > 0)
            {
                drawList->PathArcTo(
                    ImVec2(boundingBox.x + boundingBox.width - config->cornerRadius.topRight,
                           boundingBox.y + config->cornerRadius.topRight),
                    config->cornerRadius.topRight,
                    IM_PI * 1.5f,
                    IM_PI * 2.0f);
            }
            else
            {
                drawList->PathLineTo(ImVec2(boundingBox.x + boundingBox.width, boundingBox.y));
            }

            // Bottom-right corner
            if (config->cornerRadius.bottomRight > 0)
            {
                drawList->PathArcTo(
                    ImVec2(boundingBox.x + boundingBox.width - config->cornerRadius.bottomRight,
                           boundingBox.y + boundingBox.height - config->cornerRadius.bottomRight),
                    config->cornerRadius.bottomRight,
                    0.0f,
                    IM_PI * 0.5f);
            }
            else
            {
                drawList->PathLineTo(ImVec2(boundingBox.x + boundingBox.width, boundingBox.y + boundingBox.height));
            }

            // Bottom-left corner
            if (config->cornerRadius.bottomLeft > 0)
            {
                drawList->PathArcTo(
                    ImVec2(boundingBox.x + config->cornerRadius.bottomLeft,
                           boundingBox.y + boundingBox.height - config->cornerRadius.bottomLeft),
                    config->cornerRadius.bottomLeft,
                    IM_PI * 0.5f,
                    IM_PI);
            }
            else
            {
                drawList->PathLineTo(ImVec2(boundingBox.x, boundingBox.y + boundingBox.height));
            }

            drawList->PathFillConvex(color);
        }
        break;
    }

    case CLAY_RENDER_COMMAND_TYPE_BORDER:
    {
        Clay_BorderElementConfig *config = command.config.borderElementConfig;

        // Check if all borders are identical
        bool uniformBorder =
            config->left.width == config->right.width &&
            config->left.width == config->top.width &&
            config->left.width == config->bottom.width &&
            config->left.color.r == config->right.color.r &&
            config->left.color.g == config->right.color.g &&
            config->left.color.b == config->right.color.b &&
            config->left.color.a == config->right.color.a &&
            config->left.color.r == config->top.color.r &&
            config->left.color.g == config->top.color.g &&
            config->left.color.b == config->top.color.b &&
            config->left.color.a == config->top.color.a &&
            config->left.color.r == config->bottom.color.r &&
            config->left.color.g == config->bottom.color.g &&
            config->left.color.b == config->bottom.color.b &&
            config->left.color.a == config->bottom.color.a;

        // Snap coordinates to pixels to avoid blurry lines
        float x = std::round(boundingBox.x);
        float y = std::round(boundingBox.y);
        float w = std::round(boundingBox.width);
        float h = std::round(boundingBox.height);

        if (uniformBorder && config->left.width > 0)
        {
            // Optimized path for uniform borders
            ImColor borderColor = clayToImGuiColor(config->left.color);
            float borderWidth = std::round(config->left.width); // Round border width to nearest pixel

            // Add small offset to ensure pixel-perfect alignment
            float offset = borderWidth * 0.5f;

            drawList->PathClear();

            // Start at the beginning of the top edge
            drawList->PathLineTo(ImVec2(x + config->cornerRadius.topLeft, y + offset));

            // Top-right corner
            if (config->cornerRadius.topRight > 0)
            {
                drawList->PathArcTo(
                    ImVec2(x + w - config->cornerRadius.topRight,
                           y + config->cornerRadius.topRight),
                    config->cornerRadius.topRight - offset,
                    -IM_PI * 0.5f,
                    0.0f,
                    12); // Increase segment count for smoother corners
            }

            // Right edge
            drawList->PathLineTo(ImVec2(x + w - offset, y + h - config->cornerRadius.bottomRight - offset));

            // Bottom-right corner
            if (config->cornerRadius.bottomRight > 0)
            {
                drawList->PathArcTo(
                    ImVec2(x + w - config->cornerRadius.bottomRight,
                           y + h - config->cornerRadius.bottomRight),
                    config->cornerRadius.bottomRight - offset,
                    0.0f,
                    IM_PI * 0.5f,
                    12);
            }

            // Bottom edge
            drawList->PathLineTo(ImVec2(x + config->cornerRadius.bottomLeft + offset, y + h - offset));

            // Bottom-left corner
            if (config->cornerRadius.bottomLeft > 0)
            {
                drawList->PathArcTo(
                    ImVec2(x + config->cornerRadius.bottomLeft,
                           y + h - config->cornerRadius.bottomLeft),
                    config->cornerRadius.bottomLeft - offset,
                    IM_PI * 0.5f,
                    IM_PI,
                    12);
            }

            // Left edge as a perfectly vertical line
            drawList->PathLineTo(ImVec2(x + offset, y + h - config->cornerRadius.bottomLeft - offset));

            // Top-left corner
            if (config->cornerRadius.topLeft > 0)
            {
                drawList->PathArcTo(
                    ImVec2(x + config->cornerRadius.topLeft,
                           y + config->cornerRadius.topLeft),
                    config->cornerRadius.topLeft - offset,
                    IM_PI,
                    IM_PI * 1.5f,
                    12);
            }

            // Close the path by going back to the starting point
            drawList->PathStroke(borderColor, false, borderWidth);
        }
        else
        {
            // Seperate border drawing is not yet implemented
        }
        break;
    }

    case CLAY_RENDER_COMMAND_TYPE_IMAGE:
    {
        Clay_ImageElementConfig *config = command.config.imageElementConfig;
        ImTextureID textureId = (ImTextureID)(intptr_t)config->imageData;

        if (textureId)
        {
            drawList->AddImage(
                textureId,
                ImVec2(boundingBox.x, boundingBox.y),
                ImVec2(boundingBox.x + boundingBox.width, boundingBox.y + boundingBox.height));
        }
        break;
    }

    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
    {
        drawList->PushClipRect(
            ImVec2(boundingBox.x, boundingBox.y),
            ImVec2(boundingBox.x + boundingBox.width, boundingBox.y + boundingBox.height),
            true);
        break;
    }

    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
    {
        drawList->PopClipRect();
        break;
    }
    }
}

void clay_imgui_render(Clay_RenderCommandArray renderCommands)
{
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::PushFont(clay_font);

    // ImGui::ShowFontSelector("Font Selector");

    ImDrawList *drawList = ImGui::GetForegroundDrawList();

    for (int j = 0; j < renderCommands.length; j++)
    {
        Clay_RenderCommand *renderCommand = Clay_RenderCommandArray_Get(&renderCommands, j);
        draw_clay(*renderCommand, drawList);
    }

    ImGui::PopFont();

    ImGui::EndFrame();
    ImGui::Render();
}
#include "fonts.hpp"
#include <stdio.h>

// Define the static member variable
std::unordered_map<string, ImFont *> FontManager::fonts;

void FontManager::LoadFont(string fontPath, string fontName, float fontSize, ImFontConfig *config)
{
    auto io = ImGui::GetIO();
    ImFont *font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize, config, io.Fonts->GetGlyphRangesDefault());

    if (font == NULL)
    {
        fprintf(stderr, "Failed to load font %s\n", fontName.c_str());
        IM_ASSERT(font != NULL);
    }

    fonts[fontName] = font;
}

ImFont *FontManager::GetFont(string fontName)
{
    auto font = FontManager::fonts[fontName];

    if (font == NULL)
    {
        fprintf(stderr, "Font %s not found\n", fontName.c_str());
        IM_ASSERT(font != NULL);
    }

    return font;
}

void FontManager::LoadDefaults()
{
    ImFontConfig font_config;
    font_config.OversampleH = 2;
    font_config.OversampleV = 2;

    FontManager::LoadFont("resources/Roboto-Regular.ttf", "Roboto", 16.0f, &font_config);
}
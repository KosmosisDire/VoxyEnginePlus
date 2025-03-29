#pragma once
#include <imgui.h>
#include <string>
#include <unordered_map>
#include "resources.hpp"
#include <stdio.h>
#include <filesystem>
#include <iostream>

using namespace std;

// a static class which loads ImGui fonts for access later
struct FontManager
{
    public:
        inline static void LoadFont(string fontPath, string fontName, float fontSize, ImFontConfig *config)
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

        inline static ImFont *GetFont(string fontName)
        {
            auto font = FontManager::fonts[fontName];

            if (font == NULL)
            {
                fprintf(stderr, "Font %s not found\n", fontName.c_str());
                IM_ASSERT(font != NULL);
            }

            return font;
        }

        inline static void LoadDefaults()
        {
            ImFontConfig font_config;
            font_config.OversampleH = 2;
            font_config.OversampleV = 2;

            // Get and print current working directory
            auto fontPath = Resources::GetResourcePath("fonts/Roboto-Regular.ttf").string();
            FontManager::LoadFont(fontPath, "Roboto", 16.0f, &font_config);
        }

    private:
        inline static std::unordered_map<string, ImFont *> fonts;
};
#pragma once
#include <imgui.h>
#include <string>
#include <unordered_map>
#include "Resources.hpp"
#include <stdio.h>
#include <filesystem>

using namespace std;

// a static class which loads ImGui fonts for access later
struct FontManager
{
    public:
        inline static void load_font(string fontPath, string fontName, float fontSize, ImFontConfig *config)
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

        inline static ImFont *get_font(string fontName)
        {
            auto font = FontManager::fonts[fontName];

            if (font == NULL)
            {
                fprintf(stderr, "Font %s not found\n", fontName.c_str());
                IM_ASSERT(font != NULL);
            }

            return font;
        }

        inline static void load_defaults()
        {
            ImFontConfig font_config;
            font_config.OversampleH = 2;
            font_config.OversampleV = 2;

            // Get and print current working directory
            auto fontPath = Resources::get_resource_path("fonts/Roboto-Regular.ttf").string(); // Keep Resources::GetResourcePath for now
            FontManager::load_font(fontPath, "Roboto", 16.0f, &font_config); // Update call
        }

    private:
        inline static std::unordered_map<string, ImFont *> fonts;
};

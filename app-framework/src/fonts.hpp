#pragma once
#include <imgui.h>
#include <string>
#include <unordered_map>

using namespace std;

// a static class which loads ImGui fonts for access later
struct FontManager
{
  public:
    static void LoadFont(string fontPath, string fontName, float fontSize, ImFontConfig *config = NULL);
    static ImFont *GetFont(string fontName);
    static void LoadDefaults();

  private:
    static std::unordered_map<string, ImFont *> fonts;
};
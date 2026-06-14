#pragma once
#include "helper/clay-types.hpp"        
#include "helper/clay-state.hpp"  

struct Text
{
  private:
    static int textId;
    ElementProps props;

    // use the static properties from Element to track state

  public:
    Text(const std::string &text);
    Text &color(const Color &color);
    Text &color(const ThemeColor imguiColor);
    Text &fontSize(float size);
    void _Begin();
};

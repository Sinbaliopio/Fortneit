#pragma once
#include "imgui.h"
#include <memory>

class c_colors
{
public:
	ImColor accent{ 103, 100, 255 };
    struct
    {
        ImColor background{ 25, 25, 32 };
        ImColor bar{ 17, 17, 22 };
        ImColor child{ 20, 20, 27 };
        ImColor rect{ 25,25,33, };
        ImColor line{ 54,55,66 };
    } window;
    struct
    {
        ImColor text{ 255, 255, 255 };
        ImColor text_inactive{ 86,85,106 };
        ImColor section_on{ 77,79,95 };
        ImColor widgets_active{ 35,35,46 };

        ImColor background_color{ 23,23,28 };
        ImColor background_color_line{ 29,29,37 };

        ImColor widgets_inactive{ 47,47,63 };
        ImColor widgets_background{ 28,28,39 };

        ImColor hex_field{ 27,27,33 };
        ImColor black{ 0,0,0 };
        ImColor reb{ 255,122,124 };

    } widgets;
};

inline std::unique_ptr<c_colors> clr = std::make_unique<c_colors>();

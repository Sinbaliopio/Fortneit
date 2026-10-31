#pragma once
#include <string>
#include "imgui.h"

class c_elements
{
public:
	struct
	{
		ImVec2 size{ 65,530 };
		float size_rect{ 35 };
		float distance_rect{ 81 };
		ImVec2 padding{ 15, 15 };
		ImVec2 spacing{ 10, 10 };
		float rounding{ 4 };
	}section;
	struct
	{
		ImVec2 size{ 840,80 };
		float padding{ 20 };
		float margin{ 20 };
	}bar;
	struct
	{
		ImVec2 size{ 730,430 };
		float padding{ 10 };
		float margin{ 20 };
		float name{ 24 };
		float rounding{ 8 };
	}child;

	struct
	{
		float size{ 40 };
		ImVec2 size_rect{ 30,20 };
		float rounding{ 100 };
		float indent{ 10 };
		float size_circle{ 14 };
		float indent_circle{ 3 };
	}checkbox;
	struct
	{
		float size{ 40 };
		ImVec2 size_rect{ 160,6 };
		float indent{ 17 };
		ImVec2 indent_text{ 175,15 };
		float rounding{ 100 };
		float size_circle{ 12 };
	}slider;
	struct
	{
		float size{ 45 };
		float size_rect{ 160 };
		float indent{ 10 };
		float indent_text{ 24 };
		float rounding{ 8 };
		float rounding_text{ 4 };
		float window_padding{ 8 };
		float size_opening_window{ 24 };
		float padding_text{ 17 };
		float padding{ 15 };

	}dropdown;
	struct
	{
		ImVec2 size{ 161,40 };
		ImVec2 rect_icon{ 40,40 };
		float rounding{ 8 };
		float indent{ 15 };
		ImVec2 settings_size{ 35, 35 };
		ImVec2 settings_size_opened{ 30, 30 };

	}button;
	struct
	{
		ImVec2 size{ 68,40 };
		float rounding{ 8 };
	}button_page;

	struct
	{
		float rounding{ 8 };

		ImVec2 size_opened_off{ 40,40 };
		float size_opened_no{ 300 };
	}search;
	struct
	{
		float size{ 10 };
		float size_circle{ 24 };

		float rounding{ 6 };
		float rounding_bar{ 1000 };
		float rounding_color{ 4 };
		float circle_top{ 12 };
		float rect_circle_top{ 138 };
		float width{ 160 };
		ImVec2 padding{ 10, 10 };
		ImVec2 spacing{ 10, 10 };
		float size_circle_mini{ 10 };
		float bar_height{ 5 };
		float bar_grab{ 8 };
		float button_size{ 27 };

		float indent{ 10 };
		float size_circle_rect{ 10 };
		float padding_text{ 15 };

	}color_edit;
	struct
	{
		float indent{ 15 };
		float indent_text{ 10 };

		float size_rect{ 20 };
		float rounding_window{ 4 };
		ImVec2 padding{ 15, 15 };
		ImVec2 spacing{ 10, 15 };
		ImVec2 size_window{ 170,159 };

		ImVec2 key_select_size{ 140,27 };
		ImVec2 mode_select_size{ 140,27 };
		float rounding_widgets{ 4 };

		ImVec2 checbox_keybind_size{ 140,15 };
		float size_rect_chec{ 25 };
		float rounding_checbox{ 100 };
		float size_circle{ 9 };
		float indent_circle{ 3 };

		ImVec2 main_size{ 160, 96 };
		float size_main_widgets{ 24 };

		float rect_plus{ 10 };
		ImVec2 padding_main{ 8, 9 };
		ImVec2 spacing_main{ 10, 3 };
		float padding_main_icon{ 7 };
		float padding_main_name{ 34 };

	}keybind;
	struct
	{
		float height{ 50 };
		float size{ 30 };
		float rounding{ 4 };
		float padding{ 12 };
		float line_padding{ 8 };
	} textfield;

};

inline std::unique_ptr<c_elements> elements = std::make_unique<c_elements>();

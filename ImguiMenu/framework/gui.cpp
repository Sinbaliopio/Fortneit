#include "headers/includes.h"
#include "headers/blur.h"

void c_gui::render()
{
	gui->initialize();
	gui->set_next_window_size(SCALE(var->window.size));
	gui->begin("menu", nullptr, var->window.flags);
	{
		gui->set_style();
		gui->draw_decorations();
		{

			gui->set_pos(SCALE(elements->section.distance_rect), pos_y);
			gui->begin_content("section", SCALE(elements->section.size), SCALE(elements->section.padding), SCALE(elements->section.spacing), window_flags_no_move);
			{
				for (int i = 0; i < var->gui.sections.size(); i++)
				{
					widgets->section(var->gui.sections[i].text, var->gui.sections[i].icon, i, var->gui.section_index);
				}
			}
			gui->end_content();

			gui->set_pos(ImVec2(SCALE(elements->section.padding.x), gui->window_height() - SCALE(elements->section.padding.y + elements->button.settings_size.y)), pos_all);
			if (widgets->settings_button(var->gui.image_texture).value_changed)
				var->gui.settings_opened = true;

			gui->easing(var->gui.settings_alpha, var->gui.settings_opened ? 1.f : 0.f, 8.f, static_easing);

			if (var->gui.settings_alpha > 0.f)
			{
				gui->set_next_window_pos(gui->window_pos() + ImVec2(SCALE(elements->section.padding.x), gui->window_height() - SCALE(elements->section.padding.y + elements->button.settings_size.y)));
				gui->set_next_window_size(SCALE(var->gui.settings_size));
				gui->push_var(style_var_alpha, var->gui.settings_alpha);
				gui->push_var(style_var_window_padding, SCALE(0, 0));
				gui->push_var(style_var_item_spacing, SCALE(0, 1));
				gui->begin("settings_window", nullptr, window_flags_tooltip | window_flags_no_decoration | window_flags_no_scrollbar | window_flags_no_scroll_with_mouse | window_flags_no_background | window_flags_no_nav);
				{
					var->gui.settings_hovered = gui->get_window()->Rect().Contains(gui->mouse_pos()) || (GImGui->HoveredWindow && (strstr(GImGui->HoveredWindow->Name, "coloredit_window") || strstr(GImGui->HoveredWindow->Name, "dropdown_window")));
					if (!var->gui.settings_hovered && gui->mouse_clicked(mouse_button_left))
						var->gui.settings_opened = false;

					gui->begin_content("blur_s", SCALE(var->gui.settings_size));
					{
						draw_background_blur(gui->window_drawlist(), var->device, var->context, SCALE(var->window.rounding));
						draw->rect_filled(gui->window_drawlist(), gui->window_pos(), gui->window_pos() + gui->window_size(), draw->get_clr(clr->window.background, 0.9f), SCALE(var->window.rounding));
					}
					gui->end_content();

					gui->set_pos(ImVec2(0, 0), pos_all);
					gui->begin_content("top", ImVec2(0, SCALE(var->gui.settings_top)));
					{
						draw->image_rounded(gui->window_drawlist(), var->gui.image_texture, gui->window_pos() + SCALE(elements->section.padding), gui->window_pos() + SCALE(elements->section.padding + elements->button.settings_size_opened), ImVec2(0, 0), ImVec2(1, 1), draw->get_clr({ 1.f, 1.f, 1.f, 1.f }), SCALE(elements->button.settings_size_opened.x / 2));
						draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 14), gui->window_pos() + SCALE(elements->section.padding.x * 2 + elements->button.settings_size_opened.x, elements->section.padding.y), gui->window_pos() + gui->window_size(), draw->get_clr(clr->widgets.text), "Past Owl", NULL, NULL, ImVec2(0.f, 0.f));
						draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 12), gui->window_pos() + SCALE(elements->section.padding.x * 2 + elements->button.settings_size_opened.x, 0), gui->window_pos() + ImVec2(gui->window_width(), gui->window_height() - SCALE(elements->section.padding.y)), draw->get_clr(clr->widgets.text_inactive), "Till: 1 Jan 2025", NULL, NULL, ImVec2(0.f, 1.f));
					}
					gui->end_content();

					gui->begin_content("bottom", ImVec2(0, 0), SCALE(elements->section.padding), SCALE(elements->section.padding), window_flags_no_scrollbar | window_flags_no_scroll_with_mouse);
					{
						widgets->dropdown_settings("Language");

						static bool changed;
						if (widgets->dropdown_settings("DPI Menu").value_changed)
							changed = true;
						if (changed)
						{
							if (cfg->get<settings_dropdown_t>("DPI Menu").callback == 0)
								var->gui.stored_dpi = 75;
							else if (cfg->get<settings_dropdown_t>("DPI Menu").callback == 1)
								var->gui.stored_dpi = 100;
							else if (cfg->get<settings_dropdown_t>("DPI Menu").callback == 2)
								var->gui.stored_dpi = 150;
							else if (cfg->get<settings_dropdown_t>("DPI Menu").callback == 3)
								var->gui.stored_dpi = 200;

							var->gui.dpi_changed = true;
							changed = false;
						}
						static bool init_clr{ false };
						if (!init_clr)
						{
							cfg->get<color_edit_t>("Styles").color[0] = clr->accent.Value.x;
							cfg->get<color_edit_t>("Styles").color[1] = clr->accent.Value.y;
							cfg->get<color_edit_t>("Styles").color[2] = clr->accent.Value.z;

							init_clr = true;
						}

						widgets->color_edit("Styles");
						clr->accent.Value.x = cfg->get<color_edit_t>("Styles").color[0];
						clr->accent.Value.y = cfg->get<color_edit_t>("Styles").color[1];
						clr->accent.Value.z = cfg->get<color_edit_t>("Styles").color[2];

					}
					gui->end_content();

					draw->line(gui->window_drawlist(), gui->window_pos() + SCALE(elements->section.padding.x, var->gui.settings_top), gui->window_pos() + ImVec2(gui->window_width() - SCALE(elements->section.padding.x), SCALE(var->gui.settings_top)), draw->get_clr(clr->widgets.widgets_active), SCALE(1));
				}
				gui->end();
				gui->pop_var(3);
			}
		}

		{
			gui->set_pos(SCALE(elements->section.size.x, 0), pos_all);
			gui->begin_content("bar", SCALE(elements->bar.size), SCALE(elements->bar.margin, elements->bar.margin), SCALE(elements->bar.padding, elements->bar.padding));
			{
				widgets->search_field("search", search->search_buf, sizeof search->search_buf);
				search->search_element = search->search_buf;

				gui->sameline();

				static int selected_cfg;
				std::vector<std::string> items{ "New Config 1", "New Config 2", "New Config 3" };
				widgets->dropdown_cfg("drpdwn_cfg", &selected_cfg, items);
			}
			gui->end_content();
		}
		{

			gui->set_pos(SCALE(65, 80), pos_all);
			gui->begin_content("child", SCALE(0, 0), SCALE(elements->child.margin, 0), SCALE(elements->child.margin, elements->child.margin), window_flags_no_scrollbar);
			{
				if (strlen(search->search_buf) > 0)
				{
					widgets->begin_child("SEARCH");
					{
						search->search();
					}
					widgets->end_child();
				}
				else
				{

					gui->begin_group();
					{
						widgets->begin_child("AIMBOT");
						{
							widgets->checkbox("Enable aimbot");
							widgets->dropdown("Conditions");
							widgets->multi_dropdown("Bone aimbot");
							widgets->multi_dropdown("Hitboxes");
							widgets->slider_int("Field of view");
							widgets->slider_int("Smoothing");
							widgets->slider_int("Reaction time");
							widgets->slider_int("Target switch delay");
							widgets->checkbox("First bullet delay");
							widgets->checkbox("Recoil control");

						}
						widgets->end_child();
					}
					gui->end_group();
					gui->sameline();
					gui->begin_group();
					{
						widgets->begin_child("TRIGGERBOT");
						{
							widgets->checkbox("Enable triggerbot");
							widgets->dropdown("Conditions##1");
							widgets->slider_int("Hit change");
							widgets->slider_int("Reaction time##1");
							widgets->slider_int("Burst time");
							widgets->checkbox("Quick scope");
						}
						widgets->end_child();

					}
					gui->end_group();
				}

				gui->dummy(ImVec2(0, 0));
			}
		}
		gui->end_content();
	}
	gui->end();
}
#include "../headers/functions.h"
#include "../headers/widgets.h"

widget_t checkbox_ex(std::string_view name, bool* callback, bool keybind, std::vector<key_data_t>* state)
{
    struct anim_t
    {
        float alpha;
        float offset;
        ImVec4 off_on_text{ clr->widgets.text };
        ImVec4 off_on_circle{ clr->accent };
        ImVec4 keyclr{ clr->widgets.text_inactive };

        float keybind_alpha{ 0 };
        bool keybind_hovered{ false };
        bool keybind_opened{ false };
        int key;
        int mode;
        bool value;
        bool show;
        int key_index{ -1 };

        float alpha_2;
        float alpha_3;

    };

    widget_t data = gui->register_item(name, ImVec2(gui->content_avail().x, SCALE(elements->checkbox.size)));
    anim_t* anim = gui->anim_container<anim_t>(data->id);

    const ImRect rect{ data->rect.GetTR() + SCALE(0,elements->checkbox.indent) - SCALE(elements->checkbox.size_rect.x,0), data->rect.Max - SCALE(0,elements->checkbox.indent) };
    const ImRect key_rect{ rect.GetTL() - SCALE(elements->keybind.indent + elements->keybind.size_rect,0), rect.GetBL() - SCALE(elements->keybind.indent,0) };

    data->value_changed = data->hovered && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0) && !key_rect.Contains(gui->mouse_pos());

    if (data->value_changed)
        *callback = !*callback;
    gui->easing(anim->alpha, *callback ? 1.f : 0.f, 8.f, static_easing);
    gui->easing(anim->alpha_2, anim->key_index == 0 ? 1.f : 0.f, 8.f, static_easing);
    gui->easing(anim->alpha_3, anim->key_index == 1 ? 1.f : 0.f, 8.f, static_easing);

    gui->easing(anim->off_on_text, *callback ? clr->widgets.text.Value : clr->widgets.text_inactive.Value, 16.f, dynamic_easing);
    gui->easing(anim->off_on_circle, *callback ? clr->accent.Value : clr->widgets.widgets_inactive.Value, 16.f, dynamic_easing);
    gui->easing(anim->offset, *callback ? rect.GetWidth() - SCALE(elements->checkbox.indent_circle * 2 + elements->checkbox.size_circle) : 0.f, 16.f, dynamic_easing);

    draw->rect_filled(gui->window_drawlist(), rect.Min, rect.Max, draw->get_clr(clr->widgets.widgets_background), SCALE(elements->checkbox.rounding));
    draw->rect_filled(gui->window_drawlist(), rect.Min, rect.Max, draw->get_clr(clr->accent, anim->alpha * 0.10f), SCALE(elements->checkbox.rounding));
    draw->circle_filled(gui->window_drawlist(), ImVec2(rect.Min.x + SCALE(elements->checkbox.indent_circle + elements->checkbox.size_circle / 2) + anim->offset, rect.GetCenter().y), SCALE(elements->checkbox.size_circle / 2), draw->get_clr(anim->off_on_circle));
    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 14), data->rect.Min, data->rect.Max, draw->get_clr(anim->off_on_text), name.data(), NULL, NULL, ImVec2(0.f, 0.5f));

    draw->separator();

    if (keybind)
    {
        gui->easing(anim->keybind_alpha, anim->keybind_opened ? 1.f : 0.f, 8.f, static_easing);
        gui->easing(anim->keyclr, key_rect.Contains(gui->mouse_pos()) || anim->keybind_opened ? clr->widgets.text.Value : clr->widgets.text_inactive.Value, 16.f, dynamic_easing);

        draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 12), key_rect.Min, key_rect.Max, draw->get_clr(anim->keyclr), "I", NULL, NULL, ImVec2(0.5f, 0.5f));

        if ((key_rect.Contains(gui->mouse_pos()) && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0)) || (anim->keybind_opened && (gui->mouse_clicked(mouse_button_left) || gui->mouse_clicked(mouse_button_right)) && !anim->keybind_hovered))
            anim->keybind_opened = !anim->keybind_opened;

        if (anim->keybind_alpha >= 0.01f)
        {
            gui->push_var(style_var_alpha, anim->keybind_alpha);
            gui->push_var(style_var_window_padding, SCALE(elements->keybind.padding_main));
            gui->push_var(style_var_item_spacing, SCALE(elements->keybind.spacing_main));
            gui->push_var(style_var_window_rounding, SCALE(elements->keybind.rounding_window));
            gui->push_color(style_col_popup_bg, draw->get_clr(clr->window.background));
            gui->push_var(style_var_popup_border_size, 0.f);
            gui->push_color(style_col_border, 0);

            gui->set_next_window_pos(key_rect.Min);
            gui->set_next_window_size(SCALE(elements->keybind.main_size));
            gui->begin("keybind_window" + std::string(name), nullptr, window_flags_tooltip | window_flags_always_use_window_padding | window_flags_no_saved_settings | window_flags_no_focus_on_appearing | window_flags_always_auto_resize | window_flags_no_decoration | window_flags_no_scrollbar | window_flags_no_scroll_with_mouse);
            {
                gui->set_window_focus();
                anim->keybind_hovered = gui->is_window_hovered(0);

                widgets->button_keybind("New bind", "O", 0, anim->key_index);
                widgets->button_keybind("Hotkeys", "P", 1, anim->key_index);
                if (widgets->button_keybind_reset("Reset", "Q").value_changed)
                    state->clear();
            }
            gui->end();
            gui->pop_var(5);
            gui->pop_color(2);

            if (anim->alpha_2 >= 0.f)
            {
                gui->push_var(style_var_alpha, anim->alpha_2 * anim->keybind_alpha);
                gui->push_var(style_var_window_padding, SCALE(elements->keybind.padding));
                gui->push_var(style_var_item_spacing, SCALE(elements->keybind.spacing));
                gui->push_var(style_var_window_rounding, SCALE(elements->keybind.rounding_window));
                gui->push_color(style_col_popup_bg, draw->get_clr(clr->window.background));
                gui->push_var(style_var_popup_border_size, 0.f);
                gui->push_color(style_col_border, 0);

                gui->set_next_window_size(SCALE(elements->keybind.size_window));
                gui->set_next_window_pos(key_rect.Min + SCALE(elements->keybind.main_size.x + elements->keybind.padding.x, 0));
                gui->begin("keybind_window_2" + std::string(name), nullptr, window_flags_tooltip | window_flags_always_use_window_padding | window_flags_no_saved_settings | window_flags_no_focus_on_appearing | window_flags_always_auto_resize | window_flags_no_decoration | window_flags_no_scrollbar | window_flags_no_scroll_with_mouse);
                {
                    gui->set_window_focus();
                    anim->keybind_hovered |= gui->get_window()->Rect().Contains(gui->mouse_pos());

                    if (widgets->key_select("key_select", &anim->key).value_changed)
                    {
                        state->push_back({ anim->key, anim->mode, anim->value, anim->show });
                        anim->key = 0;
                        anim->mode = 0;
                        anim->value = 0;
                        anim->show = 0;
                    }
                    widgets->mode_select("mode_select", &anim->mode);

                    widgets->checkbox_keybind("Value", &anim->value);
                    widgets->checkbox_keybind("Show in binds", &anim->show);

                }
                gui->end();
                gui->pop_var(5);
                gui->pop_color(2);

            }

            if (anim->alpha_3 >= 0.f)
            {
                gui->push_var(style_var_alpha, anim->alpha_3 * anim->keybind_alpha);
                gui->push_var(style_var_window_padding, SCALE(8, 8));
                gui->push_var(style_var_item_spacing, SCALE(4, 4));
                gui->push_var(style_var_window_rounding, SCALE(elements->keybind.rounding_window));
                gui->push_color(style_col_popup_bg, draw->get_clr(clr->window.background));
                gui->push_var(style_var_popup_border_size, 0.f);
                gui->push_color(style_col_border, 0);

                gui->set_next_window_size(SCALE(120, 120));
                gui->set_next_window_pos(key_rect.Min + SCALE(elements->keybind.main_size.x + elements->keybind.padding.x, 0));
                gui->begin("keybind_window_3" + std::string(name), nullptr, window_flags_tooltip | window_flags_always_use_window_padding | window_flags_no_saved_settings | window_flags_no_focus_on_appearing | window_flags_always_auto_resize | window_flags_no_decoration | window_flags_no_scrollbar);
                {
                    gui->set_window_focus();
                    anim->keybind_hovered |= gui->get_window()->Rect().Contains(gui->mouse_pos());

                    for (int i = 0; i < state->size(); ++i)
                    {
                        if (widgets->button_keybind_added(std::to_string(i), state->at(i).key).value_changed)
                            state->erase(state->begin() + i);
                    }

                }
                gui->end();
                gui->pop_var(5);
                gui->pop_color(2);

            }
        }

    }

    return data;
}

widget_t c_widgets::checkbox(std::string_view key)
{
    checkbox_t* data = cfg->fill<checkbox_t>(key.data());

    return checkbox_ex(data->name, &data->callback, data->keybind, &data->state);
}
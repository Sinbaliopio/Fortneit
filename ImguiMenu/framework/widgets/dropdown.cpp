#include "../headers/functions.h"
#include "../headers/widgets.h"

widget_t selectable(std::string_view name, bool active)
{
    struct anim_t
    {
        ImVec4 text;
        float alpha;
        float alpha2;
    };

    widget_t data = gui->register_item(name, ImVec2(gui->content_avail().x, SCALE(elements->dropdown.size_opening_window)));
    anim_t* anim = gui->anim_container<anim_t>(data->id);

    data->value_changed = data->hovered && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0);
    gui->easing(anim->text, active || data->hovered ? clr->widgets.text.Value : clr->widgets.text_inactive.Value, 16.f, dynamic_easing);
    gui->easing(anim->alpha, data->hovered || active ? 1.f : 0.f, 8.f, static_easing);
    gui->easing(anim->alpha2, active ? 1.f : 0.f, 8.f, static_easing);

    draw->rect_filled(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.widgets_active, anim->alpha), SCALE(elements->dropdown.rounding_text));
    draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 8), data->rect.Min + SCALE(8, 0), data->rect.Max, draw->get_clr(clr->widgets.text, anim->alpha2), "R", NULL, NULL, ImVec2(0.f, 0.5f));

    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 12), data->rect.Min + SCALE(elements->dropdown.indent_text, 0), data->rect.Max, draw->get_clr(anim->text), name.data(), NULL, NULL, ImVec2(0.f, 0.5f));
    return data;
}

bool begin_dropdown(std::string_view name, std::string& preview, int val, widget_t& data, bool multi = false)
{
    struct anim_t
    {
        bool opened{ false };
        bool hovered{ false };
        int stored_val{ -1 };
        float alpha{ 0 };
        float height{ 0 };
    };

    data = gui->register_item(name, ImVec2(gui->content_avail().x, SCALE(elements->dropdown.size)));
    anim_t* anim = gui->anim_container<anim_t>(data->id);
    const ImRect rect{ data->rect.GetTR() + SCALE(0, elements->dropdown.indent) - SCALE(elements->dropdown.size_rect,0), data->rect.Max - SCALE(0,elements->dropdown.indent) };

    if (anim->stored_val < 0)
        anim->stored_val = val;

    val = val > 4 ? 4 : val;

    if ((ImRect{ rect.Min, rect.Max }.Contains(gui->mouse_pos()) && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0)) || (anim->opened && gui->mouse_clicked(mouse_button_left) && !anim->hovered))
        anim->opened = !anim->opened;

    gui->easing(anim->alpha, anim->opened ? 1.f : 0.f, 8.f, static_easing);
    draw->rect_filled(gui->window_drawlist(), rect.Min, rect.Max, draw->get_clr(clr->widgets.widgets_background), SCALE(elements->dropdown.rounding_text));
    draw->separator();
    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 14), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.text_inactive), name.data(), gui->text_end(name.data()), NULL, ImVec2(0.f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 11), rect.Min + SCALE(elements->dropdown.indent, 0), rect.Max, draw->get_clr(clr->widgets.text), preview.data(), NULL, NULL, ImVec2(0.f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 8), rect.Min, rect.Max - SCALE(elements->dropdown.indent, 0), draw->get_clr(clr->widgets.text), "G", NULL, NULL, ImVec2(1.f, 0.5f));


    if (anim->alpha <= 0.01f || !gui->is_rect_visible(ImRect{ rect.GetBL(), rect.GetBL() + SCALE(0, 2) }))
    {
        anim->hovered = false;

        return false;
    }

    gui->push_var(style_var_alpha, anim->alpha);
    gui->push_var(style_var_window_padding, SCALE(8, 8));
    gui->push_var(style_var_item_spacing, SCALE(1, 1));
    gui->push_var(style_var_window_rounding, SCALE(elements->dropdown.rounding));
    gui->push_color(style_col_popup_bg, draw->get_clr(clr->window.background));
    gui->push_var(style_var_popup_border_size, 0.f);
    gui->set_next_window_size(SCALE(elements->dropdown.size_rect, 0));
    gui->set_next_window_pos(ImVec2(rect.Min.x, rect.GetCenter().y - anim->height / 2));
    gui->begin("dropdown_window" + std::to_string(data->id), nullptr, window_flags_tooltip | window_flags_always_use_window_padding | window_flags_no_saved_settings | window_flags_no_focus_on_appearing | window_flags_always_auto_resize | window_flags_no_title_bar | window_flags_no_resize | window_flags_no_collapse | (anim->stored_val <= 2 ? (window_flags_no_scrollbar | window_flags_no_scroll_with_mouse) : window_flags_no_scrollbar));

    gui->set_window_focus();
    anim->hovered = gui->is_window_hovered(0);

    anim->height = gui->window_height();

    return true;
}

void end_dropdown()
{
    gui->end();
    gui->pop_var(5);
    gui->pop_color();
}

widget_t c_widgets::dropdown(std::string_view key)
{
    dropdown_t* data = cfg->fill<dropdown_t>(key.data());
    widget_t item_data;

    std::string preview{ data->items.at(data->callback) };

    if (begin_dropdown(data->name, preview, data->items.size(), item_data))
    {
        for (int i = 0; i < data->items.size(); ++i)
        {
            if (selectable(data->items.at(i), data->callback == i).value_changed)
            {
                data->callback = i;
                item_data->value_changed = gui->is_item_clicked(mouse_button_left);
            }
        }

        end_dropdown();
    }

    return item_data;
}

widget_t c_widgets::multi_dropdown(std::string_view key)
{
    multi_dropdown_t* data = cfg->fill<multi_dropdown_t>(key.data());
    widget_t item_data;

    std::string preview = "...";

    for (size_t i = 0; i < data->items.size(); ++i)
    {
        if (data->callback.at(i))
        {
            if (preview == "...")
                preview = data->items[i];
            else
                preview += (", ") + data->items[i];
        }
    }

    if (begin_dropdown(data->name, preview, data->items.size(), item_data, true))
    {
        for (size_t i = 0; i < data->items.size(); ++i)
        {
            if (selectable(data->items.at(i), data->callback.at(i)).value_changed)
            {
                data->callback.at(i) = !data->callback.at(i);
                item_data->value_changed = gui->is_item_clicked(mouse_button_left);
            }
        }
        end_dropdown();
    }

    preview = ("...");

    return item_data;
}

bool begin_dropdown_settings(std::string_view name, std::string_view icon, std::string& preview, int val, widget_t& data, bool multi = false)
{
    struct anim_t
    {
        bool opened{ false };
        bool hovered{ false };
        int stored_val{ -1 };
        float alpha{ 0 };

    };

    data = gui->register_item(name, ImVec2(gui->content_avail().x, SCALE(10)));
    anim_t* anim = gui->anim_container<anim_t>(data->id);

    if (anim->stored_val < 0)
        anim->stored_val = val;

    val = val > 4 ? 4 : val;

    if ((ImRect{ data->rect.Min, data->rect.Max }.Contains(gui->mouse_pos()) && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0)) || (anim->opened && gui->mouse_clicked(mouse_button_left) && !anim->hovered))
        anim->opened = !anim->opened;

    gui->easing(anim->alpha, anim->opened ? 1.f : 0.f, 8.f, static_easing);

    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 12), data->rect.Min + SCALE(elements->dropdown.padding, 0), data->rect.Max, draw->get_clr(clr->widgets.text), name.data(), NULL, NULL, ImVec2(0.f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 10), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.text), icon.data(), NULL, NULL, ImVec2(0.f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 7), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.text), "G", NULL, NULL, ImVec2(1.f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 12), data->rect.Min, data->rect.Max - SCALE(elements->dropdown.padding_text, 0), draw->get_clr(clr->widgets.text), preview.data(), NULL, NULL, ImVec2(1.f, 0.5f));

    if (anim->alpha <= 0.01f || !gui->is_rect_visible(ImRect{ data->rect.GetBL(), data->rect.GetBL() + SCALE(0, 2) }))
    {
        anim->hovered = false;
        anim->alpha = 0;

        return false;
    }

    gui->push_var(style_var_alpha, anim->alpha);
    gui->push_var(style_var_window_padding, SCALE(8, 8));
    gui->push_var(style_var_item_spacing, SCALE(1, 1));
    gui->push_var(style_var_window_rounding, SCALE(elements->dropdown.rounding));
    gui->push_color(style_col_popup_bg, draw->get_clr(clr->window.background));
    gui->push_var(style_var_popup_border_size, 0.f);
    gui->set_next_window_size(SCALE(elements->dropdown.size_rect, 0));
    gui->set_next_window_pos(data->rect.GetBL() + SCALE(0, elements->dropdown.indent));
    gui->begin("dropdown_window" + std::to_string(data->id), nullptr, window_flags_tooltip | window_flags_always_use_window_padding | window_flags_no_saved_settings | window_flags_no_focus_on_appearing | window_flags_always_auto_resize | window_flags_no_title_bar | window_flags_no_resize | window_flags_no_collapse | (anim->stored_val <= 2 ? (window_flags_no_scrollbar | window_flags_no_scroll_with_mouse) : window_flags_no_scrollbar));

    gui->set_window_focus();
    anim->hovered = gui->is_window_hovered(0);

    return true;
}

void end_dropdown_settings()
{
    gui->end();
    gui->pop_var(5);
    gui->pop_color();
}

widget_t c_widgets::dropdown_settings(std::string_view key)
{
    settings_dropdown_t* data = cfg->fill<settings_dropdown_t>(key.data());
    widget_t item_data;

    std::string preview{ data->items.at(data->callback) };

    if (begin_dropdown_settings(data->name, data->icon, preview, data->items.size(), item_data))
    {
        for (int i = 0; i < data->items.size(); ++i)
        {
            if (selectable(data->items.at(i), data->callback == i).value_changed)
            {
                data->callback = i;
                item_data->value_changed = gui->is_item_clicked(mouse_button_left);
            }
        }

        end_dropdown_settings();
    }

    return item_data;
}

bool begin_dropdown_cfg(std::string_view name, std::string& preview, int val, widget_t& data, bool multi = false)
{
    struct anim_t
    {
        bool opened{ false };
        bool hovered{ false };
        int stored_val{ -1 };
        float alpha{ 0 };
        float height{ 0 };
        float rotate{ 0 };
    };

    data = gui->register_item(name, SCALE(elements->button.size));
    anim_t* anim = gui->anim_container<anim_t>(data->id);
    const ImRect rect_icon{ data->rect.Min, data->rect.GetTL() + SCALE(elements->button.rect_icon) };

    if (anim->stored_val < 0)
        anim->stored_val = val;

    val = val > 4 ? 4 : val;

    if ((ImRect{ data->rect.Min, data->rect.Max }.Contains(gui->mouse_pos()) && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0)) || (anim->opened && gui->mouse_clicked(mouse_button_left) && !anim->hovered))
        anim->opened = !anim->opened;

    gui->easing(anim->alpha, anim->opened ? 1.f : 0.f, 8.f, static_easing);
    gui->easing(anim->rotate, anim->opened ? 1.f : 0.f, 16.f, dynamic_easing);

    draw->rect_filled(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(clr->window.child), SCALE(elements->button.rounding));
    draw->rect(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(clr->window.rect), SCALE(elements->button.rounding));
    draw->line(gui->window_drawlist(), rect_icon.GetTR(), rect_icon.GetBR(), draw->get_clr(clr->window.rect));

    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 14), rect_icon.GetTR() + SCALE(elements->button.indent, 0), data->rect.Max, draw->get_clr(clr->widgets.text), preview.data(), NULL, NULL, ImVec2(0.f, 0.5f));
    
    draw->rotate_start(gui->window_drawlist());
    draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 8), data->rect.Min, data->rect.Max - SCALE(elements->button.indent, 0), draw->get_clr(clr->widgets.text), "G", NULL, NULL, ImVec2(1.f, 0.5f));
    draw->rotate_end(gui->window_drawlist(), 90.f - (180.f * anim->rotate));

    draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 12), rect_icon.Min, rect_icon.Max, draw->get_clr(clr->widgets.text), "H", NULL, NULL, ImVec2(0.5f, 0.5f));

    if (anim->alpha <= 0.01f || !gui->is_rect_visible(ImRect{ data->rect.GetBL(), data->rect.GetBL() + SCALE(0, 2) }))
    {
        anim->hovered = false;

        return false;
    }

    gui->push_var(style_var_alpha, anim->alpha);
    gui->push_var(style_var_window_padding, SCALE(8, 8));
    gui->push_var(style_var_item_spacing, SCALE(1, 1));
    gui->push_var(style_var_window_rounding, SCALE(elements->dropdown.rounding));
    gui->push_color(style_col_popup_bg, draw->get_clr(clr->window.background));
    gui->push_var(style_var_popup_border_size, 0.f);
    gui->set_next_window_size(ImVec2(data->rect.GetWidth(), 0));
    gui->set_next_window_pos(data->rect.GetBL() + SCALE(0, 10));
    gui->begin("dropdown_window" + std::to_string(data->id), nullptr, window_flags_tooltip | window_flags_always_use_window_padding | window_flags_no_saved_settings | window_flags_no_focus_on_appearing | window_flags_always_auto_resize | window_flags_no_title_bar | window_flags_no_resize | window_flags_no_collapse | (anim->stored_val <= 2 ? (window_flags_no_scrollbar | window_flags_no_scroll_with_mouse) : window_flags_no_scrollbar));

    gui->set_window_focus();
    anim->hovered = gui->is_window_hovered(0);

    anim->height = gui->window_height();

    return true;
}

void end_dropdown_cfg()
{
    gui->end();
    gui->pop_var(5);
    gui->pop_color();
}

widget_t c_widgets::dropdown_cfg(std::string_view name, int* selected, const std::vector<std::string>& items)
{
    widget_t item_data;

    std::string preview{ items.at(*selected) };

    if (begin_dropdown_cfg(name, preview, items.size(), item_data))
    {
        for (int i = 0; i < items.size(); ++i)
        {
            if (selectable(items.at(i), *selected == i).value_changed)
            {
                *selected = i;
                item_data->value_changed = gui->is_item_clicked(mouse_button_left);
            }
        }

        end_dropdown_cfg();
    }

    return item_data;
}

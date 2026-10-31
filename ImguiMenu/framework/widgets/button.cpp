#include "../headers/functions.h"
#include "../headers/widgets.h"

widget_t c_widgets::button(std::string_view name)
{
    struct anim_t
    {
        float active{ false };
        float alpha;
        ImVec4 offset{ clr->accent };

    };

    widget_t data = gui->register_item(name, SCALE(elements->button.size));
    anim_t* anim = gui->anim_container<anim_t>(data->id);

    const ImRect rect_icon{ data->rect.Min, data->rect.GetTL() + SCALE(elements->button.rect_icon) };

    data->value_changed = data->hovered && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0);
    if (data->value_changed)
        anim->active = true;
    gui->easing(anim->alpha, anim->active ? 1.f : 0.f, 6.f, static_easing);
    gui->easing(anim->offset, anim->active ? clr->accent.Value : clr->window.rect.Value, 16.f, dynamic_easing);

    if (anim->alpha >= 0.99f)
        anim->active = false;
    draw->rect_filled(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(clr->window.child), SCALE(elements->button.rounding));
    draw->rect(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(anim->offset), SCALE(elements->button.rounding));
    draw->line(gui->window_drawlist(), rect_icon.GetTR(), rect_icon.GetBR(), draw->get_clr(anim->offset));

    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 14), rect_icon.GetTR() + SCALE(elements->button.indent, 0), data->rect.Max, draw->get_clr(clr->widgets.text), name.data(), gui->text_end(name.data()), NULL, ImVec2(0.f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 8), data->rect.Min, data->rect.Max - SCALE(elements->button.indent, 0), draw->get_clr(clr->widgets.text), "G", NULL, NULL, ImVec2(1.f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 12), rect_icon.Min, rect_icon.Max, draw->get_clr(clr->widgets.text), "H", NULL, NULL, ImVec2(0.5f, 0.5f));

    return data;
}

widget_t c_widgets::button_page(std::string_view name)
{
    struct anim_t
    {
        float active{ false };
        float alpha;
        ImVec4 offset{ clr->accent };

    };

    widget_t data = gui->register_item(name, ImVec2(gui->text_size(font->get(inter_medium_data, 14), name.data()).x + SCALE(elements->button.indent), SCALE(elements->button_page.size.y))); 
    anim_t* anim = gui->anim_container<anim_t>(data->id);

    data->value_changed = data->hovered && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0);
    if (data->value_changed)
        anim->active = true;
    gui->easing(anim->alpha, anim->active ? 1.f : 0.f, 6.f, static_easing);
    gui->easing(anim->offset, anim->active ? clr->accent.Value : clr->window.rect.Value, 16.f, dynamic_easing);

    if (anim->alpha >= 0.99f)
        anim->active = false;
    draw->rect_filled(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(clr->window.child), SCALE(elements->button.rounding));
    draw->rect(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(anim->offset), SCALE(elements->button.rounding));

    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 14), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.text), name.data(), gui->text_end(name.data()), NULL, ImVec2(0.5f, 0.5f));

    return data;
}

widget_t c_widgets::settings_button(ImTextureID image)
{
    widget_t data = gui->register_item("settings_button", SCALE(elements->button.settings_size));

    data->value_changed = data->hovered && gui->mouse_clicked(mouse_button_left);

    draw->image_rounded(gui->window_drawlist(), image, data->rect.Min, data->rect.Max, ImVec2(0, 0), ImVec2(1, 1), draw->get_clr({ 1.f, 1.f, 1.f, 1.f }), data->rect.GetWidth() / 2);

    return data;
}
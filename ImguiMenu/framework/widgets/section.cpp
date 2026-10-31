#include "../headers/functions.h"
#include "../headers/widgets.h"

widget_t c_widgets::section(std::string_view name, std::string_view icon, int index, int& count)
{
    struct anim_t
    {
        float alpha;
        ImVec4 icon{ clr->widgets.text_inactive };
    };

    widget_t data = gui->register_item(name, SCALE(elements->section.size_rect, elements->section.size_rect));
    anim_t* anim = gui->anim_container<anim_t>(data->id);
    bool active = index == count;
    data->value_changed = data->hovered && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0);
    if (data->value_changed)
        count = index;
    gui->easing(anim->alpha, active ? 1.f : 0.f, 4.f, static_easing);
    gui->easing(anim->icon, active ? clr->widgets.text.Value : clr->widgets.text_inactive.Value, 16.f, dynamic_easing);

    draw->rect_filled(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.section_on, anim->alpha *0.2f), SCALE(elements->section.rounding));
    draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 16), data->rect.Min, data->rect.Max, draw->get_clr(anim->icon), icon.data(), NULL, NULL, ImVec2(0.5f, 0.5f));

    return data;

}
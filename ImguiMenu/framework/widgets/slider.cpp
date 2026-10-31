#include "../headers/functions.h"
#include "../headers/widgets.h"

template <typename T>
widget_t slider_ex(std::string_view name, T* callback, T min, T max, std::string_view format)
{
    struct anim_t
    {
        float offset{ 0 };
        float alpha;

    };

    widget_t data = gui->register_item(name, ImVec2(gui->content_avail().x, SCALE(elements->slider.size)));
    anim_t* anim = gui->anim_container<anim_t>(data->id);
    const ImRect rect{ data->rect.GetTR() - SCALE(elements->slider.size_rect.x,0) + SCALE(0, elements->slider.indent), data->rect.Max - SCALE(0,elements->slider.indent)};

    bool hovered, held;
    bool pressed = gui->button_behavior(rect, data->id, &hovered, &held);

    const float padding = SCALE(0); // отступ от края
    const float grab_width = SCALE(elements->slider.size_circle); // размер круга
    const float width_with_grab = rect.GetWidth() - grab_width - padding * 2;
    const float grab_offset = padding + grab_width / 2 + static_cast<float>((*callback - min)) / static_cast<float>(max - min) * width_with_grab;

    char value_buf[64]; const char* value_buf_end = gui->get_fmt(value_buf, callback, format);
    const float t = ImSaturate((gui->mouse_pos().x - (rect.Min.x + padding + grab_width / 2)) / width_with_grab);

    data->value_changed = held;

    if (data->value_changed)
    {

        *callback += static_cast<T>(((min + t * (max - min)) - *callback));
        *callback = ImClamp(*callback, min, max);

    }

    gui->easing(anim->offset, grab_offset, 20.f, dynamic_easing);
    gui->easing(anim->alpha, *callback ? 1.f : 0.f, 8.f, static_easing);

    draw->rect_filled(gui->window_drawlist(), rect.Min, rect.Max, draw->get_clr(clr->widgets.widgets_background), SCALE(elements->slider.rounding));
    draw->rect_filled(gui->window_drawlist(), ImVec2(rect.Min.x, rect.Min.y), ImVec2(rect.Min.x + SCALE(0 + elements->slider.size_circle / 2) + anim->offset, rect.Max.y), draw->get_clr(clr->accent), SCALE(elements->slider.rounding));
    draw->circle_filled(gui->window_drawlist(), ImVec2(rect.Min.x + anim->offset, rect.GetCenter().y), SCALE(elements->slider.size_circle / 2), draw->get_clr(clr->widgets.text), 30);

    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 14.f), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.text_inactive), name.data(), gui->text_end(name.data()), NULL, ImVec2(0.f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 14), data->rect.Min, data->rect.Max - SCALE(elements->slider.indent_text), draw->get_clr(clr->widgets.text), value_buf, value_buf_end, NULL, ImVec2(1.f, 1.f));

    draw->separator();

    return data;
}

widget_t c_widgets::slider_int(std::string_view key)
{
    slider_int_t* data = cfg->fill<slider_int_t>(key.data());

    return slider_ex(data->name, &data->callback, data->min, data->max, data->format);
}

widget_t c_widgets::slider_float(std::string_view key)
{
    slider_float_t* data = cfg->fill<slider_float_t>(key.data());

    return slider_ex(data->name, &data->callback, data->min, data->max, data->format);
}

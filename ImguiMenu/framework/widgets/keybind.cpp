#include "../headers/functions.h"
#include "../headers/widgets.h"

const char* const keys[] = { "Tab", "Left", "Right", "Up", "Down", "Page Up", "Page Down", "Home", "End", "Insert", "Delete", "Backspace", "Space", "Enter", "Escape", "Ctrl", "Shift", "Alt", "Super", "Ctrl", "Shift", "Alt", "Super", "Menu", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "Apostrophe", "Comma", "Minus", "Period", "Slash", "Semicolon", "Equal", "Left Bracket", "Backslash", "Right Bracket", "Grave Accent", "Caps Lock", "Scroll Lock", "Num Lock", "Print Screen", "Pause", "Keypad 0", "Keypad 1", "Keypad 2", "Keypad 3", "Keypad 4", "Keypad 5", "Keypad 6", "Keypad 7", "Keypad 8", "Keypad 9", "Keypad .", "Keypad /", "Keypad *", "Keypad -", "Keypad +", "Keypad Enter", "Keypad =", "App Back", "App Forward", "Gamepad Start", "Gamepad Back", "Gamepad Face Left", "Gamepad Face Right", "Gamepad Face Up", "Gamepad Face Down", "Gamepad Dpad Left", "Gamepad Dpad Right", "Gamepad Dpad Up", "Gamepad Dpad Down", "Gamepad L1", "Gamepad R1", "Gamepad L2", "Gamepad R2", "Gamepad L3", "Gamepad R3", "Gamepad L Stick Left", "Gamepad L Stick Right", "Gamepad L Stick Up", "Gamepad L Stick Down", "Gamepad R Stick Left", "Gamepad R Stick Right", "Gamepad R Stick Up", "Gamepad R Stick Down", "Mouse 1", "Mouse 2", "Mouse 3", "Mouse 4", "Mouse 5", "Mouse Wheel X", "Mouse Wheel Y", "Ctrl", "Shift", "Alt", "Super" };

const char* get_key_name(ImGuiKey key)
{
    if (key == ImGuiKey_None)
        return "-";

    return keys[key - ImGuiKey_NamedKey_BEGIN];
}

widget_t c_widgets::key_select(std::string_view name, int* key)
{
    struct anim_t
    {
        bool active{ false };
        float alpha{ 0 };
    };

    widget_t data = gui->register_item(name, SCALE(elements->keybind.key_select_size));
    anim_t* anim = gui->anim_container<anim_t>(data->id);

    const ImRect plus_rect{ data->rect.GetTR() - SCALE(elements->keybind.key_select_size.y,0) ,data->rect.Max };
    const ImRect main_rect{ data->rect.Min ,plus_rect.GetBL() };

    std::string buf_display = anim->active ? "-" : get_key_name((ImGuiKey)*key);

    if (main_rect.Contains(gui->mouse_pos()) && gui->mouse_released(mouse_button_left) && gui->is_window_hovered(0))
        anim->active = true;


    data->value_changed = plus_rect.Contains(gui->mouse_pos()) && gui->mouse_clicked(mouse_button_left);

    if (anim->active)
    {
        for (int i = 0; i < ImGuiKey_COUNT; ++i)
        {

            if (IsKeyPressed(ImGuiKey_Escape))
            {
                *key = 0;
                anim->active = false;
            }
            if (IsKeyPressed((ImGuiKey)i))
            {
                *key = i;
                anim->active = false;
            }
        }
    }
    draw->rect_filled(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.widgets_active), SCALE(elements->keybind.rounding_widgets));

    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 12), data->rect.Min + SCALE(elements->keybind.indent_text, 0), data->rect.Max, draw->get_clr(clr->widgets.text), buf_display.data(), gui->text_end(buf_display.data()), NULL, ImVec2(0.f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 16), plus_rect.Min, plus_rect.Max, draw->get_clr(clr->widgets.text), "+", NULL, NULL, ImVec2(0.5f, 0.5f));

    return data;
}

widget_t c_widgets::mode_select(std::string_view name, int* mode)
{
    struct anim_t
    {
        float alpha[2]{ 0, 0 };
        ImVec4 text[2]{ clr->widgets.text, clr->widgets.text };
    };

    widget_t data = gui->register_item(name, ImVec2(SCALE(elements->keybind.mode_select_size)));
    anim_t* anim = gui->anim_container<anim_t>(data->id);

    const ImRect toggle{ data->rect.Min, ImVec2(data->rect.GetCenter().x - var->style.item_spacing.x / 2, data->rect.Max.y) };
    const ImRect hold{ ImVec2(data->rect.GetCenter().x + var->style.item_spacing.x / 2, data->rect.Min.y), data->rect.Max };

    if (toggle.Contains(gui->mouse_pos()) && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0))
        *mode = 0;
    if (hold.Contains(gui->mouse_pos()) && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0))
        *mode = 1;

    gui->easing(anim->alpha[0], *mode == 0 ? 1.f : 0.f, 8.f, static_easing);
    gui->easing(anim->alpha[1], *mode == 1 ? 1.f : 0.f, 8.f, static_easing);

    gui->easing(anim->text[0], *mode == 0 ? clr->widgets.black.Value : clr->widgets.text.Value, 16.f, dynamic_easing);
    gui->easing(anim->text[1], *mode == 1 ? clr->widgets.black.Value : clr->widgets.text.Value, 16.f, dynamic_easing);


    draw->rect_filled(gui->window_drawlist(), toggle.Min, toggle.Max, draw->get_clr(clr->widgets.widgets_active), SCALE(elements->keybind.rounding_widgets));
    draw->rect_filled(gui->window_drawlist(), hold.Min, hold.Max, draw->get_clr(clr->widgets.widgets_active), SCALE(elements->keybind.rounding_widgets));
    draw->rect_filled(gui->window_drawlist(), toggle.Min, toggle.Max, draw->get_clr(clr->accent, anim->alpha[0]), SCALE(elements->keybind.rounding_widgets));
    draw->rect_filled(gui->window_drawlist(), hold.Min, hold.Max, draw->get_clr(clr->accent, anim->alpha[1]), SCALE(elements->keybind.rounding_widgets));

    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 12), toggle.Min, toggle.Max, draw->get_clr(anim->text[0]), "Toggle", NULL, NULL, ImVec2(0.5f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 12), hold.Min, hold.Max, draw->get_clr(anim->text[1]), "Hold", NULL, NULL, ImVec2(0.5f, 0.5f));

    return data;
}

widget_t c_widgets::checkbox_keybind(std::string_view name, bool* callback)
{
    struct anim_t
    {
        float alpha;
        float offset;
        ImVec4 off_on_circle{ clr->accent };

        ImVec4 text[2]{ clr->widgets.text, clr->widgets.text };
    };

    widget_t data = gui->register_item(name, ImVec2(SCALE(elements->keybind.checbox_keybind_size)));
    anim_t* anim = gui->anim_container<anim_t>(data->id);
    const ImRect rect{ data->rect.GetTR() - SCALE(elements->keybind.size_rect_chec,0), data->rect.Max };

    data->value_changed = data->hovered && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered(0);

    if (data->value_changed)
        *callback = !*callback;
    gui->easing(anim->alpha, *callback ? 1.f : 0.f, 8.f, static_easing);
    gui->easing(anim->offset, *callback ? rect.GetWidth() - SCALE(elements->checkbox.indent_circle * 3 + elements->checkbox.size_circle / 2) : 0.f, 16.f, dynamic_easing);
    gui->easing(anim->off_on_circle, *callback ? clr->accent.Value : clr->widgets.widgets_inactive.Value, 16.f, dynamic_easing);



    draw->rect_filled(gui->window_drawlist(), rect.Min, rect.Max, draw->get_clr(clr->widgets.widgets_background), SCALE(elements->keybind.rounding_checbox));
    draw->rect_filled(gui->window_drawlist(), rect.Min, rect.Max, draw->get_clr(clr->accent, anim->alpha * 0.10f), SCALE(elements->keybind.rounding_checbox));
    draw->circle_filled(gui->window_drawlist(), ImVec2(rect.Min.x + SCALE(elements->keybind.indent_circle + elements->keybind.size_circle / 2) + anim->offset, rect.GetCenter().y), SCALE(elements->keybind.size_circle / 2), draw->get_clr(anim->off_on_circle));

    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 12), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.text), name.data(), NULL, NULL, ImVec2(0.f, 0.5f));

    return data;
}


widget_t c_widgets::button_keybind(std::string_view name, std::string_view icon, int index, int& count)
{
    struct anim_t
    {
        float active{ false };
        float alpha;
        ImVec4 offset{ clr->widgets.widgets_active };

    };

    widget_t data = gui->register_item(name, ImVec2(gui->content_avail().x, SCALE(elements->keybind.size_main_widgets)));
    anim_t* anim = gui->anim_container<anim_t>(data->id);

    bool active = index == count;

    data->value_changed = data->hovered && gui->mouse_clicked(mouse_button_left);
    if (data->value_changed)
        count = index;

    gui->easing(anim->alpha, active ? 1.f : 0.f, 6.f, static_easing);
    gui->easing(anim->offset, anim->active ? clr->widgets.widgets_active.Value : clr->window.background.Value, 16.f, dynamic_easing);

    if (anim->alpha >= 0.99f)
        anim->active = false;

    draw->rect_filled(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.widgets_inactive, anim->alpha), SCALE(elements->keybind.rounding_widgets));
    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 12), data->rect.Min + SCALE(elements->keybind.padding_main_name, 0), data->rect.Max, draw->get_clr(clr->widgets.text), name.data(), NULL, NULL, ImVec2(0.f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 12), data->rect.Min + SCALE(elements->keybind.padding_main_icon, 0), data->rect.Max, draw->get_clr(clr->widgets.text), icon.data(), NULL, NULL, ImVec2(0.f, 0.5f));

    return data;
}

widget_t c_widgets::button_keybind_reset(std::string_view name, std::string_view icon)
{
    struct anim_t
    {
        float active{ false };
        float alpha;
        ImVec4 offset{ clr->widgets.widgets_active };

    };

    widget_t data = gui->register_item(name, ImVec2(gui->content_avail().x, SCALE(elements->keybind.size_main_widgets)));
    anim_t* anim = gui->anim_container<anim_t>(data->id);

    data->value_changed = data->hovered && gui->mouse_clicked(mouse_button_left);
    if (data->value_changed)
        anim->active = true;

    gui->easing(anim->alpha, anim->active ? 1.f : 0.f, 6.f, static_easing);
    if (anim->alpha >= 0.99f)
        anim->active = false;

    draw->rect_filled(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.widgets_inactive, anim->alpha), SCALE(elements->keybind.rounding_widgets));

    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 12), data->rect.Min + SCALE(elements->keybind.padding_main_name, 0), data->rect.Max, draw->get_clr(clr->widgets.reb), name.data(), NULL, NULL, ImVec2(0.f, 0.5f));
    draw->text_clipped(gui->window_drawlist(), font->get(icon_data, 12), data->rect.Min + SCALE(elements->keybind.padding_main_icon, 0), data->rect.Max, draw->get_clr(clr->widgets.reb), icon.data(), NULL, NULL, ImVec2(0.f, 0.5f));

    return data;
}

widget_t c_widgets::button_keybind_added(std::string_view name, int key)
{
    struct anim_t
    {

    };

    widget_t data = gui->register_item(name, ImVec2(gui->content_avail().x, SCALE(elements->keybind.size_main_widgets)));
    anim_t* anim = gui->anim_container<anim_t>(data->id);

    data->value_changed = data->hovered && gui->mouse_clicked(mouse_button_left) && gui->is_window_hovered();

    draw->rect_filled(gui->window_drawlist(), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.widgets_active), SCALE(4));
    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 12), data->rect.Min, data->rect.Max, draw->get_clr(clr->widgets.text), get_key_name((ImGuiKey)key), NULL, NULL, ImVec2(0.5f, 0.5f));

    return data;
}
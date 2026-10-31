#include "../headers/functions.h"
#include "../headers/widgets.h"

void c_widgets::begin_child(std::string_view name)
{
    struct child_state
    {
        float height{ 0 };
        float width{ 0 };
    };

    ImGuiWindow* window = gui->get_window();
    const ImVec2 pos = window->DC.CursorPos + SCALE(0, elements->child.name);

    child_state* anim = gui->anim_container<child_state>(window->GetID(name.data()));
    anim->width = (gui->content_max().x - var->style.item_spacing.x - var->style.window_padding.x) / 2.f;
    draw->rect_filled(gui->window_drawlist(), pos, pos + ImVec2(anim->width, anim->height), draw->get_clr(clr->window.child), SCALE(elements->child.rounding));
    draw->rect(gui->window_drawlist(), pos, pos + ImVec2(anim->width, anim->height), draw->get_clr(clr->window.rect), SCALE(elements->child.rounding));

    draw->text_clipped(gui->window_drawlist(), font->get(inter_medium_data, 14), window->DC.CursorPos, pos + ImVec2(anim->width, 0), draw->get_clr(clr->widgets.text_inactive), name.data());
    gui->push_var(style_var_window_padding, SCALE(elements->child.padding,0));
    gui->set_screen_pos(pos, pos_all);
    gui->begin_def_child(name.data() + std::string("content_child"), ImVec2(anim->width, anim->height), 0, window_flags_always_use_window_padding | window_flags_no_move | window_flags_nav_flattened | window_flags_no_saved_settings | window_flags_no_scrollbar | window_flags_no_scroll_with_mouse);
    gui->push_var(style_var_item_spacing, SCALE(0,1));

    anim->height = gui->get_window()->ContentSize.y;
}

void c_widgets::end_child()
{
    gui->pop_var();
    gui->end_def_child();
    gui->pop_var();
}
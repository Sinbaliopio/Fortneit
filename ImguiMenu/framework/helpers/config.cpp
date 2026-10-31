#include "../headers/config.h"

void c_config::init_config()
{
    static bool init = false;

    if (!init)
    {
        add_option<checkbox_t>("Enable aimbot", true, true);
        add_option< dropdown_t>("Conditions", 0, string_t{ "Head", "Chest", "Stomach", "Arms", "Legs", "Feat" });
        add_option< multi_dropdown_t>("Bone aimbot", bool_t{ 0, 0, 0, 0,0,0 }, string_t{ "Through", "Smoke", "Always" });
        add_option< multi_dropdown_t>("Hitboxes", bool_t{ 0, 0, 0, 0,0,0 }, string_t{ "Head", "Chest", "Stomach", "Smoke" });
        add_option<slider_int_t>("Field of view", 90, 0, 180, "%du");
        add_option<slider_int_t>("Smoothing", 50, 0, 100, "%d%%");
        add_option<slider_int_t>("Reaction time", 2000, 0, 5000, "%dmc");
        add_option<slider_int_t>("Target switch delay", 2000, 0, 5000, "%dmc");
        add_option<checkbox_t>("First bullet delay", true);
        add_option<checkbox_t>("Recoil control", false);

        add_option<checkbox_t>("Enable triggerbot", true);
        add_option< dropdown_t>("Conditions##1", 0, string_t{ "Head", "Chest", "Stomach", "Arms", "Legs", "Feat" });
        add_option<slider_int_t>("Hit change", 100, 0, 100, "%d%%");
        add_option<slider_int_t>("Reaction time##1", 0, 0, 100, "%dmc");
        add_option<slider_int_t>("Burst time", 0, 0, 100, "%dmc");
        add_option<checkbox_t>("Quick scope", true);


        add_option<settings_dropdown_t>("Language", "K", 0, string_t{ "English", "Russian" });
        add_option<settings_dropdown_t>("DPI Menu", "L", 1, string_t{ "75%", "100%", "150%", "200%" });
        add_option < color_edit_t>("Styles", col_t{ 1.f, 1.f, 1.f, 1.f }, true, "M", "N");

        init = true;
    }
}

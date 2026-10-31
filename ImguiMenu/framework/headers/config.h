#pragma once
#include <map>
#include <variant>
#include <array>
#include <vector>
#include <string>
#include <mutex>
#include <memory>

struct key_data_t
{
    int key;
    int mode;
    bool value;
    bool show;
};

struct color_edit_t
{
    std::string name;
    std::array<float, 4> color;
    bool alpha;
    std::string icon;
    std::string icon_2;

};

struct checkbox_t
{
    std::string name;
    bool callback;
    bool keybind;
    std::vector<key_data_t> state;

};

template <typename T>
struct dropdown_data
{
    std::string name;
    T callback;
    std::vector<std::string> items;
};

template <typename T>
struct key_data
{
    int key;
    int mode;
    T value;
};

template <typename T>
struct key_state_t
{
    T stored;
    std::vector<key_data<T>> keys;
};

struct text_field_t
{
    std::string name;
    int size;
    std::string callback;
};

template <typename T>
struct slider_data
{
    std::string name;
    T callback;
    T min;
    T max;
    std::string format;
};

struct settings_dropdown_t
{
    std::string name;
    std::string icon;
    int callback;
    std::vector<std::string> items;
};

enum config_type
{
    checkbox_type,
    slider_int_type,
    slider_float_type,
    dropdown_type,
    multi_dropdown_type,
    text_field_type,
    range_int_type,
    range_float_type,
    color_edit_type,
    settings_dropdown_type
};

using col_t = std::array<float, 4>;
using string_t = std::vector<std::string>;
using slider_int_t = slider_data<int>;
using dropdown_t = dropdown_data<int>;

using slider_float_t = slider_data<float>;
using bool_t = std::vector<bool>;
using multi_dropdown_t = dropdown_data<bool_t>;
using config_variant = std::variant<checkbox_t, slider_int_t, slider_float_t, multi_dropdown_t, dropdown_t, color_edit_t, text_field_t, settings_dropdown_t>;

class c_config
{
public:

    void init_config();

    template <typename T>
    T& get(const std::string& name) { return std::get<T>(options[name]); }

    template <typename T>
    T* fill(const std::string& name)
    {
        auto& option = options[name];

        return std::get_if<T>(&option);
    }

    std::vector<std::pair<std::string, int>> order;

private:

    template <typename T, typename... Args>
    void add_option(const std::string& name, Args&&... args)
    {
        T option{ name, std::forward<Args>(args)... };
        options[name] = option;
        order.push_back({ name, get_type<T>() });
    }

    template <typename T>
    int get_type() const
    {
        if constexpr (std::is_same_v<T, checkbox_t>) return checkbox_type;
        if constexpr (std::is_same_v<T, slider_int_t>) return slider_int_type;
        if constexpr (std::is_same_v<T, multi_dropdown_t>) return multi_dropdown_type;
        if constexpr (std::is_same_v<T, slider_float_t>) return slider_float_type;
        if constexpr (std::is_same_v<T, color_edit_t>) return color_edit_type;
        if constexpr (std::is_same_v<T, dropdown_t>) return dropdown_type;
        if constexpr (std::is_same_v<T, text_field_t>) return text_field_type;
        if constexpr (std::is_same_v<T, multi_dropdown_t>) return multi_dropdown_type;
        if constexpr (std::is_same_v<T, settings_dropdown_t>) return settings_dropdown_type;
    }

    std::map<std::string, config_variant> options;
};

inline std::unique_ptr<c_config> cfg = std::make_unique<c_config>();
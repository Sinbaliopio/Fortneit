
#include "../headers/includes.h"
#include "../headers/widgets.h"
#include <algorithm>

void c_search::search()
{
    ImGuiContext& g = *GImGui;
    const ImVec2 pos = GetWindowPos();
    const ImVec2 size = GetWindowSize();
    ImDrawList* drawlist = GetWindowDrawList();

    std::string search_element_lower = search_element;
    std::transform(search_element_lower.begin(), search_element_lower.end(), search_element_lower.begin(), [](unsigned char c) { return std::tolower(c); });

    for (int i = 0; i < cfg->order.size(); i++)
    {
        std::string name_lower = cfg->order.at(i).first;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), [](unsigned char c) { return std::tolower(c); });

        if (search_element.empty())
            continue;

        if (!search_element_lower.empty() && name_lower.find(search_element_lower) == std::string::npos)
            continue;

        if (cfg->order.at(i).second == checkbox_type)
        {
            widgets->checkbox(cfg->order.at(i).first.data());
        }
        if (cfg->order.at(i).second == slider_int_type)
        {
            widgets->slider_int(cfg->order.at(i).first.data());
        }
        if (cfg->order.at(i).second == slider_float_type)
        {
            widgets->slider_float(cfg->order.at(i).first.data());
        }
        if (cfg->order.at(i).second == dropdown_type)
        {
            widgets->dropdown(cfg->order.at(i).first.data());
        }
        if (cfg->order.at(i).second == multi_dropdown_type)
        {
            widgets->multi_dropdown(cfg->order.at(i).first.data());
        }
        
    }

}
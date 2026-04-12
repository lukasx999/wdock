#include "application.hpp"
#include "widgets.hpp"

int main() {

    int width = 700;
    int height = 800;
    auto anchor = window::anchor::right;
    window::margin margin =  {0, 200, 0, 0};

    std::vector<std::unique_ptr<widgets::widget>> widgets;
    widgets.push_back(std::make_unique<widgets::date>("Europe/Vienna"));
    widgets.push_back(std::make_unique<widgets::time>("Europe/Vienna"));
    widgets.push_back(std::make_unique<widgets::kernel>());


    application app(width, height, anchor, margin, std::move(widgets));
    app.run();

}
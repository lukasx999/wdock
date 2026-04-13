#include "application.hpp"
#include "widgets.hpp"

int main() {

    int width = 700;
    int height = 800;
    auto anchor = window::anchor::right;
    window::margin margin =  {0, 200, 0, 0};

    application app(width, height, anchor, margin);

    // widgets must be added AFTER the application has been constructed, as this
    // is when the opengl context gets initialized, which a widget might use
    app.add_widget<widgets::date>("Europe/Vienna");
    app.add_widget<widgets::time>("Europe/Vienna");
    app.add_widget<widgets::kernel>();
    app.add_widget<widgets::memory>();
    app.add_widget<widgets::image>("./image.jpg");

    app.run();

}
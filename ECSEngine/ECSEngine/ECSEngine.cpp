#include "Application.h"
#include <iostream>

int main() {
    Application& app = Application::getInstance();

    if (!app.init(800, 600, "GameObject & Transform Test")) {
        std::cerr << "Failed to initialize Application." << std::endl;
        return -1;
    }

    app.run();

    return 0;
}
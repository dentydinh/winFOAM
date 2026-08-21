#include "gui/Application.hpp"

#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    winfoam::gui::Application app;

    if (!app.initialize("winFOAM - OpenFOAM GUI", 1600, 900)) {
        std::cerr << "Failed to initialize application\n";
        return EXIT_FAILURE;
    }

    app.run();

    return EXIT_SUCCESS;
}
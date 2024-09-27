#include <iostream>

#include "IApplication.h"

int main(int argc, char* argv[]) {
    try {
        auto* app = IApplication::Create();
        app->Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

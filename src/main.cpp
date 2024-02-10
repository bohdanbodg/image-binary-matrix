#include "app.hpp"

namespace fs = std::filesystem;

static bool appStart(int argc, char const *argv[]) {
    if (argc == 0) {
        return false;
    }

    auto app = new IBMApplication(argv[0]);
    if (app->init()) {
        app->run();
    }

    delete app;

    return true;
}

int main(int argc, char const *argv[]) {
    return (int)(!appStart(argc, argv));
}

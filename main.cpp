//
// Created by Khang on 8/23/2025.
//
#include <memory>

#include <surface.h>

int main(int argc, char *argv[]) {
    std::unique_ptr<xcSurface> xcs = std::make_unique<xcSurface>();

    if (!xcs->init()) {
        return -1;
    }

    xcs->run();

    xcs->cleanup();

    return 0;
}
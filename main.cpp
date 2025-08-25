//
// Created by Khang on 8/23/2025.
//
//#include <memory>

#include <surface.h>
#include <grx.h>

int main(int argc, char *argv[]) {
    xcSurface *xcs = new xcSurface();

    if (!xcs->init(640, 480)) {
        return -1;
    }

	xcGraphics *xcg = new xcGraphics(xcs);
	xcg->init();

    xcs->run();

	xcg->cleanup();
	delete xcg;
    xcs->cleanup();
	delete xcs;

    return 0;
}
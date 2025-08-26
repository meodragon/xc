//
// Created by Khang on 8/23/2025.
//
//#include <memory>

#include <surface.h>
#include <grx.h>

int main(int argc, char *argv[]) {
    xcSurface *xcs = new xcSurface();

	unsigned int WIDTH = 640;
	unsigned int HEIGHT = 480;

	if (!xcs->init(WIDTH, HEIGHT)) {
        return -1;
    }

	xcGraphics *xcg = new xcGraphics(xcs);
	xcg->init(WIDTH, HEIGHT);

    xcs->run();

	xcg->cleanup();
	delete xcg;
    xcs->cleanup();
	delete xcs;

    return 0;
}
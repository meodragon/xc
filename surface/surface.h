//
// Created by Khang on 8/23/2025.
//

#ifndef SURFACE_H
#define SURFACE_H

#include <windows.h>
#include "resource.h"

#include <stdint.h>

class xcSurface {
    public:
		HINSTANCE hInstance;
		HWND hWnd;
		uint32_t width;
		uint32_t height;

		bool init();
        void run();
		void cleanup();
    private:

};
#endif //SURFACE_H

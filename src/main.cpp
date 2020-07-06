
#define _USE_MATH_DEFINES
#include "MainGui.hpp"
#include "functions.h"
#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>


int main(int argc, char* argv[]) {
	std::cout << "Kitty on your lap!!" << std::endl;
	try {
		nanogui::init();

		/* scoped variables */ {
			nanogui::ref<MainScreen> app = new MainScreen();
			app->drawAll();
			app->setVisible(true);
			nanogui::mainloop();
		}

		nanogui::shutdown();
	}
	catch (const std::runtime_error &e) {
		std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
#if defined(_WIN32)
		MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
#else
		std::cerr << error_msg << endl;
#endif
		return -1;
	}
	return 0;
}

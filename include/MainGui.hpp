#pragma once

#include <nlohmann/json.hpp>
#include <nanogui/nanogui.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

class MainScreen : public nanogui::Screen {
public:
	MainScreen() : nanogui::Screen(Eigen::Vector2i(900, 700), "NanoGUI Test", false) {

		nanogui::Window *window = new nanogui::Window(this, "GLCanvas Demo");
		window->setPosition(nanogui::Vector2i(15, 15));
		window->setLayout(new nanogui::GroupLayout());

		
		Widget *tools = new Widget(window);
		tools->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal,
			nanogui::Alignment::Middle, 0, 5));

		nanogui::Button *b4 = new nanogui::Button(tools, "Open Json");
		b4->setCallback([&] {
			std::string dialogResult = nanogui::file_dialog(
				{ {"json", "json file"} }, false);
			std::cout << "File dialog result: " << dialogResult << std::endl;
			if (dialogResult.length() > 0) {
				project = nlohmann::json();
				std::ifstream ifs(dialogResult);
				ifs >> project;
			}
		});

		nanogui::Button *b1 = new nanogui::Button(tools, "Open Ptx");
		b1->setCallback([&] {
			std::string dialogResult = nanogui::file_dialog(
				{ {"ptx", "ptx file"} }, false);
			std::cout << "File dialog result: " << dialogResult << std::endl;
			if (dialogResult.length() > 0) {
				project["ptx"] = dialogResult;
			}
		});

		nanogui::Button *b2 = new nanogui::Button(tools, "Generate Panorama");
		b2->setCallback([&] {
			std::string dialogResult = nanogui::file_dialog(
				{ {"png", "png file"} }, true);
			std::cout << "File dialog result: " << dialogResult << std::endl;
			if (dialogResult.length() > 0) {
				project["ptx"] = dialogResult;
			}
		});

		performLayout();


	}

	virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
		if (Screen::keyboardEvent(key, scancode, action, modifiers))
			return true;
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			setVisible(false);
			return true;
		}
		return false;
	}



	virtual void draw(NVGcontext *ctx) {
		/* Draw the user interface */
		Screen::draw(ctx);
	}
private:
	nlohmann::json project;

};

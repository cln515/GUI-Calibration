#pragma once

#include <nlohmann/json.hpp>
#include <nanogui/nanogui.h>
#include <image_utility/image_utility.h>
#undef ERROR
#include <ceres/ceres.h>
#include <functions.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>


class imageViewClicker :public nanogui::ImageView {
public:
	imageViewClicker(nanogui::Window* window, GLuint textureId) :nanogui::ImageView(window, textureId) {};

	virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override {
		nanogui::ImageView::mouseButtonEvent(p, button, down, modifiers);
		Eigen::Vector2f pf,off = offset(); pf << p(0)- positionF()(0), p(1) - positionF()(1);
		Eigen::Vector2f ip = imageCoordinateAt(pf);
		if (onKey && down) {
			cv::Point2f nearest;
			double ndist = -1;
			cv::Point2f clicked(ip(0), ip(1));
			for (auto itr = keypoints.begin(); itr != keypoints.end(); itr++) {
				double dist = cv::norm(*itr - clicked);
				if (ndist < 0 || dist < ndist) {
					ndist = dist;
					nearest = *itr;
				}
			}
			keepedNearest = nearest;
			std::cout << keepedNearest << std::endl;

		}
		
		return false;
	}

	virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
		if (nanogui::ImageView::keyboardEvent(key, scancode, action, modifiers))
			return true;
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			onKey = true;
			return true;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			onKey = false;
			return true;
		}

		return false;
	}
	vector<cv::Point2f> keypoints;
	cv::Point2f keepedNearest;
	bool onKey=false;
};



class MainScreen : public nanogui::Screen {
public:
	MainScreen() : nanogui::Screen(Eigen::Vector2i(1200, 1000), "NanoGUI Test", false) {

		nanogui::Window *window = new nanogui::Window(this, "GLCanvas Demo");
		window->setPosition(nanogui::Vector2i(15, 15));
		window->setLayout(new nanogui::GroupLayout());

		nanogui::Window* imageWindow = new nanogui::Window(this, "Selected image");
		imageWindow->setPosition(Vector2i(15, 75));
		imageWindow->setLayout(new nanogui::GroupLayout());

		// Set the first texture
		std::cout << mTextureId << std::endl;
	

		imageView = new imageViewClicker(imageWindow, mTextureId[0]);
		
		imageView->setFixedHeight(500);
		imageView->setFixedWidth(1000);


		nanogui::Window* imageWindow2 = new nanogui::Window(this, "Camera image");
		imageWindow2->setPosition(Vector2i(15, 75));
		imageWindow2->setLayout(new nanogui::GroupLayout());
		imageView2 = new imageViewClicker(imageWindow2, mTextureId[1]);
		imageView2->setFixedHeight(800);
		imageView2->setFixedWidth(800);

	
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
				if (!project["color"].is_null()) {
					std::cout << project["color"].get<std::string>() << std::endl;
					pano = cv::imread(project["color"].get<std::string>());
					genGLTextureFromCvMat(pano,0);
					std::cout << mTextureId << std::endl;
					imageView->nanogui::ImageView::bindImage(mTextureId[0]);
				}
			}
		});

		nanogui::Button *b3 = new nanogui::Button(tools, "Save Json");
		b3->setCallback([&] {
			std::string dialogResult = nanogui::file_dialog(
				{ {"json", "json file"} }, true);
			std::cout << "File dialog result: " << dialogResult << std::endl;
			if (dialogResult.length() > 0) {
				std::ofstream ofs(dialogResult);
				ofs<<project;
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
				{ {"png", "png file (color)"} }, true);
			std::cout << "File dialog result: " << dialogResult << std::endl;
			if (dialogResult.length() > 0) {
				std::string dialogResult2 = nanogui::file_dialog(
					{ {"png", "png file (depth)"} }, true);
				std::cout << "File dialog result: " << dialogResult2 << std::endl;
				if (dialogResult2.length() > 0) {
					cvtPTX2panorama(project["ptx"].get<std::string>(), dialogResult, dialogResult2);
					project["color"] = dialogResult;
					project["depth"] = dialogResult2;
					genGLTextureFromCvMat(cv::imread(dialogResult),0);
					imageView->bindImage(mTextureId[0]);
				}
			}
		});

		nanogui::Button *b5 = new nanogui::Button(tools, "FP Ext");
		b5->setCallback([&] {
			double minDistance = 10;
			vector<uchar> status;
			vector<float> err;
			vector<cv::Point2f> src, src2;
			cv::Mat image_g, pano_cp,cam_cp; pano_cp = pano.clone();
			cv::cvtColor(pano, image_g, CV_RGB2GRAY);
			cv::goodFeaturesToTrack(image_g, src, 0, 1e-5, minDistance, cv::noArray(), 3, true);
			for (auto itr = src.begin(); itr != src.end(); itr++) {
				cv::circle(pano_cp, *itr, 2, cv::Scalar(255, 0, 0, 255));
			}
			for (auto itr = panorama_pt.begin(); itr != panorama_pt.end(); itr++) {
				cv::circle(pano_cp, *itr, 2, cv::Scalar(0, 255, 0, 255));
			}
			imageView->keypoints = src;
			genGLTextureFromCvMat(pano_cp, 0);
			imageView->nanogui::ImageView::bindImage(mTextureId[0]);

			cv::cvtColor(cam, image_g, CV_RGB2GRAY);
			cv::goodFeaturesToTrack(image_g, src2, 0, 1e-5, minDistance, cv::noArray(), 3, true);
			cam_cp = cam.clone();
			for (auto itr = src2.begin(); itr != src2.end(); itr++) {
				cv::circle(cam_cp, *itr, 2, cv::Scalar(255, 0, 0, 255));
			}
			for (auto itr = camera_pt.begin(); itr != camera_pt.end(); itr++) {
				cv::circle(cam_cp, *itr, 2, cv::Scalar(0, 255, 0, 255));
			}

			imageView2->keypoints = src2;
			genGLTextureFromCvMat(cam_cp, 1);
			imageView2->nanogui::ImageView::bindImage(mTextureId[1]);
		});


		nanogui::Button *b6 = new nanogui::Button(tools, "Open Image");
		b6->setCallback([&] {
			std::string dialogResult = nanogui::file_dialog(
				{ {"*", "image file"} }, false);
			std::cout << "File dialog result: " << dialogResult << std::endl;
			if (dialogResult.length() > 0) {
				cam = cv::imread(dialogResult);
				genGLTextureFromCvMat(cam, 1);
				imageView2->bindImage(mTextureId[1]);
				cameraImage = dialogResult;
			}
		});

		nanogui::Button *b8 = new nanogui::Button(tools, "Open Camera Data");
		b8->setCallback([&] {
			std::string dialogResult = nanogui::file_dialog(
				{ {"json", "json file"} }, false);
			std::cout << "File dialog result: " << dialogResult << std::endl;
			if (dialogResult.length() > 0) {
				nlohmann::json cameras = nlohmann::json();
				std::ifstream ifs(dialogResult);
				ifs >> cameras;
				project["camera"] = cameras;
			}
		});

		nanogui::Button *b9 = new nanogui::Button(tools, "Multi Camera Calibration");
		b9->setCallback([&] {
			if (!project["camera"].is_null()) {
				nlohmann::json cameradat = project["camera"].get<nlohmann::json>();
				int camnum = cameradat.size();
				double lensParam[5][8];
				for (int camid = 0; camid < camnum; camid++) {
					std::string camidstr = std::to_string(camid);
					lensParam[camid][0] = cameradat[camidstr]["cx"];
					lensParam[camid][1] = cameradat[camidstr]["cy"];
					lensParam[camid][2] = cameradat[camidstr]["fx"];
					lensParam[camid][3] = cameradat[camidstr]["fx"];
					lensParam[camid][4] = cameradat[camidstr]["k1"];
					lensParam[camid][5] = cameradat[camidstr]["k2"];
					lensParam[camid][6] = cameradat[camidstr]["k3"];
					lensParam[camid][7] = cameradat[camidstr]["k4"];
				}

				int pos = 0;
				cv::Mat depth_og = cv::imread(project["depth"], CV_LOAD_IMAGE_UNCHANGED);
				cv::Mat depth(depth_og.rows, depth_og.cols, CV_32FC1, depth_og.data);
				ceres::Problem problem;

				double worldPos[10][6];
				double cameraPos[10][6];
				for (int x = 0; x < 10; x++) {
					for (int y = 0; y < 6; y++) {
						worldPos[x][y] = 0;
						cameraPos[x][y] = 0;
					}
					cameraPos[x][2] = M_PI;
				}
				
				while (true) {
					if (project["match"]["0-"+std::to_string(pos)]["fp_pano"].is_null())break;
					for (int camid = 0; camid < camnum; camid++) {
						std::stringstream ss; ss << camid << "-" << pos;
						
						std::vector<float> panofp = project["match"][ss.str()]["fp_pano"].get<std::vector<float>>();
						std::vector<float> camfp = project["match"][ss.str()]["fp_cam"].get<std::vector<float>>();

						Vector3d v;
						double d[8];
						//Matrix3d rot = axisRot2R(0, 0, -M_PI / 2);
						for (int j = 0; j < panofp.size()/2; j++) {
							cv::Point2f p(panofp.at(j*2), panofp.at(j * 2 + 1));
							rev_omniTrans(p.x, p.y, pano.cols, pano.rows, v);
							getSubPixel_float(depth, p, d);
							double dep = d[0] * d[1] + d[2] * d[3] + d[4] * d[5] + d[6] * d[7];
							v = 50.0 * dep * v;
							std::cout << v.transpose() << std::endl;
							Vector2d pix; pix << camfp.at(j * 2), camfp.at(j * 2 + 1);
							if (camid > 0) {
								ceres::CostFunction* c = new ceres::NumericDiffCostFunction < calibrationCostFunc, ceres::CENTRAL, 2, 6, 6>(
									new calibrationCostFunc(pix, v, lensParam[camid])
									);
								problem.AddResidualBlock(c, new ceres::CauchyLoss(0.1), worldPos[pos], cameraPos[camid - 1]);
							}
							else {
								ceres::CostFunction* c = new ceres::NumericDiffCostFunction < calibrationAnchorCostFunc, ceres::CENTRAL, 2, 6>(
									new calibrationAnchorCostFunc(pix, v, lensParam[camid])
									);
								problem.AddResidualBlock(c, new ceres::CauchyLoss(0.1), worldPos[pos]);
							}
						}
					}
					pos++;
				}
				ceres::Solver::Options options;
				options.max_num_iterations = 1e4;
				options.function_tolerance = 1e-7;
				options.parameter_tolerance = 1e-7;
				options.linear_solver_type = ceres::DENSE_QR;
				ceres::Solver::Summary summary;
				ceres::Solve(options, &problem, &summary);
				std::cout << summary.FullReport() << std::endl;
				_6dof resmot = { cameraPos[0][0],cameraPos[0][1],cameraPos[0][2],cameraPos[0][3],cameraPos[0][4],cameraPos[0][5] };
				_6dof resmot_ = { worldPos[0][0],worldPos[0][1],worldPos[0][2],worldPos[0][3],worldPos[0][4],worldPos[0][5] };
				_6dof resmot2_ = { worldPos[1][0],worldPos[1][1],worldPos[1][2],worldPos[1][3],worldPos[1][4],worldPos[1][5] };
				std::cout << resmot<< std::endl;
				std::cout << _6dof2m(resmot) << std::endl;
				std::cout << resmot_ << std::endl;
				std::cout << resmot2_ << std::endl;

			};
		});

		window = new nanogui::Window(this, "Grid of small widgets");
		window->setPosition(Vector2i(425, 300));
		nanogui::GridLayout *layout =
			new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2,
				nanogui::Alignment::Middle, 15, 5);
		layout->setColAlignment(
			{ nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		layout->setSpacing(0, 10);
		window->setLayout(layout);
		new nanogui::Label(window, "Camera id:", "sans-bold");
		nanogui::IntBox<int>* ib1 = new nanogui::IntBox<int>(window);
		ib1->setEditable(true);
		new nanogui::Label(window, "Position id:", "sans-bold");
		nanogui::IntBox<int>* ib2 = new nanogui::IntBox<int>(window);
		ib2->setEditable(true);

		nanogui::Button *b7 = new nanogui::Button(window, "Set Match");
		b7->setCallback([&,ib1,ib2] {
			nlohmann::json matchdata;
			std::vector<float> ptf,ptf2;
			for (auto itr = panorama_pt.begin(); itr != panorama_pt.end(); itr++) {
				ptf.push_back(itr->x); ptf.push_back(itr->y);
			}
			for (auto itr = camera_pt.begin(); itr != camera_pt.end(); itr++) {
				ptf2.push_back(itr->x); ptf2.push_back(itr->y);
			}
			matchdata["fp_pano"] = ptf;
			matchdata["fp_cam"] = ptf2;
			matchdata["filepath"]=cameraImage;
			std::stringstream ss; ss << ib1->value() << "-" << ib2->value();
			project["match"][ss.str()]=matchdata;
		});

		nanogui::Button *b10 = new nanogui::Button(window, "Load Match");
		b10->setCallback([&, ib1, ib2] {
			nlohmann::json matchdata;
			std::vector<float> ptf, ptf2;
			
			std::stringstream ss; ss << ib1->value() << "-" << ib2->value();
			ptf = project["match"][ss.str()]["fp_pano"].get<std::vector<float>>();
			ptf2 = project["match"][ss.str()]["fp_cam"].get<std::vector<float>>();

			panorama_pt.clear();
			camera_pt.clear();
			for (int i = 0; i < ptf.size()/2; i++) {
				panorama_pt.push_back(cv::Point2f(ptf.at(i*2), ptf.at(i * 2 + 1)));
				std::cout << cv::Point2f(ptf.at(i * 2), ptf.at(i * 2 + 1)) << std::endl;
			}
			for (int i = 0; i < ptf2.size() / 2; i++) {
				camera_pt.push_back(cv::Point2f(ptf2.at(i * 2), ptf2.at(i * 2 + 1)));
				std::cout << cv::Point2f(ptf2.at(i * 2), ptf2.at(i * 2 + 1)) << std::endl;
			}
			cameraImage = project["match"][ss.str()]["filepath"].is_null()?"":project["match"][ss.str()]["filepath"].get<std::string>();
			cam = cv::imread(cameraImage);
			if (cam.cols > 0) {
				genGLTextureFromCvMat(cam, 1);
				imageView2->bindImage(mTextureId[1]);
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
		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			panorama_pt.push_back(imageView->keepedNearest);
			camera_pt.push_back(imageView2->keepedNearest);
			std::cout << imageView->keepedNearest << "--" << imageView2->keepedNearest << std::endl;
			return true;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			if (panorama_pt.size() > 0) {
				std::cout << "Deleted :" << panorama_pt.at(panorama_pt.size() - 1) << "-" << camera_pt.at(camera_pt.size() - 1) << std::endl;
				panorama_pt.pop_back();
				camera_pt.pop_back();
			}

		}
		return false;
	}







	void genGLTextureFromCvMat(cv::Mat mat,int id) {
		if (mTextureId[id]) {
			glDeleteTextures(1, &mTextureId[id]);
			mTextureId[id] = 0;
		}
		glGenTextures(1, &mTextureId[id]);
		glBindTexture(GL_TEXTURE_2D, mTextureId[id]);
		int w = mat.size().width, h = mat.size().height, n = mat.channels();

		GLint internalFormat;
		GLint format;
		switch (n) {
		case 1: internalFormat = GL_R8; format = GL_RED; break;
		case 2: internalFormat = GL_RG8; format = GL_RG; break;
		case 3: internalFormat = GL_RGB8; format = GL_RGB; break;
		case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
		default: internalFormat = 0; format = 0; break;
		}
		std::cout << n << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE,mat.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	virtual void draw(NVGcontext *ctx) {
		/* Draw the user interface */
		Screen::draw(ctx);
	}
private:
	nlohmann::json project;
	nanogui::Window* imageWindow;
	GLuint mTextureId[5] = {0,0,0,0,0};
	cv::Mat pano,cam;
	imageViewClicker* imageView, *imageView2;
	std::vector<cv::Point2f> panorama_pt;
	std::vector<cv::Point2f> camera_pt;
	std::string cameraImage;
};

#include "functions.h"


void cvtPTX2panorama(std::string ptxFile) {
	BasicPly bp;
	bp.convertFAROColoredPtx2SpherePly(sf_);
	PanoramaRenderer renderer;
	int height = 2500;
	Matrix3d R = Matrix3d::Identity();
	Vector3d t; t << 0, 0, 0;
	Matrix4d m = Matrix4d::Identity();
	renderer.createContext(1024);
	renderer.setDepthFarClip(50.0);
	renderer.setDataRGB(bp.getVertecesPointer(), bp.getFaces(), bp.getRgbaPointer(), bp.getVertexNumber(), bp.getFaceNumber());
	renderer.renderColor(m);
	cv::Mat out(1024, 2048, CV_8UC3);
	out.data = (uchar*)renderer.getColorData();
	//cv::imwrite(argv[3], out);
	cv::cvtColor(out, out, CV_RGB2BGR);
	cv::flip(out, out, 0);
	//renderer.outputColor(argv[3]);
	cv::imwrite(argv[3], out);
	if (argc >= 5) {
		cv::Mat depth(1024, 2048, CV_32FC1);
		depth.data = (uchar*)renderer.getDepthData();
		float* depthfloat = (float*)depth.data;
		for (int i = 0; i < 1000; i++) {
			std::cout << depthfloat[i * 2048 + i];

		}
		cv::Mat saved_depth(depth.rows, depth.cols, CV_8UC4, depth.data);
		cv::flip(saved_depth, saved_depth, 0);

		cv::imwrite(argv[4], saved_depth);
	}


}
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <Eigen/Core>
#include <basicPly/BasicPly.h>
#include <opencv2/opencv.hpp>
#include <PanoramaRenderer/PanoramaRenderer.h>

void cvtPTX2panorama(std::string ptxFile, std::string colorOut, std::string depthOut);

struct calibrationCostFunc {
public:
	calibrationCostFunc(Vector2d& pix_, Vector3d& p_,double* lensParam)
	{
		params = lensParam;
		pix = pix_;
		p = p_;
	};
	bool operator()(const double* parameters, const double* parameters2, double* residual) const {

		//calibration: camera_base <-- world
		double rx = parameters[0];
		double ry = parameters[1];
		double rz = parameters[2];
		double x = parameters[3];
		double y = parameters[4];
		double z = parameters[5];

		//calibration: camera_current <-- camera_base
		double rx2 =  parameters2[0];
		double ry2 = parameters2[1];
		double rz2 = parameters2[2];
		double x2 = parameters2[3];
		double y2 = parameters2[4];
		double z2 = parameters2[5];

		//camera_base<--world
		Matrix3d R = axisRot2R(rx, ry, rz);
		Vector3d t; t << x, y, z;

		//camera_current <-- camera_base
		Matrix3d R2 = axisRot2R(rx2, ry2, rz2);
		Vector3d t2; t2 << x2, y2, z2;

		//projection error
		Vector3d plot = R2.transpose() * (R.transpose() * (p - t) - t2);

		double u, v;
		FisheyeTransCV(plot(0), plot(1), plot(2), u, v, params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7]);
		residual[0] = u-pix(0);
		residual[1] = v-pix(1);
		return true;
	}
private:
	Vector3d p;
	Vector2d pix;
	double* params;
};

struct calibrationAnchorCostFunc {
public:
	calibrationAnchorCostFunc(Vector2d& pix_, Vector3d& p_, double* lensParam)
	{
		params = lensParam;
		pix = pix_;
		p = p_;
	};
	bool operator()(const double* parameters, double* residual) const {

		//calibration: camera_base <-- world
		double rx = parameters[0];
		double ry = parameters[1];
		double rz = parameters[2];
		double x = parameters[3];
		double y = parameters[4];
		double z = parameters[5];


		//camera_base<--world
		Matrix3d R = axisRot2R(rx, ry, rz);
		Vector3d t; t << x, y, z;


		//projection error
		Vector3d plot = R.transpose() *( p  - t);

		double u, v;
		FisheyeTransCV(plot(0), plot(1), plot(2), u, v, params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7]);
		residual[0] = u - pix(0);
		residual[1] = v - pix(1);
		return true;
	}
private:
	Vector3d p;
	Vector2d pix;
	double* params;
};


struct LiDARCost {
public:
	LiDARCost(Matrix4d& trans)
	{
		Matrix3d r = trans.block(0, 0, 3, 3);
		ql = dcm2q(r);
		tl = trans.block(0, 3, 3, 1);
	};
	bool operator()(const double* parameters, const double* parameters2, double* residual) const {

		//calibration: camera_base <-- world
		double rx = parameters[0];
		double ry = parameters[1];
		double rz = parameters[2];
		double x = parameters[3];
		double y = parameters[4];
		double z = parameters[5];

		//calibration: LiDAR_current <-- camera_base
		double rx2 = parameters2[0];
		double ry2 = parameters2[1];
		double rz2 = parameters2[2];
		double x2 = parameters2[3];
		double y2 = parameters2[4];
		double z2 = parameters2[5];

		//camera_base<--world
		Matrix3d R = axisRot2R(rx, ry, rz);
		Vector3d t; t << x, y, z;

		//camera_current <-- camera_base
		Matrix3d R2 = axisRot2R(rx2, ry2, rz2);
		Vector3d t2; t2 << x2, y2, z2;

		//transformation

		Vector3d tcomp = t + R * t2;
		Matrix3d RR2 = R * R2;
		Vector4d qcomp = dcm2q(RR2);


		residual[0] = w1 * (tl(0) - tcomp(0));
		residual[1] = w1 * (tl(1) - tcomp(1));
		residual[2] = w1 * (tl(2) - tcomp(1));
		residual[3] = w2 * (ql(0) - qcomp(0));
		residual[4] = w2 * (ql(1) - qcomp(1));
		residual[5] = w2 * (ql(2) - qcomp(2));
		residual[6] = w2 * (ql(3) - qcomp(3));
		return true;
	}
	double w1=10;
	double w2=100;
private:
	Vector3d tl;
	Vector4d ql;
};
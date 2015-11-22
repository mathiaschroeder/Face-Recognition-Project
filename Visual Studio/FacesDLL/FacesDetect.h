#pragma once

#include "FacesBase.h"
#include "FacesException.h"
#include <opencv2/opencv.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <direct.h>
#include <sstream>
#include <utility>
#include <string>
#include <sstream>
#include "vector"

using namespace cv;

class FacesDetect : public FacesBase {
	public:
		FacesDetect();
		~FacesDetect();

		int facesToPgm();
		int facesToPgm(string _imageFolder);
		int facesToPgm(string _imageFolder, string _imageName);
		int saveAsPgm(Mat _image, string _imagePath, string _imageName);
		Mat encodeAsPgm(Mat _image);
		vector<Mat> getFaceCropped(string _imagePath, string _imageName);
	private:
		void rotate(Mat& src, double angle, Mat& dst);
};


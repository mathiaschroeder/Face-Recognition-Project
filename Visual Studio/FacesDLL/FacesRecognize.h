#pragma once
#include "FacesBase.h"
#include "FacesDetect.h"

#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>

using namespace cv;
using namespace std;

#define MODEL_NAME "eigenModel.yml"

class FacesRecognize : public FacesBase {

public:
	struct facesDir{
		vector<Mat> faces;
		vector<int> labels;
	};

	FacesRecognize();
	~FacesRecognize();
	void train();
	vector<string> predict(string _imagePath);

private:

	Ptr<FaceRecognizer> eigenRecognizer = nullptr;
	map<int, string> facesMap;
	const static int LOADING = 0;
	const static int MAPPING = 1;

	void setFacesMap(map<int, string> _facesMap);
	map<int, string> getFacesMap();
	facesDir loadFacesFromDir(int _flag = LOADING);
	Ptr<FaceRecognizer> getEigenRecognizer();

};


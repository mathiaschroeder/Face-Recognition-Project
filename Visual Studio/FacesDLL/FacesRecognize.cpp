#include "stdafx.h"
#include "FacesRecognize.h"
#include "FacesException.h"

FacesRecognize::FacesRecognize(){}

FacesRecognize::~FacesRecognize(){}

FacesRecognize::facesDir FacesRecognize::loadFacesFromDir(int _flag){

	int labelsCount = 0;
	string pgmDir, dirName;
	vector<string> dirFaces, pgmFiles, imgType;
	map<int, string> labelXdir;
	vector<Mat> faces;
	vector<int> labels;
	facesDir facesDirRet;

	//Captura diretorio de imagens PGM
	pgmDir = getConfig()->getConfigValue("dirPgm");

	//Captura todos os no caminho de diretorios PGM
	dirFaces = readDir(pgmDir);

	if (dirFaces.size() == 0)
		throw FacesException("Nenhuma imagem encontrada para treinamento");

	if (dirFaces.size() > 0){
		//Percorre os diretorios
		for (int iFaces = 0; iFaces < dirFaces.size(); iFaces++){

			dirName = pgmDir + dirFaces.at(iFaces);
			//ler somente arquivos de extensao pgm
			imgType.push_back("pgm");
			pgmFiles = readDir(dirName, imgType);

			//Percorre as imagens no diretorio
			for (int iPgm = 0; iPgm < pgmFiles.size(); iPgm++){

				if (_flag == LOADING){
					//Carrega as imagens e labels
					dirName = pgmDir + dirFaces.at(iFaces) + "\\" + pgmFiles.at(iPgm);
					Mat image = imread(dirName, 0);
					if (!image.data)
						throw FacesException("Erro ao carregar imagem: " + dirName + " durante treinamento.");

					faces.push_back(image);
					labels.push_back(labelsCount);
				}
				labelXdir[labelsCount] = dirFaces.at(iFaces);
				labelsCount++;
			}
		}
	}

	if (faces.size() > 0){
		facesDirRet.faces = faces;
		facesDirRet.labels = labels;
	}

	setFacesMap(labelXdir);

	return facesDirRet;
}

void FacesRecognize::setFacesMap(map<int, string> _facesMap){
	facesMap = _facesMap;
}

map<int, string> FacesRecognize::getFacesMap(){
	return facesMap;
}

Ptr<FaceRecognizer> FacesRecognize::getEigenRecognizer(){

	if (eigenRecognizer == nullptr)
		eigenRecognizer = createEigenFaceRecognizer();

	return eigenRecognizer;
}

void FacesRecognize::train(){

	facesDir facesDir = loadFacesFromDir();
	getEigenRecognizer()->train(facesDir.faces, facesDir.labels);

	//Captura diretório para salvar o modelo gerado
	string dirModel = getConfig()->getConfigValue("dirTrainModel");
	string fileModel = dirModel + MODEL_NAME;
	getEigenRecognizer()->save(dirModel + MODEL_NAME);

	if (!fileExists(fileModel))
		throw FacesException("Erro ao gerar aquivo de treinamento: " + fileModel);
}

vector<string> FacesRecognize::predict(string _imagePath){

	string imagePath, imageName, dirPredicted;
	vector<Mat> faces;
	Mat imgPgm, imgGray;
	int predict = 0;
	double confidence = 0.0;
	vector<string> rtnPredict;

	//Captura o modelo de treino
	string dirModel = getConfig()->getConfigValue("dirTrainModel");
	if (fileExists(dirModel + MODEL_NAME))
		getEigenRecognizer()->load(dirModel + MODEL_NAME);
	else
		//Se nao encontrar efetua o treinamento
		train();

	//Captura a face da imagem
	imagePath = _imagePath.substr(0, _imagePath.find_last_of("\\") + 1);
	imageName = _imagePath.substr(_imagePath.find_last_of("\\") + 1, _imagePath.length());
	FacesDetect *facesDetect = new FacesDetect();

	//Verifica se a imagem já está no formato PGM
	if (imageName.find(".pgm") != string::npos){
		imgPgm = imread(imagePath + imageName, 1);
		if (!imgPgm.data)
			throw FacesException("Erro ao ler imagem: " + imagePath + imageName);
		else {
			cvtColor(imgPgm, imgGray, CV_BGR2GRAY);
			imgPgm = imgGray;
		}
	}
	else {
		//Captura a face na imagem
		faces = facesDetect->getFaceCropped(imagePath, imageName);

		//Converte a imagem para PGM
		if (faces.size() > 0)
			imgPgm = facesDetect->encodeAsPgm(faces[0]);
		else
			throw FacesException("Nenhuma face encontrada na imagem de teste.");
	}

	//Verifica se a imagem existe no banco de faces
	getEigenRecognizer()->predict(imgPgm, predict, confidence);

	//Carrega o mapa de faces
	loadFacesFromDir(MAPPING);

	//Procura a pasta correspondente da label referente a imagem encontrada
	map<int, string> itMap = getFacesMap();
	map<int, string>::iterator it;
	for (it = itMap.begin(); it != itMap.end(); ++it){
		if (it->first == predict){
			dirPredicted = it->second.c_str();
			break;
		}
	}

	rtnPredict.push_back(dirPredicted);
	rtnPredict.push_back(to_string(confidence));

	return rtnPredict;
}


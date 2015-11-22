#pragma once

#include "stdafx.h"
#include "FacesDetect.h"

#define PI 3.14159265

FacesDetect::FacesDetect(){}

FacesDetect::~FacesDetect(){}

int FacesDetect::facesToPgm(){
	return facesToPgm("", "");
}

int FacesDetect::facesToPgm(string _imageFolder){
	return facesToPgm(_imageFolder, "");
}

int FacesDetect::facesToPgm(string _imageFolder, string _imageName){

	vector<Mat> faces;
	int currentDir;
	size_t dirCount, imgCount = 0;
	vector<string> arquivos, imgTypes, diretorios;
	string dirRcb, dirPgm, dirPrc, dirSave, dirNew, dirOld, strTypes;

	dirRcb = getConfig()->getConfigValue("dirRcb");
	dirPgm = getConfig()->getConfigValue("dirPgm");
	dirPrc = getConfig()->getConfigValue("dirPrc");
	strTypes = getConfig()->getConfigValue("imgTypes");

	istringstream iss(strTypes);
	for (string token; getline(iss, token, ',');){
		imgTypes.push_back(move(token));
	}

	if (!_imageName.empty()){
		//Le somente o arquivo informado
		arquivos.push_back(_imageName);
	}
	else{
		//Le os arquivos recebidos
		diretorios = readDir(dirRcb, imgTypes);
		if (diretorios.size() == 0)
			throw FacesException("Nenhum arquivo encontrado em arquivos recebidos");
	}

	for (size_t loopDir = 0; loopDir < diretorios.size(); loopDir++){

		arquivos = readDir(dirRcb + "\\" + diretorios.at(loopDir) + "\\", imgTypes);

		if (arquivos.size() > 0){

			if (_imageFolder.empty()){
				//Cria uma nova pasta
				dirCount = readDir(dirPgm).size() + 1;
				dirSave = dirPgm + diretorios.at(loopDir) + "\\";
				if (_mkdir(dirSave.c_str()) != 0)
					throw FacesException("Erro ao criar diretorio: " + dirSave);
				currentDir = dirCount;
			}
			else {
				//Utiliza a pasta informada
				dirSave = dirPgm + _imageFolder + "\\";
				imgCount = readDir(dirSave).size() + 1;
				currentDir = atoi(_imageFolder.c_str());
			}

			//Itera sobre os arquivos e converte para PGM
			for (size_t loopImg = 0; loopImg < arquivos.size(); loopImg++)
			{
				try {
					faces = getFaceCropped(dirRcb + "\\" + diretorios.at(loopDir) + "\\", arquivos[loopImg]);

					if (faces.size() > 0)
					{
						//Salva a face no diretorio
						for (size_t loopFace = 0; loopFace < faces.size(); loopFace++){

							if (saveAsPgm(faces[loopFace], dirSave, arquivos[loopImg]) != 0){
								throw FacesException("Erro gerar arquivo PGM no diretório: " + dirSave);
							}

						}
					}

				}
				catch (FacesException e){
					if (!dirPrc.empty()){

						imgCount++;
						dirOld = dirRcb + diretorios.at(loopDir) + "\\" + arquivos[loopImg];
						dirNew = dirPrc + arquivos[loopImg];

						//Se tem diretorio de processado move para lá
						if (rename(dirOld.c_str(), dirNew.c_str()) != 0){}
					}
				}
			}
		}
	}
	return currentDir;
}

vector<Mat> FacesDetect::getFaceCropped(string _imagePath, string _imageName)
{
	Mat img, frameGray, rotated, cropped, resized, tmp;
	float x, y, arc_angle;
	vector<Mat> returnFaces;
	vector<Rect> faces, eyes;
	CascadeClassifier faceCascade, eyes_cascade;
	bool debug = false;

	if (_imageName.find(".pgm") != string::npos){

		img = imread(_imagePath + _imageName, 1);
		if (!img.data)
			throw FacesException("Erro ao ler imagem: " + _imagePath + _imageName);

		//Converte para cinza 
		cvtColor(img, frameGray, CV_BGR2GRAY);
		returnFaces.push_back(frameGray);

	}
	else {
		//Carrega XML para detecao de faces
		if (!faceCascade.load("haarcascade_frontalface_alt.xml"))
			throw FacesException("Arquivo haarcascade_frontalface_alt.xml nao encontrado");

		//Carrega XML para detecao dos olhos e fazer o alinhamento da face
		if (!eyes_cascade.load("haarcascade_eye_tree_eyeglasses.xml"))
			throw FacesException("Arquivo haarcascade_eye_tree_eyeglasses.xml nao encontrado");

		//Le a imagem
		img = imread(_imagePath + _imageName, 1);
		if (!img.data)
			throw FacesException("Erro ao ler imagem: " + _imagePath + _imageName);

		//Redimensiona a imagem, fazendo com que a soma de suas dimensões seja menor que mil
		// mantendo sua proporção
		if (img.rows + img.cols > 1000){

			float newWidth, newHeight, total;
			total = img.rows + img.cols;
			newWidth = (img.cols / total) * (total - 1000);
			newHeight = (img.rows / total) * (total - 1000);

			resize(img, resized, cvSize(img.cols - newWidth,
				img.rows - newHeight));
			img = resized;
		}

		//Converte para cinza 
		cvtColor(img, frameGray, CV_BGR2GRAY);

		//Verifica se a imagem contém faces
		faceCascade.detectMultiScale(frameGray,  //Imagem a ser processada
			faces,  //Vetor com as coordenadas das faces encontradas
			1.05,  //Scale factor, o quanto o retângulo diminui a cada iteração na imagem
			3,  //MinNeighbour, 
			0 | CV_HAAR_SCALE_IMAGE,  //
			Size(50, 50));//Tamanho mínno da face 30 por 30

		if (debug){
			imshow("a", frameGray);
			while (true){
				int c = waitKey(10);
				if ((char)c == 'c') { break; }
			}
		}

		//Nenhuma imagem encontrada
		if (faces.size() < 1)
			throw FacesException("Nenhuma face encontrada na imagem: " + _imageName);

		for (int i = 0; i < faces.size(); i++){

			//Cria um retangulo do tamanho e posição da face encontrada
			Rect facerect = Rect(faces.at(i).x, faces.at(i).y, faces.at(i).width, faces.at(i).height);

			//Para fins de debug... 
			try {
				rectangle(img,
					facerect,
					Scalar(255, 255, 51), 2, 8, 0);
				imwrite("C:\\Users\\Sociesc\\Desktop\\outOpenCV\\" + _imageName, img);
			}
			catch (runtime_error& ex) {/*Do nothing...*/ }

			if (faces.size() > 1)
				throw FacesException("A imagem '" + _imageName + "' deve conter apenas uma face!");

			//Gera uma nova imagem a partir deste retangulo
			cropped = frameGray(facerect).clone();

			//Detecta os olhos
			eyes_cascade.detectMultiScale(cropped, eyes, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

			if (debug){
				imshow("a", cropped);
				while (true){
					int c = waitKey(10);
					if ((char)c == 'c') { break; }
				}
			}


			//Alinha a imagem, deixando a reta que liga os dois olhos com angulo 0
			if (eyes.size() >= 2){
				if (abs(eyes.at(0).y - eyes.at(1).y) > 8){
					y = (eyes.at(0).y - eyes.at(1).y);
					x = (eyes.at(0).x - eyes.at(1).x);
					arc_angle = atan(y / x) * (180 / PI);
					arc_angle += 2;
					rotate(frameGray, arc_angle, rotated);
					frameGray = rotated;

					if (debug){
						imshow("a", frameGray);
						while (true){
							int c = waitKey(10);
							if ((char)c == 'c') { break; }
						}
					}

					//Captura a face novamente na imagem rotacionanda
					//Verifica se a imagem contém faces
					faceCascade.detectMultiScale(frameGray, //Imagem a ser processada
						faces, //Vetor com as coordenadas das faces encontradas
						1.05,  //Scale factor, o quanto o retângulo diminui a cada iteração na imagem
						3,  //MinNeighbour, 
						0 | CV_HAAR_SCALE_IMAGE,  //
						Size(50, 50));  //Tamanho mínno da face 30 por 30

					//Nenhuma imagem encontrada
					if (faces.size() < 1)
						throw FacesException("Nenhuma face encontrada na imagem: " + _imageName);

					tmp = frameGray.clone();
					eyes_cascade.detectMultiScale(tmp, eyes, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

					if (eyes.size() > 1){

						facerect.x = min(eyes.at(0).x, eyes.at(1).x) - 20;
						facerect.width = max(eyes.at(0).x, eyes.at(1).x) -
							min(eyes.at(0).x, eyes.at(1).x) +
							eyes.at(1).width + 40;

						Rect eye1 = Rect(eyes.at(0).x, eyes.at(0).y, eyes.at(0).width, eyes.at(0).height);
						rectangle(tmp,
							eye1,
							Scalar(51, 255, 255), 2, 8, 0);
						Rect eye2 = Rect(eyes.at(1).x, eyes.at(1).y, eyes.at(1).width, eyes.at(1).height);
						rectangle(tmp,
							eye2,
							Scalar(51, 255, 255), 2, 8, 0);
						rectangle(tmp,
							facerect,
							Scalar(51, 255, 255), 2, 8, 0);
						if (debug){
							imshow("a", tmp);
							while (true){
								int c = waitKey(10);
								if ((char)c == 'c') { break; }
							}
						}
						//Para fins de debug... 
						try {
							imwrite("C:\\Users\\Sociesc\\Desktop\\outOpenCV\\_" + _imageName, tmp);
						}
						catch (runtime_error& ex) {/*Do nothing...*/ }
					}

					//Gera uma nova imagem a partir deste retangulo
					cropped = frameGray(facerect).clone();

					if (debug){
						imshow("a", cropped);
						while (true){
							int c = waitKey(10);
							if ((char)c == 'c') { break; }
						}
					}

				}
			} 

			//Redimensiona para o tamanho padrão
			resize(cropped, resized, cvSize(92, 112));

			//Adiciona a face recortada para o vetor de retorno
			returnFaces.push_back(resized);
		}
	}
	return returnFaces;
}

void FacesDetect::rotate(Mat& src, double angle, Mat& dst){

	int len = max(src.cols, src.rows);
	Point2f pt(len / 2., len / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);

	warpAffine(src, dst, r, Size(len, len));
}

int FacesDetect::saveAsPgm(Mat _image, string _imagePath, string _imageName){

	vector<int> compression_params;

	//Carrega parametros de compressao para formato PGM
	compression_params.push_back(CV_IMWRITE_PXM_BINARY);
	compression_params.push_back(0);

	try {
		imwrite(_imagePath + _imageName + ".pgm", _image, compression_params);

	}
	catch (runtime_error& ex) {
		string what = ex.what();
		throw FacesException("Erro ao salvar imagem: " + what);
	}

	return 0;
}

Mat FacesDetect::encodeAsPgm(Mat _image){

	Mat imgEncoded;
	vector<uchar> buff;//buffer for coding

	//Carrega parametros de compressao para formato PGM
	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PXM_BINARY);
	compression_params.push_back(0);

	if (!imencode(".pgm", _image, buff, compression_params))
		throw FacesException("Erro ao codificar imagem para PGM");

	return _image;
}
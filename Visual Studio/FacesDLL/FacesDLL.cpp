#include "stdafx.h"
#include "FacesDLL.h"

using namespace std;

#define DLL_EXPORT

extern "C"
{
	DECLDIR const char* facesToPgm(char *dirId, char *imgName){
		
		string message = "";
		int ret;
		map<string, string> jsonReturn;
		FacesDetect *facesFacesDetect = new FacesDetect();

		try {
			string strDirId(dirId);
			string strImgName(imgName);
			if (strDirId.empty() && strImgName.empty()){
				ret = facesFacesDetect->facesToPgm();
				jsonReturn["status"] = "OK";
				jsonReturn["message"] = "Imagens convertidas com sucesso";
				jsonReturn["dirId"] = to_string(ret);

			}
			else if (strDirId.empty()){
				ret = facesFacesDetect->facesToPgm("", strImgName);
				jsonReturn["status"] = "OK";
				jsonReturn["message"] = "Face adicionada com sucesso";
				jsonReturn["dirId"] = to_string(ret);

			} else {
				ret = facesFacesDetect->facesToPgm(strDirId, strImgName);
				jsonReturn["status"] = "OK";
				jsonReturn["message"] = "Face adicionada com sucesso";
				jsonReturn["dirId"] = to_string(ret);
			}

		} catch (exception e){
			jsonReturn["status"] = "ERRO";
			jsonReturn["message"] = e.what();

		} catch (FacesException e){
			jsonReturn["status"] = "ERRO";
			jsonReturn["message"] = e.what();

		} catch (...){
			jsonReturn["status"] = "ERRO";
			jsonReturn["message"] = "Erro desconhecido.";
		}

		message = facesFacesDetect->toJSON(jsonReturn);
		message += "\0";

		char *ptrPgm = (char*)malloc(message.length()*sizeof(char));
		strcpy(ptrPgm, message.c_str());

		delete facesFacesDetect;
		return ptrPgm;
	}
	
	DECLDIR const char* train(){
		
		string message = "";
		map<string, string> jsonReturn;
		FacesRecognize *facesRecognize = new FacesRecognize();

		try {
			facesRecognize->train();
			jsonReturn["status"] = "OK";
			jsonReturn["message"] = "Treinamento efetuado com sucesso";

		} catch (exception e){
			jsonReturn["status"] = "ERRO";
			jsonReturn["message"] = e.what();

		} catch (FacesException e){
			jsonReturn["status"] = "ERRO";
			jsonReturn["message"] = e.what();

		} catch (...){
			jsonReturn["status"] = "ERRO";
			jsonReturn["message"] = "Erro desconhecido.";
		}	

		message = facesRecognize->toJSON(jsonReturn);
		message += "\0";
		char *ptrTrain = (char*)malloc(message.length()*sizeof(char));
		strcpy(ptrTrain, message.c_str());

		delete facesRecognize;
		return ptrTrain;
	}

	DECLDIR const char* predict(char *str)
	{		
		string message = "";
		map<string, string> jsonReturn;
		FacesRecognize *facesRecognize = new FacesRecognize();

		try {
			vector<string> ret = facesRecognize->predict(str);
			jsonReturn["status"] = "OK";
			jsonReturn["message"] = "Reconhecimento efetuado com sucesso.";
			jsonReturn["dirId"] = ret.at(0);
			
		} catch (exception e){
			jsonReturn["status"] = "ERRO";
			jsonReturn["message"] = e.what();

		} catch (FacesException e){
			jsonReturn["status"] = "ERRO";
			jsonReturn["message"] = e.what();

		} catch (...){
			jsonReturn["status"] = "ERRO";
			jsonReturn["message"] = "Erro desconhecido.";
		}

		message = facesRecognize->toJSON(jsonReturn);
		message += "\0";
		char *ptrPredict = (char*)malloc(message.length()*sizeof(char));
		strcpy(ptrPredict, message.c_str());

		delete facesRecognize;
		return ptrPredict;
	}
}


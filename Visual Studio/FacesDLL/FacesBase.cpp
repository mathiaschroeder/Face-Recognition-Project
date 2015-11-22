#include "stdafx.h"
#include "FacesBase.h"

string FacesBase::toJSON(map<string, string> _keyValue){

	int i = 1;
	string tmpStr;

	string strJSON = "{";
	map<string, string>::iterator it;
	for (it = _keyValue.begin(); it != _keyValue.end(); ++it){

		tmpStr = it->second;

		//Remove trailing backslahes
		while (tmpStr.rbegin() != tmpStr.rend() && *tmpStr.rbegin() == '\\')
			tmpStr.pop_back();

		auto it2 = find(tmpStr.begin(), tmpStr.end(), '\\');
		while (it2 != tmpStr.end()) {
			auto it3 = tmpStr.insert(it2, '\\');

			// skip over the slashes we just inserted
			it2 = find(it2 + 2, tmpStr.end(), '\\');
		}

		strJSON += "\"" + it->first + "\" : " + "\"" + tmpStr + "\"";
		if (i < _keyValue.size())
			strJSON += ",";
		i++;
	}
	strJSON += "}";

	return strJSON;
}

FacesBase::FacesBase(){

	if (getConfig()->getConfigValue("dirRcb").size() < 1)
		throw FacesException("Diretorio de arquivos (dirRcb) recebidos nao parametrizado!");

	if (getConfig()->getConfigValue("dirPrc").size() < 1)
		throw FacesException("Diretorio de arquivos (dirPrc) processados nao parametrizado!");

	if (getConfig()->getConfigValue("dirPgm").size() < 1)
		throw FacesException("Diretorio de arquivos PGM (dirPgm) nao parametrizado!");

	if (getConfig()->getConfigValue("dirTrainModel").size() < 1)
		throw FacesException("Diretorio para modelo de treinamento (dirTrainModel) nao parametrizado!");

	if (getConfig()->getConfigValue("imgTypes").size() < 1)
		throw FacesException("Tipos de imagens (imgTypes) nao parametrizado!");
}

FacesBase::~FacesBase(){}

GlobalConfig* FacesBase::getConfig(){

	if (config == nullptr)
		config = new GlobalConfig();

	return config;
}

vector<string> FacesBase::readDir(string _dirPath){

	vector<string> fileType;
	return readDir(_dirPath, fileType);
}

bool FacesBase::fileExists(string _fileName){

	ifstream ifs(_fileName.c_str(), ifstream::in);
	if (ifs.good())
		return true;
	else
		return false;
}

vector<string> FacesBase::readDir(string _dirPath, vector<string> _fileType){

	struct dirent *ptrDirStruct;
	vector<string> arquivos;
	string nome;
	DIR *ptrDir;

	if ((ptrDir = opendir(_dirPath.c_str())) != NULL) {

		while ((ptrDirStruct = readdir(ptrDir)) != NULL) {

			nome = ptrDirStruct->d_name;

			//Diretorio atual ou anterior desconsidera
			if (nome == ".." || nome == ".")
				continue;

			//Busca com extensão
			if (_fileType.size() > 0){
				for (int i = 0; i < _fileType.size(); i++){
					//Se não for alguma das extensões parametrizadas, ignora o arquivo
					if (nome.find("." + _fileType[i]) != string::npos){
						arquivos.push_back(nome);
						break;
					}
				}
				//Busca tudo
			}
			else
				arquivos.push_back(nome);
		}
		delete ptrDirStruct;
		closedir(ptrDir);
	}
	else
		throw FacesException("Erro ao abrir diretorio: " + _dirPath);

	return arquivos;
}

#pragma once

#include "GlobalConfig.h"
#include "FacesException.h"
#include <dirent.h>
#include "vector"
#include <fstream>
#include <map>

class FacesBase{

public:

	FacesBase();
	~FacesBase();

	GlobalConfig* getConfig();
	string toJSON(map<string, string> _keyValue);
	
protected: //Métodos visíveis somente por classes filhas
	bool fileExists(string _fileName);
	vector<string> readDir(string _dirPath, vector<string> _fileType);
	vector<string> readDir(string _dirPath);

private:
	GlobalConfig *config = nullptr;

};

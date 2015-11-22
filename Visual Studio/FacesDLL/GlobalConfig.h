#pragma once

#include <iostream>
#include <string.h>
#include <pugixml.hpp>

using namespace pugi;
using namespace std;

#define XML_FILE_NAME "config.xml"

class GlobalConfig{

public:

	//Métodos
	GlobalConfig();
	~GlobalConfig();

	string getConfigValue(string _configName);
	bool setConfigValue(string _configName, string _configValue);

private:

	//Atributos
	string xmlStr;
	struct xml_string_writer : xml_writer
	{
		string result;
		virtual void write(const void* data, size_t size)
		{
			result += std::string(static_cast<const char*>(data), size);
		}
	};

	//Métodos
	string getXmlStr();
	void setXmlStr(string _xmlStr);
	void initialize();
};


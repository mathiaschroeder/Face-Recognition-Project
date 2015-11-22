#include "stdafx.h"
#include "GlobalConfig.h"
#include "FacesException.h"

GlobalConfig::GlobalConfig(){
	initialize();
}

GlobalConfig::~GlobalConfig(){}

void GlobalConfig::initialize(){

	xml_document xmlDoc;
	xml_string_writer writer;

	xml_parse_result result = xmlDoc.load_file(XML_FILE_NAME);
	if (result.status != xml_parse_status::status_ok)
		throw FacesException("Erro ao serializar arquivo 'config.xml', verifique se o mesmo existe e esta com a estrutura correta");

	xmlDoc.save(writer);

	setXmlStr(writer.result);

}

string GlobalConfig::getXmlStr(){
	return xmlStr;
}

void GlobalConfig::setXmlStr(string _xmlStr){

	if (_xmlStr.empty())
		throw FacesException("String XML vazia");

	xmlStr = _xmlStr;
}

string GlobalConfig::getConfigValue(string _configName){

	string value;
	xml_document xmlDoc;

	xml_parse_result result = xmlDoc.load(getXmlStr().c_str());
	if (result.status != xml_parse_status::status_ok)
		throw FacesException("Erro ao carregar XML");

	xml_node settings = xmlDoc.child("configuration").child("appSettings");
	if (!settings)
		throw FacesException("Tag 'configuration' ou 'appSettings' nao encontrada");

	for (xml_node node = settings.child("add"); node; node = node.next_sibling("add")){

		if (node.attribute("key").value() == _configName){
			value = node.attribute("value").value();
		}
	}

	if (value.empty())
		throw FacesException("Chave " + _configName + " nao encontrada no XML");

	return value;
}

bool GlobalConfig::setConfigValue(string _configName, string _configValue){

	xml_document xmlDoc;
	xml_string_writer writer;

	xml_parse_result result = xmlDoc.load(getXmlStr().c_str());
	if (result.status != xml_parse_status::status_ok)
		throw FacesException("Erro ao carregar XML");

	xml_node settings = xmlDoc.child("configuration").child("appSettings");
	if (!settings)
		throw FacesException("Tag 'configuration' ou 'appSettings' nao encontrada");

	for (xml_node node = settings.child("add"); node; node = node.next_sibling("add")){

		if (node.attribute("key").value() == _configName){
			node.attribute("value").set_value(_configValue.c_str());
		}
	}

	xmlDoc.save(writer);
	setXmlStr(writer.result);

	return xmlDoc.save_file(XML_FILE_NAME);
}

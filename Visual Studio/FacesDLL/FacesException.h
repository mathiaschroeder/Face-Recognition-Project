#pragma once

#include <exception>
#include <string>
using namespace std;

class FacesException
{
public:
	FacesException(string _msg){

		if (_msg.empty())
			_msg = "FacesException nao tratada";

		err_msg = _msg;
	};

	const char *what() const throw() {
		return this->err_msg.c_str();
	};

private:
	string err_msg;
};


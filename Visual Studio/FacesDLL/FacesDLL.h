#ifndef _DLL_TUTORIAL_H_
#define _DLL_TUTORIAL_H_

#include "FacesException.h"
#include "GlobalConfig.h"
#include "FacesBase.h"
#include "FacesDetect.h"
#include "FacesRecognize.h"

#ifndef DLL_EXPORT
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif

extern "C"
{
	DECLDIR const char* facesToPgm(char *dirId, char *imgName);
	DECLDIR const char* train();
	DECLDIR const char* predict(char *str);
}

#endif
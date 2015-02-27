#ifndef FCGI_UTIL
#define FCGI_UTIL
#include <vector>
#include <string>
#include <jsoncpp/json/json.h>
#include <stdlib.h>
#include "fcgio.h"
#include "fcgi_config.h"  // HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
#include "fcgi_stdio.h"
using namespace std;

//Maximum number of bytes allowed to be read from stdin 
const unsigned long STDIN_MAX = 1000000;

/**
  error code 400 in the HTTP header when an error is encountered
*/
void outputErrorMessage();

/**
  HTTP status 200 when no error is encountered
*/
void outputNormalMessage();

/**
  converts a vector of strings into a list of Json values
*/
void stringToJson(vector<string> &stringVals, Json::Value &jsonVals);

/**
  converts a list of Json values into a vector of strings 
*/
void jsonToString(Json::Value &jsonVals, vector<string> &stringVals);

#endif /* SHARED_FUNCTIONS */

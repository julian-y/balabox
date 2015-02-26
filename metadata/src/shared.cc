/* c++ headers*/
#include <stdlib.h>
#include "fcgio.h"
#include "fcgi_config.h"  // HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
#include "fcgi_stdio.h"
#include <jsoncpp/json/json.h>
#include <vector>
#include <string>
#include "shared.hpp"
using namespace std;

void outputErrorMessage() 
{
     cout << "Status: 400\r\n"
          << "Content-type: text/html\r\n"
          << "\r\n"
          << "<html><p>400 INVALID INPUT</p></html>"
          << "\r\n";
}

void outputNormalMessage()
{
     cout << "Content-type: text/html\r\n"
          <<  "\r\n";
}

void stringToJson(vector<string> &stringVals, Json::Value &jsonVals)
{
    for (int i = 0 ; i < stringVals.size(); i++)
    {
        jsonVals[i] = stringVals[i];
    }
}

void jsonToString(Json::Value &jsonVals, vector<string> &stringVals)
{
   for (int i = 0; i < jsonVals.size(); i++) 
   {
       stringVals.push_back(jsonVals[i].asString());
   }
}


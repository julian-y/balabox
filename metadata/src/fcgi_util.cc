/* c++ headers*/
#include <stdlib.h>
#include "fcgio.h"
#include "fcgi_config.h"  // HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
#include "fcgi_stdio.h"
#include <jsoncpp/json/json.h>
#include <vector>
#include <string>
#include <sstream>
#include "fcgi_util.hpp"
using namespace std;

void outputErrorMessage() 
{
     cout << "Status: 400\r\n"
          << "Content-type: text/html\r\n"
          << "\r\n"
          << "<html><p>400 INVALID INPUT</p></html>"
          << "\r\n";
}


void outputNoEntryMessage()
{
          cout << "Status: 400\r\n"
               << "Content-type: text/html\r\n"
               << "\r\n"
               << "<html><p>400 NO ENTRY FOR USER</p></html>"
               << "\r\n";
}


void outputNormalMessage()
{
     cout << "Content-type: application/json\r\n"
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

string intToStr(int i)
{
   string temp;
   stringstream out;
   out << i;
   return out.str();
}

int getQueryParam(const std::string& query_string, 
         const std::string& param, std::string& value) {

      // Verify that the parameter required is found
      int paramPos = query_string.find(param + "=");

      if (paramPos == string::npos) {
           return 1;
      }   

      int valuePos = query_string.find("=", paramPos) + 1;
      int nextParam = query_string.find("&", valuePos);
                
      //if there is another parameter after the one
      //we're searching for
      if (nextParam == string::npos) {
          value = query_string.substr(valuePos);    
      } else {
          value = query_string.substr(valuePos, nextParam - valuePos);
      }   

      return 0;
}

/**
 * allows the user to associate themselves with a cache 
 * See metadata_api.md for more info
*/

/* c++ headers */
#include <stdlib.h>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
extern char ** environ;
#endif
#include "fcgio.h"
#include "fcgi_config.h"  // HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
#include "fcgi_stdio.h"
#include <jsoncpp/json/json.h>
#include <vector> 
#include <map>
#include <sstream>

/* mysql access helpers*/
#include "mysql_helper.hpp"

/* shared function helpers */
#include "fcgi_util.hpp"
using namespace std;

static long gstdin(FCGX_Request * request, char ** content)
{
    char * clenstr = FCGX_GetParam("CONTENT_LENGTH", request->envp);
    unsigned long clen = STDIN_MAX;

    if (clenstr)
    {
        clen = strtol(clenstr, &clenstr, 10);
        if (*clenstr)
        {
           
            cerr << "can't parse \"CONTENT_LENGTH="
                 << FCGX_GetParam("CONTENT_LENGTH", request->envp)
                 << "\"\n";
            clen = STDIN_MAX;
        }

        // *always* put a cap on the amount of data that will be read
        if (clen > STDIN_MAX) clen = STDIN_MAX;

        *content = new char[clen];

        cin.read(*content, clen);
        clen = cin.gcount();
    }
    else
    {
        // *never* read stdin when CONTENT_LENGTH is missing or unparsable
        *content = 0;
        clen = 0;
    }

    // Chew up any remaining stdin - this shouldn't be necessary
    // but is because mod_fastcgi doesn't handle it correctly.

    // ignore() doesn't set the eof bit in some versions of glibc++
    // so use gcount() instead of eof()...
    do cin.ignore(1024); while (cin.gcount() == 1024);

    return clen;
}

/**
  parses the given query string
  returns 0 upon success and nonzero otherwise
*/
int getParam(string param, map<string, string> &dict)
{
   // Verify if the parameters required are found
   int idPos = param.find("user_id"); 
   int filePos = param.find("cache_ip"); 

   if (idPos == string::npos || filePos == string::npos)
      return 1;
  
   int andPos = param.find("&");
   int equPos1 = param.find("=");
   int equPos2 = param.substr(andPos).find("="); 

   // user_id is the first parameter 
   if (idPos < filePos)
   {
      dict["user_id"] = param.substr(equPos1+1, (andPos-equPos1-1));
      dict["cache_ip"] = param.substr(andPos).substr(equPos2+1);
   }
   else
   {
      dict["cache_ip"] = param.substr(equPos1+1, (andPos-equPos1-1));
      dict["user_id"] = param.substr(andPos).substr(equPos2+1);
   }
   return 0;
}

int main (void)
{
    streambuf * cin_streambuf  = cin.rdbuf();
    streambuf * cout_streambuf = cout.rdbuf();
    streambuf * cerr_streambuf = cerr.rdbuf();

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0)
    {
        // Note that the default bufsize (0) will cause the use of iostream
        // methods that require positioning (such as peek(), seek(),
        // unget() and putback()) to fail (in favour of more efficient IO).
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        #if HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
        cin  = &cin_fcgi_streambuf;
        cout = &cout_fcgi_streambuf;
        cerr = &cerr_fcgi_streambuf;
        #else
        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);
        #endif

        // Although FastCGI supports writing before reading,
        // many http clients (browsers) don't support it (so
        // the connection deadlocks until a timeout expires!).
        char * content = NULL;
        unsigned long clen = gstdin(&request, &content);

        Json::StyledWriter styledWriter;
        Json::Value response; 
        string response_body = content;

        char* query_string = FCGX_GetParam("QUERY_STRING", request.envp);

        // Invalid inputs
        if (query_string == NULL)
        {
              outputErrorMessage();             
              continue;
        }
                    
        string param = query_string;
        map<string, string> paramMap;
        int getParamSuccess = getParam(param, paramMap);    
               
        if (getParamSuccess != 0)
        {
            outputErrorMessage();
            continue;
        }
        
        string user_id = paramMap["user_id"];
        string cache_ip = paramMap["cache_ip"];
        
        // Connect and query the database
        MySQLHelper helper;
        helper.connect();
        int CacheAddSuccess = helper.addCache(user_id, cache_ip);
        if (CacheAddSuccess == 0)
        {             
            outputNormalMessage();            
        }
        else
        {
            outputErrorMessage();
        }
        helper.close();
        
        //Testing
        /*string temp;
        stringstream out; 
        out << cache_ip;
        temp = out.str();
        string output2 = "<p>user_id: " + paramMap["user_id"] + " cache_ip: " + temp + "</p>";
        cout.write(output2.c_str(), output2.length()); 
        */
        if (content) delete []content;

        // If the output streambufs had non-zero bufsizes and
        // were constructed outside of the accept loop (i.e.
        // their destructor won't be called here), they would
        // have to be flushed here.
    }

#if HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
    cin  = cin_streambuf;
    cout = cout_streambuf;
    cerr = cerr_streambuf;
#else
    cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);
#endif

    return 0;
}

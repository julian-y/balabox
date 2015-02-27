/**
 * Sends responses for file_commit requests
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
#include <sstream> 
/* mysql access helpers*/
#include "mysql_helper.hpp"

/* shared files helper*/
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
           
            //cout << "Status: 404\r\n\r\n";
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
        
        // Client doesn't send in any data, ignore this request
        // and wait for another one. 
        if (clen == 0) 
        {
            outputErrorMessage();
            continue;
        }
        else 
        {
            Json::Value root;
            Json::Reader reader;
            Json::Value user_id;
            Json::Value file_name;
            Json::Value jsonHashes;
            Json::Value version; 
            Json::StyledWriter styledWriter;
            Json::Value response; 
            string response_body = content;
           
            // Retrieving Json values 
            bool parsedSuccess = reader.parse(response_body, root, false);
            user_id = root["user_id"];
            jsonHashes = root["block_list"];
            file_name = root["file_name"];
            version = root["version"]; 

            // Invalid inputs
            if (!parsedSuccess || user_id == Json::Value::null || version == Json::Value::null 
                  || jsonHashes == Json::Value::null || file_name == Json::Value::null)
            {
                 outputErrorMessage();             
                 continue;
            }
            
            vector<string> hashes;
            jsonToString(jsonHashes, hashes);

            // Connect and update the database
            MySQLHelper helper;
            if (helper.connect() != 0) 
            {
                outputErrorMessage();
                continue;
            }
        
            int vInt = atoi(version.asString().c_str());
            int updateFileData = helper.updateFileData(user_id.asString(), file_name.asString(), hashes, vInt);
            if (updateFileData == 0) 
            {
                outputNormalMessage();
                response["metadata_updated"] = true;
            }
            // Failure to update the metaserver 
            else 
            {
                outputErrorMessage();
                response["metadata_updated"] = false;
                if (updateFileData == EVERS)  
                    response["message"] = "Old client version.";
                else 
                {
                    string temp; 
                    stringstream out;
                    out << updateFileData;
                    string msg = "Database has encountered problems while updating the files. Please try again later. error code: " + out.str();
                    response["message"] = msg;
                }
            }
            helper.close();
            cout.write(styledWriter.write(response).c_str(), styledWriter.write(response).length());
        }
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

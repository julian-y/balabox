/**
 * Sends responses for file_query requests
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

/* mysql access helpers*/
#include "mysql_helper.hpp"

using namespace std;

// Maximum number of bytes allowed to be read from stdin
static const unsigned long STDIN_MAX = 1000000;

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
    int count = 0;
    long pid = getpid();

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
        
        if (clen == 0) 
        {
            cout << "Status: 404\r\n"
                 <<  "Content-type: text/html\r\n"
                 <<  "\r\n"
                 << "<html><p>404 NOT FOUND</p></html>";
            continue;
        }

        /*cout << "<H4>Standard Input - " << clen;
        if (clen == STDIN_MAX) cout << " (STDIN_MAX)";
        cout << " bytes</H4>\n";
        */

        if (clen) {
            //cout.write(content, clen);
            //cout.write("\n", 1);
           
            Json::Value root;
            Json::Reader reader;
            Json::Value user_id;
            Json::Value file_name;
            Json::Value jsonHashes;
            Json::StyledWriter styledWriter;
            Json::Value response; 
            string response_body = content;

            bool parsedSuccess = reader.parse(response_body, root, false);
            user_id = root["user_id"];
            jsonHashes = root["sha256sum"];
            file_name = root["file_name"];

            string status; 
            // Invalid inputs
            if (!parsedSuccess || user_id == Json::Value::null 
                  || jsonHashes == Json::Value::null || file_name == Json::Value::null)
            {
                 cout << "Status: 404\r\n\r\n";
                 continue;
            }
            
            cout << "Content-type: text/html\r\n"
                 <<  "\r\n"
                 <<  "<TITLE>file_comp</TITLE>\n"
                 <<  "<H1>file_comp</H1>\n"
                 <<  "<H4>Request Number: " << ++count << "</H4>\n";
               
            
            vector<string> hashes;
            for (int i = 0; i < jsonHashes.size(); i++) 
            {
                hashes.push_back(jsonHashes[i].asString());
            }
            
            /*
            //testing
            for (int i = 0; i < hashes.size(); i++) 
            {
                cout << hashes[i] << endl;
            }*/

            // Connect and query the database to see if the hash exists
            vector<string> missingHashes;
            MySQLHelper helper;
            helper.connect();
            int blocksMissing = helper.getMissingBlockHashes(hashes, missingHashes);
            
            // The database contains user hashes
            if (missingHashes.empty()) 
            {
                response["nb"] = false;
            }
            // The database does not contain all user hashes
            // User must first upload to block servers first
            else
            {
                response["nb"] = true;

                // Turning strings into Json values 
                Json::Value hashesNeeded;
                for (int i = 0 ; i < missingHashes.size(); i++)
                {
                    hashesNeeded[i] = missingHashes[i];
                }
                response["hashes"] = hashesNeeded;
            }    

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

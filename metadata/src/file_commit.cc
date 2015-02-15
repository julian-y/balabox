/**
 * Sends responses for file_commit requests
 * See metadata_api.md for more info 
 */
/* basic c++ headers */
#include <iostream>
#include <string>
#include <stdlib>
 
/* fast_cgi */
#include "fcgi_stdio.h"


using namespace std;
using namespace sql;

int main(void)
{
    // FCGI initialization

    while(FCGI_Accept() >= 0) {

    } // FCGI response loop

}
#include "http_helper.h"
#include <string>
#include <stdio.h>
#include <iostream>
#include <errno.h>

using namespace std;
int main () {
	string resp;
	HttpHelper::sendLocalMsg("hello", resp, 8889, true);
	cout << resp << endl;
	cout << errno << endl;
}

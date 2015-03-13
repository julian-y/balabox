/* Make sure you create the directory for the database ahead of time
*/

#include "leveldb_helper.hpp"
#include <iostream>

using namespace std;

const string db_name = "temp";

int main() {
	LevelDBHelper test(db_name);

	// Put
	int status = test.put("bala", "box");
	assert(status == 0);

	// Get
	string value;
	status = test.getD("bala", value);
	assert(status == 0);
	assert(value == "box");
	status = test.get("box", value);
	assert(status == 1);
	assert(value == "box");

	cout << "All tests passed!" << endl;
}
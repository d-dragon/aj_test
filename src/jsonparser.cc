#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>

json_t* testSuitRoot;
json_error_t err;
using namespace std;
#define LOGCXX(msg)  (std::cout<< "DBG: " << __FILE__ << "::" << __LINE__ << " | " << msg << std::endl )
int main(int argc, char* argv[]){

	size_t numTestSuit;
	
	cout << "num args:" << argc << endl; 
	testSuitRoot = json_load_file(argv[1], 0, &err);
	if (testSuitRoot == NULL) {
		LOGCXX("error: " << err.text << "at line" << err.line << endl);
		return 1;
	}

	if (!json_is_array(testSuitRoot)) {
		LOGCXX(err.text << "line" << err.line );
		json_decref(testSuitRoot);
		return 1;
	}
	
	numTestSuit = json_array_size(testSuitRoot);
	json_t *testSuitArray[numTestSuit];
	LOGCXX("number of testsuit: " << numTestSuit);

	for (size_t i = 0 ; i < numTestSuit; i++){
			
		testSuitArray[i] = json_array_get(testSuitRoot, i);
		if(!json_is_object(testSuitArray[i])) {
			LOGCXX("error: test suit invalid");
			json_decref(testSuitRoot);
			return 1;
		}
		json_t *tsNameObj, *tcObj;
		const char *tsName;
		tsNameObj = json_object_get(testSuitArray[i], "testsuit");
		if(!json_is_string(tsNameObj)){
			LOGCXX("format invalid");
			json_decref(testSuitRoot);
			return 1;
		}
		tsName = json_string_value(tsNameObj);
		LOGCXX("Test Suit: " << tsName);

		tcObj = json_object_get(testSuitArray[i], "testcases");
		if(!json_is_array(tcObj)){
			LOGCXX("format invalid");
			json_decref(testSuitRoot);
			return 1;
		}
		size_t numTestCase = 0;
		numTestCase = json_array_size(tcObj);
		LOGCXX("number test case of " << tsName << ":" << numTestCase);

	}
	
	
}


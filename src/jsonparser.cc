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


int TestCaseParser(json_t *tcObj, const char *tcTemplatePath);

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

	//get test suit then store in array test suit

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

		//get test cases then store in array
		json_t *tcArrayObj[numTestCase];
		for (size_t i = 0; i < numTestCase; i++) {
			tcArrayObj[i] = json_array_get(tcObj, i);
			if(!json_is_object(tcArrayObj[i])) {

				LOGCXX("format invalid");
				json_decref(testSuitRoot);
				return 1;
			}

			//test case handler
			TestCaseParser(tcArrayObj[i], argv[2]);
		}
	}
}


int TestCaseParser(json_t *tcObj, const char *tcTemplatePath){

	json_t *tcNameObj;
	json_t *tcInputArg;
	const char *tcName;
	tcNameObj = json_object_get(tcObj, "name");
	if(json_is_string(tcNameObj)){

		tcName = json_string_value(tcNameObj);
		LOGCXX("test case " << ": " << tcName);	
	}else{
		LOGCXX("test case format invalid");
		return -1;
	}

	tcInputArg = json_object_get(tcObj, "input");
	if(!json_is_object(tcInputArg)){
		
		LOGCXX("testcase format invalid");
		return -1;
	}else{
		LOGCXX("number args input: " << json_object_size(tcInputArg));
	}
	
	static json_t *tcTemplateObj = NULL;
	//load json template in to static variable
	if ( tcTemplateObj == NULL){
		tcTemplateObj = json_load_file(tcTemplatePath, 0, &err);
		if (!json_is_object(tcTemplateObj)){

			LOGCXX("error: " << err.text << "line: " << err.line);
			json_decref(tcTemplateObj);
			return -1;
		}
	}

	json_t *tc;
	tc = json_object_get(tcTemplateObj, tcName);
	if (!json_is_object(tc)){

		LOGCXX("Testcase " << tcName << "not found");
		return -1;
	}

	json_t *tiArray; 

	tiArray = json_object_get(tc, "testitems");
	if(!json_is_array(tiArray)){
		LOGCXX("testcase format invalid");
	}
	size_t index;
	json_t *tiObj;
	json_array_foreach(tiArray, index, tiObj) {
		LOGCXX("running test item " << index);
		//pass test item and it args in to test item processor
	}
	return 1;
}

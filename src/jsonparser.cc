#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>

#define LOGCXX(msg)  (std::cout<< "DBG: " << __FILE__ << "::" << __LINE__ << " | " << msg << std::endl )
json_t* testSuitRoot;
json_error_t err;
char *g_tiTemplatePath;
using namespace std;



int TestCaseParser(json_t *tcObj, const char *tcTemplatePath);
int TestItemProcessor(json_t *inputArg, json_t *tiObj);

json_t* getTestItemTemplateObj(const char *tiName, const char *tiTemplatePath);

int main(int argc, char* argv[]){

	size_t numTestSuit;
	g_tiTemplatePath = argv[3];

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
		//pass test item and it args in to test item processor
		TestItemProcessor(tcInputArg, tiObj);		
	}
	return 1;
}


int TestItemProcessor(json_t *inputArg, json_t *inputTIObj){

	const char *tiName;
	json_t *tiNameObj, *tiObj;

	tiNameObj = json_object_get(inputTIObj,"name");
	if(!json_is_string(tiNameObj)){
		
		LOGCXX("test item json format invalid");
		return -1;
	}
	tiName = json_string_value(tiNameObj);
	LOGCXX("running test item "<< tiName);

	tiObj = getTestItemTemplateObj(tiName, g_tiTemplatePath);

	if(tiObj == NULL){
		return -1;
	}
	
	LOGCXX("debug ---------------------------------- ");
	size_t index;
	json_t *value;
	json_t *inputArgEle;
	json_array_foreach(tiObj,  index, value){

		const char *inputArgName;
		inputArgName = json_string_value(json_object_get(value, "arg"));
		LOGCXX("updating argument value of " << inputArgName);
		inputArgEle = json_object_get(inputArg, inputArgName);
		if(json_is_string(inputArgEle)){
			if(json_object_set(value, "value", inputArgEle) != 0){
				LOGCXX("update object fail");
			}
		}
	}


	json_array_foreach(tiObj, index, value){
		json_t *tmpEle;
		tmpEle = json_object_get(value, "value");
		if(json_is_string(tmpEle)){
			LOGCXX("---------------value: " << json_string_value(tmpEle));
		}
	}

}

//int TestItemUpdater(json_t **tiObj, json_t *inputEle);

json_t* getTestItemTemplateObj(const char *tiName,const char *tiTemplatePath){
	

	static json_t *tiRoot = NULL;

	json_t *tiObj;
	if(tiRoot == NULL){

		tiRoot = json_load_file(tiTemplatePath, 0, &err);
		if(!tiRoot){
			LOGCXX("load test item json failed :: err: " << err.text << err.line);
			return NULL;
		}
	}

	tiObj = json_object_get(tiRoot, tiName);
	if(!json_is_array(tiObj)){
		
		LOGCXX("test item json format invalid");
		return NULL;
	}
	return tiObj;
}






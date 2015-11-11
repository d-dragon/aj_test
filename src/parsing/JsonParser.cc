#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>
#include "TestWorker.h"
#include "JsonParser.h"

#define OK 0
#define ERROR -1

using namespace std;

JsonParser::JsonParser(const char *tsPath, const char *tcPath, const char *tiPath){

	testSuitRoot	= NULL;
	tcTemplateRoot	= NULL;
	tiRoot			= NULL;
	worker 			= NULL;

	dfTSPath = tsPath;
	dfTCPath = tcPath;
	dfTIPath = tiPath;
}


JsonParser::~JsonParser(){
	if ( NULL != tiRoot )
		json_decref(tiRoot);
	if ( NULL != tcTemplateRoot )
		json_decref(tcTemplateRoot);
	if ( NULL != testSuitRoot )	
		json_decref(testSuitRoot);
}

int JsonParser::startParser(){

	size_t arrayIndex;
	int status;
	json_t *tsObj;
	

	testSuitRoot = json_load_file(dfTSPath, 0, &err);
	if ((testSuitRoot == NULL) || !(json_is_array(testSuitRoot))){

		cout << err.text << " at line " << err.line << endl;
		return -1;
	}

	worker = new TestWorker("com.verik.bus.VENUS_BOARD");

	worker->exportStuffToFile("<!DOCTYPE html><html><head><style>table,th,td{border:2px solid black;border-collapse:collapse;}th,td{padding: 5px;text-align: left;}</style></head><body>");
	json_array_foreach(testSuitRoot, arrayIndex, tsObj){

		if(!json_is_object(tsObj)){

			cout << "test suit format invalid" << endl;
			json_decref(testSuitRoot);
			return -1;
		}

		const char *serviceId = NULL;
		serviceId = json_string_value(json_object_get(tsObj, "serviceid"));
		//FIXME-need move this block code to suitable position for starting Alljoyn Client
		status = worker->startAlljoynClient(serviceId);
		if(status == ERROR){
			worker->StopAlljoynClient();
			return status;
		}

		status = TestsuitParser(tsObj);
 
		if(status == ERROR){
			cout << "Parsed test suit failed" << endl;
			return status;
		}
		//TODO - Stop Alljoyn Client
		worker->StopAlljoynClient();
		
	}

	worker->exportStuffToFile("</body></html>");

	cout << "JsonParser exit" << endl;
	delete worker;
	return status;
}


int JsonParser::TestsuitParser(json_t *tsObj){

	int status;
	const char *tsName;
	json_t *tcRoot;
	

	tsName = json_string_value(json_object_get(tsObj, "testsuit"));
	cout << "run test suit: " << tsName << endl;

	//TODO - check test suit validation and more action on testsuit name
	
	tcRoot = json_object_get(tsObj, "testcases");
	if(!json_is_array(tcRoot)){
		return ERROR;
	}
	status = TestCaseCollector(tcRoot);
	if(status == ERROR){
		cout << "parse testcase in " << tsName << "failed" << endl;
		return status;
	}
	return status;

}

int JsonParser::TestCaseCollector(json_t *tcRoot){

	int status;
	size_t index;
	json_t *tcObj;
	json_t *tcInputArg;
	const char *tcName = NULL;
	json_array_foreach(tcRoot, index, tcObj){

		if(!json_is_object(tcObj)){
			cout << "json format invalid" << endl;
			return ERROR;
		}
		
		tcName = json_string_value(json_object_get(tcObj, "name"));
		if(tcName == NULL){

			cout << "got test case name failed" << endl;
			return ERROR;
		}

		//TODO - more manipulate testcase name
		
		cout << "parsing test case: " << tcName << endl;
		tcInputArg = json_object_get(tcObj, "input");
		if(!json_is_object(tcInputArg)){
			cout << "cannot got input arg object" << endl;
			return ERROR;
		}
		
		if (tcTemplateRoot == NULL){
			tcTemplateRoot = json_load_file(dfTCPath, 0, &err);
			if(!json_is_object(tcTemplateRoot)){
				cout << "error while load test case template" << err.text << err.line << endl;
				json_decref(tcTemplateRoot);
				return ERROR;
			}
		}
		json_t *tcTemplateObj;
		tcTemplateObj = json_object_get(tcTemplateRoot, tcName);
		if(!json_is_object(tcTemplateObj)){

			cout << "test case template not found" << endl;
			return ERROR;
		}
		
		json_t *tiArray;
		tiArray = json_object_get(tcTemplateObj, "testitems");
		if(!json_is_array(tiArray)){
			cout << "got test item in test case failed" << endl;
			return ERROR;
		}

		json_t *tiObj;
		size_t tiIndex;

		json_array_foreach(tiArray, tiIndex, tiObj){

			status = TestItemProcessor(tcInputArg, tiObj);
			if(status == ERROR){
				cout << "run test item failed" << endl;
			}
			cout << "*************************************************\n" << endl;
		}
	 }
	return status;

}

int JsonParser::TestItemProcessor(json_t *inputArg, json_t *tiObj){

	int status;
	string tiName;
	json_t *tiExObj;

	tiName.assign(json_string_value(json_object_get(tiObj, "name")));

	cout << "proccessing test item: " << tiName << endl;
	//TODO - manipulate test item name
	
	tiExObj = getTestItemTemplateObj(tiName.c_str());
	if(tiExObj == NULL){
		cout << "test item " << tiName << "not found in template" << endl;
		return ERROR;
	}

	size_t index; 
	json_t *tiArgObj;;
	json_t *inputArgEle;
	size_t arraySize = 0;
	string tmpContent;

	arraySize = json_array_size(tiExObj);
	string tiArg[arraySize];
	tmpContent.assign("<table border=\"2\" width=\"640\"><col width=\"100\"><col width=\"140\"><col width=\"400\"><tr><th>Test Item</th><td colspan=\"2\">");
	tmpContent.append(tiName);

	char tmp[256];
	sprintf(tmp, "</td></tr><tr><th rowspan=\"%d\">Input</th></tr>", arraySize+1);
	tmpContent.append(tmp);
	memset(tmp, 0,256);

	json_array_foreach(tiExObj, index, tiArgObj){

		const char *inputArgName;
		inputArgName = json_string_value(json_object_get(tiArgObj, "arg"));
		inputArgEle = json_object_get(inputArg, inputArgName);

		if(json_is_string(inputArgEle)){
			
			status = json_object_set(tiArgObj, "value", inputArgEle);
			if(status != OK){

				cout << "update test item value for "  << inputArgName << "failed" << endl;
			}
		}

		tiArg[index].assign(json_string_value(json_object_get(tiArgObj, "value")));

		sprintf(tmp, "<tr><td>%s</td><td>%s</td></tr>", inputArgName, tiArg[index].c_str());
		tmpContent.append(tmp);
		memset(tmp, 0, 256);
		/*
		tmpContent.append("\n");
		tmpContent.append(inputArgName);
		tmpContent.append(":");
		tmpContent.append(tiArg[index]);
		*/

	}
//	tmpContent.append("\n");
	worker->exportStuffToFile(tmpContent.c_str());
	worker->executeTestItem(tiName, arraySize, tiArg);

}
json_t* JsonParser::getTestItemTemplateObj(const char *tiName){

	json_t *tiObj;

	if(tiRoot == NULL){

		tiRoot = json_load_file(dfTIPath, 0, &err);
		if(!tiRoot){
			cout << "load test item json file failed :: err: " << err.text << err.line << endl;
			 return NULL;
		}
	}

	tiObj = json_object_get(tiRoot, tiName);
	if(!json_is_array(tiObj)){

		cout << "test item json format invalid" << endl;
		return NULL;
	}
	return tiObj;

}



#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "TestWorker.h"
#include "JsonParser.h"
#include "common_def.h"


using namespace std;

JsonParser::JsonParser(const char *tsPath, const char *tcPath, const char *tiPath, const char *configPath){

	testSuitRoot	= NULL;
	tcTemplateRoot	= NULL;
	tiRoot			= NULL;
	worker 			= NULL;

//	dfTSPath = tsPath;
//	dfTCPath = tcPath;
	dfTIPath = tiPath;
//	dfConfigPath = configPath;
}


JsonParser::~JsonParser(){
	if ( NULL != tiRoot )
		json_decref(tiRoot);
	if ( NULL != tcTemplateRoot )
		json_decref(tcTemplateRoot);
	if ( NULL != testSuitRoot )	
		json_decref(testSuitRoot);
}

int JsonParser::GetDevIDInJSMsg(string *input, vector<DeviceInfo> *devList){
	LOGCXX("JsonParser::GetDevIDInJSMsg");

	int index;
	json_error_t jsonErr;
    void *iter;
	struct DeviceInfo devInfo;
    json_t *jsonMsg = NULL, *value = NULL, *object = NULL;
    json_t *jsListDevs = NULL;
    jsonMsg = json_loadb(input->c_str(), input->size(), 0, &jsonErr);
    if ( NULL == jsonMsg )
    {
        LOGCXX("Error while loading file: "<< jsonErr.text << " at line: " << jsonErr.line << std::endl);
        return -1;
    }
    // Get List of Devices in JSON Format
    jsListDevs = json_object_get(jsonMsg, "devicesList");
    if (NULL == jsListDevs) {
        LOGCXX("There's error in input list of devices");
        return -1;
    }
    if (true == json_is_array(jsListDevs)){
		if (0 == json_array_size(jsListDevs))
		{
			LOGCXX("There is no device in the list");
			return -1;
		}
		json_array_foreach(jsListDevs, index, value){
			if (true == json_is_object(value)){
				object					= json_object_get(value, "Owner");
				devInfo.Owner			= json_string_value(object);
                object					= json_object_get(value, "Serial");
                devInfo.Serial			= json_string_value(object);
                object					= json_object_get(value, "FriendlyName");
                devInfo.FriendlyName	= json_string_value(object);
                object					= json_object_get(value, "ID");
                devInfo.ID				= json_string_value(object);
                object					= json_object_get(value, "Capability");
                devInfo.Capability		= json_string_value(object);
                object					= json_object_get(value, "ProfileID");
                devInfo.ProfileID		= json_string_value(object);
                object					= json_object_get(value, "EndPoint_num");
                devInfo.EndPoint_num	= json_string_value(object);
				devList->push_back(devInfo);
			}

		}

    }
    else{
		LOGCXX("Input list devices is invalid");
    }

	if (NULL != jsonMsg){
		json_decref(jsonMsg);
	}
	return 0;
}
void JsonParser::JSONGetObjectValue(string *inputString, string objectName, string *output){

	json_t *rootObj;
	json_t *targetObj;
	const char *retString;
	int strLen;

	rootObj = json_loads(inputString->c_str(), 0, &err);
	if(!json_is_object(rootObj)){
		cout << "json format invalid" << endl;
		return;
	}

	targetObj = json_object_get(rootObj, objectName.c_str());
	if (targetObj == NULL){
		cout << "found no object" << endl;
		return;
	}
		
	retString = json_string_value(targetObj);
	if(retString != NULL){
		output->assign(retString);
	}
	json_decref(rootObj);
	json_decref(targetObj);	
 }

void JsonParser::JSONGetObjectValue(json_t *inputObj, string objectName, string *output){
	
	json_t *targetObj;
	const char *retString;

	targetObj = json_object_get(inputObj, objectName.c_str());
	inputObj = json_incref(inputObj);
	if(targetObj == NULL){
		cout << "found no required object" << endl;
		return;
	}
	retString = json_string_value(targetObj);
	if(retString != NULL){
		cout << objectName.c_str() << " : " << retString << endl;
		output->assign(retString);
	}
	json_decref(inputObj);
}
json_t *JsonParser::JSONGetObjectFromString(string *inputString, string objectName){

	json_t *rootObj;

	rootObj = json_loads(inputString->c_str(), 0, &err);
	if(!json_is_object(rootObj)){
		cout << err.text << "at line " << err.line << endl;
		return NULL;
	}

	return json_object_get(rootObj, objectName.c_str());
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

	status = GetWorkerConfiguration(worker, dfConfigPath);
	if(status == ERROR){
		cout << "read program configuration at " << dfConfigPath << " failed!!" << endl;
		delete worker;
		return status;
	}

	worker->exportStuffToFile("<!DOCTYPE html><html><head><style>table,th,td{border:2px solid black;border-collapse:collapse;}th,td{padding: 5px;text-align: left;}</style></head><body>");
	json_array_foreach(testSuitRoot, arrayIndex, tsObj){

		if(!json_is_object(tsObj)){

			cout << "test suit format invalid" << endl;
			json_decref(testSuitRoot);
			return -1;
		}

		//FIXME-need move this block code to suitable position for starting Alljoyn Client
		status = worker->startAlljoynClient(worker->mConfig.serviceId.c_str());
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
			
			TestItemInfo *ti_info;
			status = TestItemProcessor(tcInputArg, tiObj, &ti_info);
			if(status == ERROR){
				cout << "run test item failed" << endl;
			}
			else{
				cout << "Matched message: " << worker->GetPoolEleValue(ti_info->MatchedLogIndex) << endl;
				// TO DO: a test item
				// 
			}
				// TO DO: a test item
				// 
			}
			//cout << ti_info->Signal << " | " << ti_info->MatchedLogIndex << endl;
			cout << "*************************************************\n" << endl;
		}
		// TO DO: test case
	 }
	return status;

}

int JsonParser::TestItemProcessor(json_t *inputArg, json_t *tiObj, TestItemInfo **ti_info){

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
	worker->exportStuffToFile(tmpContent.c_str());
	tmpContent.clear();
	memset(tmp, 0,256);

	const char *inputArgName[arraySize];
	json_array_foreach(tiExObj, index, tiArgObj){

		inputArgName[index] = json_string_value(json_object_get(tiArgObj, "arg"));
		inputArgEle = json_object_get(inputArg, inputArgName[index]);

		if(json_is_string(inputArgEle)){
			
			tiArg[index].assign(json_string_value(inputArgEle));
		}else{

			tiArg[index].assign(json_string_value(json_object_get(tiArgObj, "value")));
		}


		cout << "index: " << index << " " << inputArgName[index] << ": " << tiArg[index] << endl;
	
	}
	worker->executeTestItem(tiName, arraySize, tiArg, ti_info);

	for(int i = 0; i < arraySize; i++){

		sprintf(tmp, "<tr><td>%s</td><td>%s</td></tr>", inputArgName[i], tiArg[i].c_str());
		tmpContent.append(tmp);
		memset(tmp, 0, 256);
		//	tmpContent.append("\n");
	}

	worker->exportStuffToFile(tmpContent.c_str());
	worker->exportStuffToFile(worker->htmlResultContent.c_str());
	sleep(5);
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


int JsonParser::GetWorkerConfiguration(TestWorker *worker, const char *dfConfigPath){

	json_t *configRootObj;

	configRootObj = json_load_file(dfConfigPath, 0 , &err);
	if(configRootObj == NULL || !json_is_object(configRootObj)){
		cout << "load configuration file failed!!!" << err.text << err.line << endl;
		return ERROR;
	}

	JSONGetObjectValue(configRootObj, "serviceid", &(worker->mConfig.serviceId));
	JSONGetObjectValue(configRootObj, "deviceindex", &(worker->mConfig.deviceIndex));
	JSONGetObjectValue(configRootObj, "altdeviceid", &(worker->mConfig.altDeviceId));
	JSONGetObjectValue(configRootObj, "devicetype", &(worker->mConfig.deviceType));
	JSONGetObjectValue(configRootObj, "associationdevidx", &(worker->mConfig.associationDevIndex));
	JSONGetObjectValue(configRootObj, "altassdevid", &(worker->mConfig.altAssDevId));

	json_decref(configRootObj);

	return OK;
}
void JsonParser::PrintConfigurationInfo(const char *filePath) {

	json_t *configRootObj;

	configRootObj = json_load_file(filePath, 0 , &err);
	if(configRootObj == NULL || !json_is_object(configRootObj)){
		cout << "load configuration file failed!!!" << err.text << err.line << endl;
	}

	const char *key;
	json_t *value;

	cout << "********************Configuration*******************" << endl;
	json_object_foreach(configRootObj, key, value) {

		cout << "*\t" << key << " : " << json_string_value(value) << endl;
	}
	cout << "****************************************************" << endl;
	json_decref(configRootObj);
}

int JsonParser::UpdateConfiguration(const char *filePath) {

	json_t *config_root_obj;

	config_root_obj = json_load_file(filePath, 0 , &err);
	if (NULL == config_root_obj || !json_is_object(config_root_obj)) {
		cout << "load configuration file failed!!!" << err.text << err.line << endl;
		return ERROR;
	}
	const char *key;
	json_t *value;
	string input_value;

	json_object_foreach(config_root_obj, key, value) {
		
		cout << "+++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
		cout << key << " : " << json_string_value(value) << endl;
		cout << "New value (Press 'Enter' to keep current value): ";
		getline(cin, input_value);
		if (!input_value.empty()) {
			json_string_set(value, input_value.c_str());
			cout << key << " : " << json_string_value(value) << endl;
		} else {

			cout << key << " : " << json_string_value(value) << endl;
		}
		cout << endl;
	}
	if (0 == json_dump_file(config_root_obj, filePath, JSON_INDENT(4) | JSON_PRESERVE_ORDER)) {
		cout << "Updated Configuration successfully!" << endl;
	} else {
		cout << "Updated Configuration failed!" << endl;
		return ERROR;
	}
	json_decref(config_root_obj);
	return OK;
}
int JsonParser::GetTestSuiteFileList(const char *dirPath){

	DIR *dir = NULL;
	struct dirent *dir_node = NULL;
	
	dir = opendir(dirPath);
	if (NULL == dir) {
		closedir(dir);
		cout << dirPath << "not found" << endl;
		return ERROR;
	}

	string file_name;
	while (NULL != (dir_node = readdir(dir))) {
	
		/* just only get regular file */
		if(dir_node->d_type == DT_REG) {
			file_name.clear();\
			file_name.assign(dir_node->d_name);
			mFileList.push_back(file_name);
		}

	}
	closedir(dir);
}

void JsonParser::ApplyPaths(const char *tsPath, const char *tcPath, const char *configPath) {

	dfTSPath = tsPath;
	dfTCPath = tcPath;
	dfConfigPath = configPath;
}

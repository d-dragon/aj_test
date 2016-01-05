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
	mRefJsonRoot	= NULL;

//	dfTSPath = tsPath;
//	dfTCPath = tcPath;
	dfTIPath = tiPath;
//	dfConfigPath = configPath;
	mVerdictHelper = new ResultVerdictHelper();
}


JsonParser::~JsonParser(){
	if ( NULL != tiRoot )
		json_decref(tiRoot);
	if ( NULL != tcTemplateRoot )
		json_decref(tcTemplateRoot);
	if ( NULL != testSuitRoot )
		json_decref(testSuitRoot);
	if ( NULL != mVerdictHelper){
		delete mVerdictHelper;
		mVerdictHelper = NULL;
	}
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
//		cout << objectName.c_str() << " : " << retString << endl;
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

int JsonParser::startParser(int reference_flag){

	size_t arrayIndex;
	int status;
	json_t *tsObj;
	json_t *ts_name_obj;
	const char *test_suite_name;

	mReferenceFlag = reference_flag;
	testSuitRoot = json_load_file(dfTSPath, 0, &err);
	if ((testSuitRoot == NULL) || !(json_is_array(testSuitRoot))){

		//LOGCXX(err.text << " at line " << err.line << endl);
		return -1;
	}


	worker = new TestWorker("com.verik.bus.VENUS_BOARD");

	status = GetWorkerConfiguration(worker, dfConfigPath);
	if(status == ERROR){
		cout << "read program configuration at " << dfConfigPath << " failed!!" << endl;
		delete worker;
		return status;
	}
	status = aReporter.InitOutputReportDir(worker->mConfig.deviceName.c_str());
	if (RET_ERR == status) {
		//LOGCXX("Prepare report output dir failed");
		return -1;
	}

	//FIXME-need move this block code to suitable position for starting Alljoyn Client
	status = worker->startAlljoynClient(worker->mConfig.serviceId.c_str());
	if(status == ERROR){
		worker->StopAlljoynClient();
		return status;
	}
	aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<!DOCTYPE html><html><head><style>table,th,td{border:2px solid black;border-collapse:collapse;}th,td{padding: 5px;text-align: left;}</style></head><body>");
	aReporter.WriteContentToReport(REPORT_TYPE_SUMMARY, "Test suite,Test case,Test Item,Result\n");
		
	/* Collect test suite info */
	json_array_foreach(testSuitRoot, arrayIndex, tsObj){

		if(!json_is_object(tsObj)){

			LOGCXX("test suite format invalid" << endl);
			json_decref(testSuitRoot);
			return -1;
		}
		

		const char *ts_name;

		ts_name = json_string_value(json_object_get(tsObj, "testsuite"));
		/* Prepare test suite report output */
		if (NULL == ts_name) {
			cout << "test suite format is invalid (have no test suite name)" << endl;
			return -1;
		}
		mTestSuiteList.push_back(ts_name);
	}

	/* TODO - Add more user choice for running test suite */
	
	/* Parsing test suite */
	aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<h1>%s</h1><hr><br>", worker->mConfig.deviceName.c_str());
	json_array_foreach(testSuitRoot, arrayIndex, tsObj){

		if(!json_is_object(tsObj)){

			cout << "test suite format invalid" << endl;
			json_decref(testSuitRoot);
			return -1;
		}
		/* Export test suite information to report */
		const char *html_content;
	aReporter.WriteContentToReport(REPORT_TYPE_SUMMARY, "Test suite,Test case,Test Item,Result\n");
		aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<h2>Test Suite: %s</h2>", mTestSuiteList[arrayIndex].c_str());
		
		status = TestsuitParser(tsObj);

		aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<hr><br>");

		if(status == ERROR){
			cout << "Parsed test suit failed" << endl;
			return status;
		}

	}

	//TODO - Stop Alljoyn Client
	worker->StopAlljoynClient();
	aReporter.WriteContentToReport(REPORT_TYPE_FULL, "</body></html>");

	cout << "JsonParser exit" << endl;
	delete worker;
	return status;
}


int JsonParser::TestsuitParser(json_t *tsObj){

	int status;
	const char *ts_name;
	json_t *tcRoot;


	ts_name = json_string_value(json_object_get(tsObj, "testsuite"));
	/* Prepare test suite report output */
	if (NULL == ts_name) {
		cout << "test suite format is invalid (have no test suite name)" << endl;
		return -1;
	}
	aReporter.WriteContentToReport(REPORT_TYPE_SUMMARY, "%s,,,\n", ts_name);
	status = aReporter.CreateTestSuiteReport(ts_name);
	if (RET_ERR == status) {
		LOGCXX("Prepare test suite report failed");
		return status;
	}
	aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<!DOCTYPE html><html><head><style>table,th,td{border:2px solid black;border-collapse:collapse;}th,td{padding: 5px;text-align: left;}</style></head><body>");
	aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<h2>Test Suite: %s</h2>", ts_name);

	LOGCXX("******************************************************************************************");
	LOGCXX("****************** Test suite: " << ts_name);
	LOGCXX("******************************************************************************************" << endl);
	//TODO - check test suit validation and more action on testsuit name

	tcRoot = json_object_get(tsObj, "testcases");
	if(!json_is_array(tcRoot)){
		return ERROR;
	}
	status = TestCaseCollector(tcRoot);
	if(status == ERROR){
		cout << "parse testcase in " << ts_name << "failed" << endl;
		return status;
	}
	aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<hr><br>");
	aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "</body></html>");
	aReporter.CloseTestSuiteReport();
	return status;

}

int JsonParser::TestCaseCollector(json_t *tcRoot){

	int status;
	size_t index;
	json_t *tcObj;
	json_t *tcInputArg;
	json_t *tc_expected_output;
	json_t *tc_verdict;
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

		LOGCXX("--------------------------------------Test case: " << tcName << "-------------------------------------" << endl);
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

		/* Use struct TestCase to store all neccessary info for verdicting */
		json_t *tiObj;
		size_t tiIndex;
		size_t num_test_item;

		num_test_item  = json_array_size(tiArray);


		struct TestCase test_case_info;
		test_case_info.name.assign(tcName);
		test_case_info.numOfTestItem = (unsigned short)num_test_item;
		test_case_info.testItemInfo = new TestItem[num_test_item];

		const char *tc_desc = NULL;
		tc_desc = json_string_value(json_object_get(tcObj, "description"));
		if (NULL != tc_desc) {
			test_case_info.testDesc.assign(tc_desc);
			LOGCXX("Description: " << tc_desc);
		}

		LOGCXX("Number test item: " << num_test_item);
#if 0
		tc_expected_output = json_object_get(tcObj, "expectedoutput");
		if(NULL != tc_expected_output) {
			test_case_info.verdictType = VERDICT_EXPECTED;
			/* Getting json expected object then fill in test case expectation info */
			test_case_info.testExpect.numOfObject = json_object_size(tc_expected_output);
			test_case_info.testExpect.expectedObjs = new JsonFormatSimulation[test_case_info.testExpect.numOfObject];
			for (int i = 0; i < test_case_info.testExpect.numOfObject; i++) {

			}
		}

#endif
		/* Collect verdict information */
		tc_verdict = json_object_get(tcObj, "verdict");
		if (NULL != tc_verdict) {

			json_t *verdict_method;

			verdict_method = json_object_get(tc_verdict, "method");
			if (NULL != verdict_method) {
				const char *method = json_string_value(verdict_method);
				if (0 == strcmp("reference", method)) {
					/**
					 * Collect reference value then fill in test case reference info.
					 * If value is "ref", verdict will based on value in references.json,
					 * which was updated by run reference test suite.
					 * Otherwise, verdict will dicectly use value in test suite.
					 */

					json_t *verdict_value;
					test_case_info.verdictType = VERDICT_REFERENCE;
					verdict_value = json_object_get(tc_verdict, "value");
					if (NULL != verdict_value) {
						size_t value_size;
						const char *value_name;
						int count_obj = 0;
						json_t *value_obj;
						value_size = json_object_size(verdict_value);
						test_case_info.testRef.numOfObject = value_size;
						test_case_info.testRef.referenceUnitObjs = new JsonFormatSimulation[value_size];
						json_object_foreach(verdict_value, value_name, value_obj) {
							test_case_info.testRef.referenceUnitObjs[count_obj].key.assign(value_name);
							/* Separately store value as a string or numeric */
							if (json_is_string(value_obj)) {
								test_case_info.testRef.referenceUnitObjs[count_obj].value.push_back(json_string_value(value_obj));
							} else if (json_is_number(value_obj)) {
								 json_unpack(value_obj, "F", &(test_case_info.testRef.referenceUnitObjs[count_obj].numValue));
							}
							count_obj++;
						}

					}
					/* Debugging - Print all reference verdict info */
#if 0
					cout << "Verdict type: " << test_case_info.verdictType << " - have " << test_case_info.testRef.numOfObject << "object" << endl;
					for (int i = 0; i < test_case_info.testRef.numOfObject; i++) {
						if (0 < test_case_info.testRef.referenceUnitObjs[i].value.size()) {

							cout << test_case_info.testRef.referenceUnitObjs[i].value[0] << endl;
						} else {
							cout << test_case_info.testRef.referenceUnitObjs[i].numValue << endl;
						}
					}

#endif
				} else if (0 == strcmp("expectation", method)){
					/* Collect expected value then fill in test case expectation info */

					json_t *verdict_value;
					test_case_info.verdictType = VERDICT_EXPECTED;
					verdict_value = json_object_get(tc_verdict, "value");
					if (NULL != verdict_value) {
						size_t value_size;
						const char *value_name;
						int count_obj = 0;
						json_t *value_obj;
						value_size = json_object_size(verdict_value);
						test_case_info.testExpect.numOfObject = value_size;
						test_case_info.testExpect.expectedObjs = new JsonFormatSimulation[value_size];
						json_object_foreach(verdict_value, value_name, value_obj) {
							test_case_info.testExpect.expectedObjs[count_obj].key.assign(value_name);
							/* Separately store value as a string or numeric */
							if (json_is_string(value_obj)) {
								test_case_info.testExpect.expectedObjs[count_obj].value.push_back(json_string_value(value_obj));
							} else if (json_is_number(value_obj)) {
								 json_unpack(value_obj, "F", &(test_case_info.testExpect.expectedObjs[count_obj].numValue));
							} if (json_is_array(value_obj)) {

								size_t index;
								json_t *obj;

								json_array_foreach(value_obj, index, obj) {
									 if (json_is_string(obj)) {
										 test_case_info.testExpect.expectedObjs[count_obj].value.push_back(json_string_value(obj));
									 }
								}
							}
							count_obj++;
						}

					}
				}

			}
		} else {
			test_case_info.verdictType = VERDICT_UNKNOWN;
		}


		/* Test item info of test case will be filled out while parsing and processing test item */
		json_array_foreach(tiArray, tiIndex, tiObj){

			TestItemInfo *ti_info;
			status = TestItemProcessor(tcInputArg, tiObj, &ti_info, &(test_case_info.testItemInfo[tiIndex]));
			if(status == ERROR){
				cout << "run test item failed" << endl;
			}
			else{
				/* TODO - update reference value into reference.json */
				if (1 == mReferenceFlag) {
					//update reference value
					LOGCXX("-----------update reference value------------");
					string response_msg = worker->GetPoolEleValue(ti_info->MatchedLogIndex);
					UpdateReferenceValue(ti_info, response_msg);
				} else {

					//mVerdictHelper->SaveInfoOfTestItem(tcInputArg, ti_info, worker->GetPoolEleValue(ti_info->MatchedLogIndex));
				}
			}
            LOGCXX("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl);
			sleep(5);
		}
		if (0 == mReferenceFlag) {

			//mVerdictHelper->DBGPrint();
			/* Implement temparary verdict reference result */
			if (VERDICT_REFERENCE == test_case_info.verdictType) {
                LOGCXX("Verdict type: REFERENCE");
				test_case_info.verdictResult = mVerdictHelper->VerdictResult(&test_case_info, "src/references/references.json");
			} else if (VERDICT_EXPECTED == test_case_info.verdictType){
                LOGCXX("Verdict type: EXPECTED");
				test_case_info.verdictResult = mVerdictHelper->VerdictResult(&test_case_info,"");
			}
			/* TODO - Export test case info to report */
			ReportTestCaseInfo(test_case_info);
		}
		LOGCXX("-------------------------------------------------------------------------------------------------------" << endl << endl);
		/* Debugging - Print all test case info */
#if 0
		cout << test_case_info.name << ": " << test_case_info.testDesc << endl << " have " << test_case_info.numOfTestItem << " items" << endl;
		for (int i = 0; i < test_case_info.numOfTestItem; i++) {
			cout << test_case_info.testItemInfo[i].name << ": " << test_case_info.testItemInfo[i].numOfArg << " argument" << endl;
			for (int j = 0; j < test_case_info.testItemInfo[i].numOfArg; j++) {
				cout << test_case_info.testItemInfo[i].testItemArg[j].key << " : " << test_case_info.testItemInfo[i].testItemArg[j].value[0] << endl;
			}
			cout << "response: " << test_case_info.testItemInfo[i].testItemLogPool[test_case_info.testItemInfo[i].matchedRespMsgIndex] << endl;
		}
#endif
		/* Manually deallocate memory of Test case member */
		DeallocateTestCaseInfo(test_case_info);
	}
	return status;

}

int JsonParser::TestItemProcessor(json_t *inputArg, json_t *tiObj, TestItemInfo **ti_info, TestItem *t_test_item){

	int status;
	string tiName;
	json_t *tiExObj;

	tiName.assign(json_string_value(json_object_get(tiObj, "name")));

	LOGCXX("+++++++++++++++++++Test item: " << tiName << "++++++++++++++++++++++");
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
	tmpContent.clear();
	memset(tmp, 0,256);

	/* Collect test item information */
	t_test_item->name.assign(tiName);
	t_test_item->numOfArg = (unsigned short)arraySize;
	t_test_item->testItemArg = new JsonFormatSimulation[t_test_item->numOfArg];

	const char *inputArgName[arraySize];
	json_array_foreach(tiExObj, index, tiArgObj){

		inputArgName[index] = json_string_value(json_object_get(tiArgObj, "arg"));
		inputArgEle = json_object_get(inputArg, inputArgName[index]);

		if(json_is_string(inputArgEle)){

			tiArg[index].assign(json_string_value(inputArgEle));
		}else{

			tiArg[index].assign(json_string_value(json_object_get(tiArgObj, "value")));
		}

		/* Save Test Item arguments as a json simulation */
		t_test_item->testItemArg[index].key.assign(inputArgName[index]);
		t_test_item->testItemArg[index].value.push_back(tiArg[index]);
		t_test_item->testItemLogPool.reserve(10);

		LOGCXX("\t" << inputArgName[index] << ": " << tiArg[index]);

	}
	worker->executeTestItem(tiName, arraySize, tiArg, ti_info, t_test_item);

	for(int i = 0; i < arraySize; i++){

		sprintf(tmp, "<tr><td>%s</td><td>%s</td></tr>", inputArgName[i], tiArg[i].c_str());
		tmpContent.append(tmp);
		memset(tmp, 0, 256);
		//	tmpContent.append("\n");
	}

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

	JSONGetObjectValue(configRootObj, "devicename", &(worker->mConfig.deviceName));
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

void JsonParser::ApplyPaths(const char *tsPath, const char *tcPath, const char *configPath, const char *ref_path) {

	dfTSPath = tsPath;
	dfTCPath = tcPath;
	dfConfigPath = configPath;
	mReferencePath = ref_path;
}

int JsonParser::UpdateReferenceValue(TestItemInfo *ti_info, string response_msg) {

	int status = 0;
	json_t *command_info_obj, *response_obj;

	/**
	 * Determine command class and type of reference value to get suitable field of value
	 * Because getting value just done by read_spec or read_s_spec. Therefore, we just
	 * consider these commands.
	 */
	if (0 == ti_info->Signal.compare("read_spec") || 0 == ti_info->Signal.compare("read_s_spec")) {

		/**
		 * Load reference.json once time at executing the first matched test item.
		 */
		mRefJsonRoot = json_load_file(mReferencePath, 0, &err);
		if (NULL == mRefJsonRoot) {
			LOGCXX("json load reference file failed");
			return ERROR;
		}

		const char *command_class_str, *type_str, *resp_status;

		/* Get response message json object */
		LOGCXX("type: " << ti_info->Type << " signal: " << ti_info->Signal);
		response_obj = json_loadb(response_msg.c_str(), response_msg.length(), 0, &err);
		if (NULL == response_obj) {
			LOGCXX("load json response message failed");
			json_decref(mRefJsonRoot);
			return  ERROR;
		}
		resp_status = json_string_value(json_object_get(response_obj, "status"));
		if (0 == strcmp(resp_status, "successful")) {

			command_info_obj = json_object_get(response_obj, "commandinfo");
			if (NULL == command_info_obj) {
				LOGCXX("load commandinfo obj failed");
				status = ERROR;
			} else {
				command_class_str = json_string_value(json_object_get(command_info_obj, "class"));
				LOGCXX("class: " << command_class_str);

				/* Update new value into references object based on command class */
				if (0 == strcmp(command_class_str, "BATTERY")) {
					status = ReplaceValueSensorMultilevel(response_obj, mRefJsonRoot, command_class_str, "", "batterylevel");
				} else if (0 == strcmp(command_class_str, "SENSOR_MULTILEVEL")) {
					string sensor_type_str;
					JSONGetObjectValue(command_info_obj, "data0", &sensor_type_str);
					cout << "sensor_type_str = " << sensor_type_str << endl;

					if (0 == sensor_type_str.compare("TEMP")) {
						status = ReplaceValueSensorMultilevel(response_obj, mRefJsonRoot,command_class_str, sensor_type_str, "fahrenheit");
						if (ERROR != status) {
							status = ReplaceValueSensorMultilevel(response_obj, mRefJsonRoot, command_class_str, sensor_type_str, "celsius");
						}
					} else if (0 == sensor_type_str.compare("HUMI")) {
						status = ReplaceValueSensorMultilevel(response_obj, mRefJsonRoot, command_class_str, sensor_type_str, "percentage");
						if (ERROR != status) {
							status = ReplaceValueSensorMultilevel(response_obj, mRefJsonRoot, command_class_str, sensor_type_str, "absolute_humidity");
						}
					} else if (0 == sensor_type_str.compare("LUMI")) {

						status = ReplaceValueSensorMultilevel(response_obj, mRefJsonRoot, command_class_str, sensor_type_str, "luminance");
					} else if (0 == sensor_type_str.compare("UV")) {
						status = ReplaceValueSensorMultilevel(response_obj, mRefJsonRoot, command_class_str, sensor_type_str, "ultraviolet");
					}

				} else if (0 == strcmp(command_class_str, "METER")) {

					status = ReplaceValueSensorMultilevel(response_obj, mRefJsonRoot, command_class_str, "", "unit");
					if (ERROR != status) {
						status = ReplaceValueSensorMultilevel(response_obj, mRefJsonRoot, command_class_str, "", "time");
						if (ERROR != status) {
							status = ReplaceValueSensorMultilevel(response_obj, mRefJsonRoot, command_class_str, "", "electricmeter");
						}
					}
				} else {
					LOGCXX("command class not matched");
					return ERROR;
				}
			}
		}
		json_decref(response_obj);

		if (ERROR != status) {

			status = json_dump_file(mRefJsonRoot, mReferencePath, JSON_INDENT(4) | JSON_PRESERVE_ORDER);
			if (0 != status) {
				LOGCXX("dump json object to file failed");
			} else {

				LOGCXX("Update sensor reference value success");
			}
		}
		json_decref(mRefJsonRoot);
	}

	return status;
}

int JsonParser::ReplaceValueSensorMultilevel(json_t *resp_root, json_t *ref_root, const char *cmd_class, string sensor_type, const char *target_name) {

	int status = OK;
	json_t *target_ref_obj, *source_resp_obj;

	target_ref_obj = json_object_get(ref_root, cmd_class);
	if (NULL == target_ref_obj) {
		return ERROR;
	}
	if (0 == strcmp(cmd_class, "SENSOR_MULTILEVEL")) {

		target_ref_obj = json_object_get(target_ref_obj, sensor_type.c_str());
	}
	source_resp_obj = json_object_get(resp_root, target_name);

	if ((NULL == target_ref_obj) || (NULL == source_resp_obj)) {
		return ERROR;
	} else {
		status = json_object_set(target_ref_obj, target_name, source_resp_obj);
	}
	return status;

}

void JsonParser::ReportTestCaseInfo(TestCase testCaseInfo) {

	/* Test case name and description */
	aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<p>Test case: %s</p>", testCaseInfo.name.c_str());
	aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<p>Test case: %s</p>", testCaseInfo.name.c_str());

	aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<p>Description: %s</p>", testCaseInfo.testDesc.c_str());
	aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<p>Description: %s</p>", testCaseInfo.testDesc.c_str());

	aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<p>Number test item of test case: %d</p>", testCaseInfo.numOfTestItem);
	aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<p>Number test item of test case: %d</p>", testCaseInfo.numOfTestItem);
	
	if (VERDICT_UNKNOWN != testCaseInfo.verdictType) {

		aReporter.WriteContentToReport(REPORT_TYPE_SUMMARY, ",%s,,%s\n", testCaseInfo.name.c_str(),mVerdictHelper->GetVerdictStringByCode(testCaseInfo.verdictResult));
	} else {

		aReporter.WriteContentToReport(REPORT_TYPE_SUMMARY, ",%s,,\n", testCaseInfo.name.c_str());
	}

	for (int i = 0; i < testCaseInfo.numOfTestItem; i++) {
		/* Print test item info */

		aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<table border=\"2\" width=\"640\"><col width=\"100\"><col width=\"140\"><col width=\"400\"><tr><th>Test Item</th><td colspan=\"2\">%s</td></tr><tr><th rowspan=\"%d\">Input</th></tr>",testCaseInfo.testItemInfo[i].name.c_str(), testCaseInfo.testItemInfo[i].numOfArg+1);
		aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<table border=\"2\" width=\"640\"><col width=\"100\"><col width=\"140\"><col width=\"400\"><tr><th>Test Item</th><td colspan=\"2\">%s</td></tr><tr><th rowspan=\"%d\">Input</th></tr>",testCaseInfo.testItemInfo[i].name.c_str(), testCaseInfo.testItemInfo[i].numOfArg+1);


		for(int j = 0; j < testCaseInfo.testItemInfo->numOfArg; j++){

			aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<tr><td>%s</td><td>%s</td></tr>",testCaseInfo.testItemInfo[i].testItemArg[j].key.c_str(), testCaseInfo.testItemInfo[i].testItemArg[j].value.at(0).c_str());
			aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<tr><td>%s</td><td>%s</td></tr>",testCaseInfo.testItemInfo[i].testItemArg[j].key.c_str(), testCaseInfo.testItemInfo[i].testItemArg[j].value.at(0).c_str());
		}

		aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<tr><th>Result</th><td colspan=\"2\">%d</td></tr>", mVerdictHelper->GetVerdictStringByCode(testCaseInfo.testItemInfo[i].verdictResult));
		aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<tr><th>Result</th><td colspan=\"2\">%d</td></tr>", mVerdictHelper->GetVerdictStringByCode(testCaseInfo.testItemInfo[i].verdictResult));
		aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<tr><th>Response Message</th><td colspan=\"2\">%s</td></tr>", testCaseInfo.testItemInfo[i].testItemLogPool[testCaseInfo.testItemInfo[i].matchedRespMsgIndex].c_str());
		aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<tr><th>Response Message</th><td colspan=\"2\">%s</td></tr>", testCaseInfo.testItemInfo[i].testItemLogPool[testCaseInfo.testItemInfo[i].matchedRespMsgIndex].c_str());
		aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<tr><th>Log Pool</th><td colspan=\"2\">");
		aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<tr><th>Log Pool</th><td colspan=\"2\">");
		for (int k = 0; k < testCaseInfo.testItemInfo[i].testItemLogPool.size(); k++) {

			aReporter.WriteContentToReport(REPORT_TYPE_FULL, "%s<br>", testCaseInfo.testItemInfo[i].testItemLogPool[k].c_str());
			aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "%s<br>", testCaseInfo.testItemInfo[i].testItemLogPool[k].c_str());

		}
		aReporter.WriteContentToReport(REPORT_TYPE_FULL, "</td></tr></table><br>");
		aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "</td></tr></table><br>");

		aReporter.WriteContentToReport(REPORT_TYPE_SUMMARY, ",,%s,%s\n", testCaseInfo.testItemInfo[i].name.c_str(), mVerdictHelper->GetVerdictStringByCode(testCaseInfo.testItemInfo[i].verdictResult));

	}
	/* Print test case result verdict */

	if (VERDICT_UNKNOWN != testCaseInfo.verdictType) {

		aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<p>Test case result verdict: %s</p>", mVerdictHelper->GetVerdictStringByCode(testCaseInfo.verdictResult));
		aReporter.WriteContentToReport(REPORT_TYPE_TEST_SUITE, "<p>Test case result verdict: %s</p>", mVerdictHelper->GetVerdictStringByCode(testCaseInfo.verdictResult));
	} else {

	}
	aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<p>*********************************************************</p>");
	//aReporter.WriteContentToReport(REPORT_TYPE_FULL, "<p>Test case log poll: ", testCaseInfo.verdictResult);
}
void JsonParser::DeallocateTestCaseInfo(TestCase test_case_info) {

	for (int i = 0; i < test_case_info.numOfTestItem; i++) {
		delete[] test_case_info.testItemInfo[i].testItemArg;
	}
	if (VERDICT_REFERENCE == test_case_info.verdictType) {

			delete[] test_case_info.testRef.referenceUnitObjs;
	} else if (VERDICT_EXPECTED == test_case_info.verdictType) {

			delete[] test_case_info.testExpect.expectedObjs;
	}
	delete[] test_case_info.testItemInfo;

}

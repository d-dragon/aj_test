#include "common_def.h"
#include "ResultVerdictHelper.h"
#include "JsonParser.h"


ResultVerdictHelper::ResultVerdictHelper(){
}

ResultVerdictHelper::~ResultVerdictHelper(){

}

int ResultVerdictHelper::VerdictResult(TestCase *test_case_t, const char *reference_path) {
	int ret = VERDICT_RET_SUCCESS;
    SignalTypeEnum sigtype;
	if (VERDICT_REFERENCE == test_case_t->verdictType) {
		int ref_verdict;
		json_error_t err;
		json_t *ref_root;

		ref_root = json_load_file(reference_path, 0, &err);
		if (NULL == ref_root) {
			cout << "load references.json failed: " << err.source << " >> " << err.text << endl;
			return VERDICT_RET_UNKNOWN;
		}

		/**
		 * Verdict each test item in test case.
		 * If there is any failed test item this mean tese case was failed too.
		 */
		for (int i = 0; i < test_case_t->numOfTestItem; i++) {
			ref_verdict = ValidateTestItemResult(test_case_t->testRef, test_case_t->testItemInfo[i], ref_root);
			test_case_t->testItemInfo[i].verdictResult = ref_verdict;
			if (VERDICT_RET_SUCCESS != ref_verdict) {
				ret = ref_verdict;
			}
			LOGCXX("Test item " << test_case_t->testItemInfo[i].name << ": " << GetVerdictStringByCode(ret) << endl);

		}
		json_decref(ref_root);
	}
    else
    if (VERDICT_EXPECTED == test_case_t->verdictType){
        sigtype = GetSignalType(*test_case_t,0);
        LOGCXX("Signal type: "<< sigtype );
        switch (sigtype) {
            case ZWAVE:
                LOGCXX("EvaluationTestCase");
                test_case_t->verdictResult = EvaluationTestCase(*test_case_t);
                ret                        = test_case_t->verdictResult;
                switch (test_case_t->verdictResult) {
                    case 	VERDICT_RET_INPUT_INVALID: /*!< Test item input invalid */
                        std::cout<<"=======>>>>>>>>>>>>>>>>Return value VERDICT_RET_INPUT_INVALID"<<std::endl;
                    break;
                    case	VERDICT_RET_RESP_INVALID:			/*!< Response message info invalid */
                        std::cout<<"=======>>>>>>>>>>>>>>>>Return value VERDICT_RET_RESP_INVALID"<<std::endl;
                    break;
                    case	VERDICT_RET_UNKNOWN:				/*!< Unknown error while verdict */
                        std::cout<<"=======>>>>>>>>>>>>>>>>Return value VERDICT_RET_UNKNOWN"<<std::endl;
                    break;
                    case	VERDICT_RET_FAILED:					/*!< Verdict result is failed */
                        std::cout<<"=======>>>>>>>>>>>>>>>>Return value VERDICT_RET_FAILED"<<std::endl;
                    break;
                    case	VERDICT_RET_SUCCESS:					/*!< Verdict result is success */
                        std::cout<<"=======>>>>>>>>>>>>>>>>Return value VERDICT_RET_SUCCESS"<<std::endl;
                    break;
                }
            break;
            case ZIGBEE:
            case UPNP:
            default:
                LOGCXX("This type of command is not support now");
            break;
        }
    }
	LOGCXX("Test case " << test_case_t->name << ": " << GetVerdictStringByCode(ret));
	return ret;
}

int ResultVerdictHelper::ValidateTestItemResult(TestCaseReferenceUnit test_ref, TestItem test_item_t, json_t *ref_root) {
	int ret = VERDICT_RET_SUCCESS;

	json_t *resp_root;
	json_error_t err;
	JsonParser parser(NULL, NULL, NULL, NULL);


	/* Load response message as a json object */
	string resp_msg = test_item_t.testItemLogPool[test_item_t.matchedRespMsgIndex];

	resp_root = json_loads(resp_msg.c_str(), 0, &err);
	if (NULL == resp_root) {

		LOGCXX("load json response message failed: " << err.source << " >> " << err.text);
		return VERDICT_RET_UNKNOWN;
	}

	/* Validate response message  then verdict the test item result */
	if ((0 == test_item_t.name.compare("read_spec")) || (0 == test_item_t.name.compare("read_s_spec")))	{
		string *device_type, *id, *cmd_class, *command, *type;
		device_type = GetTIArgumentValueByKey(test_item_t, "devicetype");

		if (NULL == device_type) {
			ret = VERDICT_RET_UNKNOWN;
		} else if (0 == device_type->compare("zwave")) {
			/* Specific verdict result for zwave device */
			string resp_device_type, resp_method, resp_id, resp_status;

			parser.JSONGetObjectValue(resp_root, "type", &resp_device_type);
			parser.JSONGetObjectValue(resp_root, "deviceid", &resp_id);
			parser.JSONGetObjectValue(resp_root, "method", &resp_method);

			id = GetTIArgumentValueByKey(test_item_t, "id");

			/* Verify device type, id and method */
			if ((0 != device_type->compare(resp_device_type)) ||
				/*(0 != id->compare(resp_id)) ||*/
				((0 != resp_method.compare("read_specR")) && (0 != resp_method.compare("read_s_specR")))) {
				ret = VERDICT_RET_RESP_INVALID;
			} else {

				json_t *resp_cmd_info;

				parser.JSONGetObjectValue(resp_root, "status", &resp_status);
				if (0 != resp_status.compare("successful")) {
					ret = VERDICT_RET_FAILED;
				} else {

					resp_cmd_info = json_object_get(resp_root, "commandinfo");
					if (NULL == resp_cmd_info) {
						ret = VERDICT_RET_RESP_INVALID;
					} else {

						/* Verify command class and sensing type */
						cmd_class = GetTIArgumentValueByKey(test_item_t, "class");
						command = GetTIArgumentValueByKey(test_item_t, "readcommand");

						if (NULL == cmd_class ||
								NULL == command) {
							ret = VERDICT_RET_UNKNOWN;
						} else {
							/* Verdict the test item response result */
							string resp_class, resp_type;
							if (0 == cmd_class->compare(SENSOR_MULTILEVEL_CLASS)) {

								type = GetTIArgumentValueByKey(test_item_t, "type");

								parser.JSONGetObjectValue(resp_cmd_info, "class", &resp_class);
								parser.JSONGetObjectValue(resp_cmd_info, "data0", &resp_type);

								if ((0 != cmd_class->compare(resp_class)) || (0 != type->compare(resp_type))) {
									ret = VERDICT_RET_RESP_INVALID;

								} else {

									for (int i = 0; i < test_ref.numOfObject; i++) {
										if (0 < test_ref.referenceUnitObjs[i].value.size())	{
											/* verdict sensing result by reference in reference.json */
											json_t *resp_sensing_value;
											resp_sensing_value = json_object_get(resp_root, test_ref.referenceUnitObjs[i].key.c_str());
											if (NULL == resp_sensing_value) {
												/* TODO - Take suitable action in this case */
												continue;
											} else {
												/* Get corresponding reference value to verdict */
												json_t *json_ref_value_obj;
												json_ref_value_obj = json_object_get(ref_root, SENSOR_MULTILEVEL_CLASS);
												json_ref_value_obj = json_object_get(json_ref_value_obj, type->c_str());
												json_ref_value_obj = json_object_get(json_ref_value_obj, test_ref.referenceUnitObjs[i].key.c_str());

												if (NULL != json_ref_value_obj) {
													if (json_is_real(resp_sensing_value) && json_is_real(json_ref_value_obj)) {
														double sensing_ref, sensing_resp, differential;

														json_unpack(json_ref_value_obj, "F", &sensing_ref);
														json_unpack(resp_sensing_value, "F", &sensing_resp);

														differential = sensing_ref * 0.1;
														LOGCXX("reference = " << sensing_ref << " | response = " << sensing_resp << " | differential = " << differential );

														if (((sensing_ref - differential) < sensing_resp) &&
															((sensing_ref + differential) > sensing_resp)) {
															ret = VERDICT_RET_SUCCESS;
														} else {
															ret = VERDICT_RET_FAILED;
														}
													}
												}
											}
										} else {
											/* Use defined reference value in test suite for verdict */
											json_t *json_ref_value_obj;
											json_ref_value_obj = json_object_get(ref_root, SENSOR_MULTILEVEL_CLASS);
											json_ref_value_obj = json_object_get(json_ref_value_obj, type->c_str());
											json_ref_value_obj = json_object_get(json_ref_value_obj, test_ref.referenceUnitObjs[i].key.c_str());

											if (NULL != json_ref_value_obj) {
												if (json_is_real(json_ref_value_obj)) {
													double sensing_ref, sensing_resp, differential;

													sensing_resp = test_ref.referenceUnitObjs[i].numValue;
													json_unpack(json_ref_value_obj, "F", &sensing_ref);
													differential = sensing_ref * 0.1;
													LOGCXX("reference = " << sensing_ref << " | response = " << sensing_resp << " | differential = " << differential );

													if (((sensing_ref - differential) < sensing_resp) &&
															((sensing_ref + differential) > sensing_resp)) {
														ret = VERDICT_RET_SUCCESS;
													} else {
														ret = VERDICT_RET_FAILED;
													}
												}
											}

										}
									}
								}


							} else if (0 == cmd_class->compare(BATTERY_CLASS)) {

							} else if (0 == cmd_class->compare(METER_CLASS)) {

							}
						}
					}
				}
			}
		}
	}

	json_decref(resp_root);

	return ret;
}
string* ResultVerdictHelper::GetTIArgumentValueByKey(TestItem test_item_t, string key) {

	for (int i = 0; i < test_item_t.numOfArg; i++) {
		if (0 == test_item_t.testItemArg[i].key.compare(key)) {
			return &(test_item_t.testItemArg[i].value[0]);
		}
	}
	return NULL;
}

/*
    Return type of signal, index use for test item.
    Return following value:
        ZWAVE = 0,
        ZIGBEE,
        UPNP,
        UNKNOWN
*/

SignalTypeEnum ResultVerdictHelper::GetSignalType(TestCase input, int index){
    SignalTypeEnum ret = UNKNOWN;
    JsonFormatSimulation *tempTIarg;
    unsigned short numArgofTI, i, j;

    // Check input value
    if (input.numOfTestItem < index)
    {
        return UNKNOWN;
    }

    numArgofTI = input.testItemInfo[index].numOfArg;
    tempTIarg = input.testItemInfo[index].testItemArg;

    for (i = 0; i < numArgofTI; i++){
        if ( 0 == tempTIarg[i].key.compare("devicetype")){
            for (j = 0; j < sizeof(signalTypeStr)/sizeof(SignalTypeStructure); j++)
            {
                if (0 == tempTIarg[i].value.front().compare(signalTypeStr[i].name)){
                    // Found a match
                    return signalTypeStr[i].val;
                }
            }
        }
    }
    return ret;
}
/*
    Return signal name, index use for test item.
    Return following value:
        data of SignalTypeEnum enumeration
        ADD_DEV = 0,
        LIST_DEV,
        GET_BIN,
        SET_BIN,
        ...
*/
SignalNameEnum ResultVerdictHelper::GetSignalName(TestCase input, int index){
    SignalNameEnum ret = UNSUPPORTED;
    unsigned short i;
    // Check input value
    if (input.numOfTestItem < index)
    {
        return UNSUPPORTED;
    }
    for (i = 0; i < sizeof(signalNameStr)/sizeof(SignalNameStructure); i++){
        if (0 == signalNameStr[i].name.compare(input.testItemInfo[index].name)){
            // Found a match
            return signalNameStr[i].val;
        }
    }
    return ret;
}

/*
    Return command class name index use for test item.
    Return following value:
    CONFIGURATION = 0,
    ASSOCIATION,
    SENSORMULTILEVEL,
    BATTERY
*/
RWSpecsCmdClassEnum ResultVerdictHelper::GetRWSpecsClass(TestCase input, int index){
    RWSpecsCmdClassEnum ret = RESERVED;
    JsonFormatSimulation *tempTIarg;
    unsigned short numArgofTI, i, j;

    // Check input value
    if (input.numOfTestItem < index)
    {
        return RESERVED;
    }
    numArgofTI = input.testItemInfo[index].numOfArg;
    tempTIarg = input.testItemInfo[index].testItemArg;
    for (j = 0; j < numArgofTI; j++){
        if (0 != tempTIarg[j].key.compare("class")){
            continue;
        }
        for (i = 0; i < sizeof(rwSpecsCommandClassStr)/sizeof(RWSpecsCommandClassStructure); i++){
            if (0 == rwSpecsCommandClassStr[i].name.compare(tempTIarg[j].value.front())){
                // Found a match
                return rwSpecsCommandClassStr[i].val;
            }
        }
    }
    return ret;
}
/*
    Parse data of test case to fill data of message
*/
void ResultVerdictHelper::GetMsgRespRWSpec(PrivateData *outData, TestCase tc, int index){
    RWSpecsCmdClassEnum rwspecCmdClass;
    unsigned short i;

    string matchedLogData = tc.testItemInfo[index].testItemLogPool.at(tc.testItemInfo[index].matchedRespMsgIndex);

    rwspecCmdClass = GetRWSpecsClass(tc);
    switch (rwspecCmdClass) {
        case CONFIGURATION:
            outData->cmdClass               = CONFIGURATION;
            outData->msgConf.type           = GetValueFromJSON(matchedLogData,"type");
            outData->msgConf.method         = GetValueFromJSON(matchedLogData,"method");
            outData->msgConf.devID          = GetValueFromJSON(matchedLogData,"deviceid");
        //    outData->cmdInfo        = GetValueFromJSON(matchedLogData,"commandinfo");
            outData->msgConf.status         = GetValueFromJSON(matchedLogData,"status");
            if ((0 == outData->msgConf.method.compare("read_specR")) || (0 == outData->msgConf.method.compare("read_s_specR")) ){
                if (0 == outData->msgConf.status.compare("successful")){ // OK case
                    outData->msgConf.parameter  = GetValueFromJSON(matchedLogData,"parameter");
                    outData->msgConf.value = GetValueFromJSONInteger(matchedLogData,"value");
                }else   // Failed case
                {
                    outData->msgConf.reason     = GetValueFromJSON(matchedLogData,"reason");
                }
            }

        break;
        case ASSOCIATION:
            outData->cmdClass                    = ASSOCIATION;
            outData->msgAssociate.type           = GetValueFromJSON(matchedLogData,"type");
            outData->msgAssociate.method         = GetValueFromJSON(matchedLogData,"method");
            outData->msgAssociate.devID          = GetValueFromJSON(matchedLogData,"deviceid");
        //    outData->cmdInfo        = GetValueFromJSON(matchedLogData,"commandinfo");
            outData->msgAssociate.status         = GetValueFromJSON(matchedLogData,"status");
            if (0 == outData->msgAssociate.method.compare("read_specR")){
                if (0 == outData->msgAssociate.status.compare("successful")){ // OK case
                    outData->msgAssociate.groupid           = GetValueFromJSONInteger(matchedLogData,"groupid");
                    outData->msgAssociate.maxnode           = GetValueFromJSONInteger(matchedLogData,"maxnode");
                    outData->msgAssociate.nodefollow        = GetArrayValueFromJSON(matchedLogData,"nodefollow");
                }else   // Failed case
                {
                    outData->msgAssociate.reason        = GetValueFromJSON(matchedLogData,"reason");
                }
            }
        break;
        case SENSORMULTILEVEL:
            outData->cmdClass               = SENSORMULTILEVEL;
            LOGCXX("Command class SENSOR MULTILEVEL");
        break;
        case BATTERY:
            outData->cmdClass               = BATTERY;
            LOGCXX("Command class BATTERY");

        break;
        default:
            LOGCXX("Command Class is not support now: " << rwspecCmdClass);
            return;
        break;
    }
}
/*
    Compare configuration and association only
    Stage A: expectation
*/
int ResultVerdictHelper::EvaluationTestCase(TestCase &tc){
    int ret, i, j, ret1, ret2;
    bool isTrue = true;
    vector<string> commandVal;
    string matchedLogData, respStatus;
    PrivateData respData;

    ret = VERDICT_RET_UNKNOWN;
    // Stage A: Evaluate expected and result of the test case
    // 1. Get responded message of last command in test case
    if ( tc.numOfTestItem < 1 ){
        // Test case is invalid
        return VERDICT_RET_UNKNOWN;
    }
    for (i = 0; i < tc.numOfTestItem; i++){
         matchedLogData = tc.testItemInfo[i].testItemLogPool.at(tc.testItemInfo[i].matchedRespMsgIndex);
         respStatus     = GetValueFromJSON(matchedLogData,"status");
         LOGCXX("Responde status of test case ;"<< respStatus);
         if (0 != respStatus.compare("successful"))
         {
            tc.testItemInfo[i].verdictResult = VERDICT_RET_FAILED;
            isTrue = false;
         }else
         {
            tc.testItemInfo[i].verdictResult = VERDICT_RET_SUCCESS;
         }
    }
    if (!isTrue){
        return VERDICT_RET_FAILED;
    }
    GetMsgRespRWSpec(&respData, tc, (tc.numOfTestItem - 1));
    // 2. Get Expected result and compare
    switch (respData.cmdClass) {
        case CONFIGURATION:
            ret1 = ExpectationComparison(tc.testExpect, respData, "");
            ret2 = InOutTestCaseComparison(tc, respData, "");
        break;
        case ASSOCIATION:
            commandVal = GetValueOfTestItem(tc.testItemInfo[tc.numOfTestItem - 2], "writecommand");
            ret1 = ExpectationComparison(tc.testExpect, respData ,commandVal.front());
            ret2 = InOutTestCaseComparison(tc, respData, commandVal.front());
        break;
        case BATTERY:
        break;
        case SENSORMULTILEVEL:
        break;
        default:
        break;
    }
    if ((ret1 == VERDICT_RET_FAILED) || (ret2 == VERDICT_RET_FAILED)){
        ret = VERDICT_RET_FAILED;
    }else if ((ret1 == VERDICT_RET_SUCCESS) && (ret2 == VERDICT_RET_SUCCESS)){
        ret = VERDICT_RET_SUCCESS;
    }
    return ret;
}

int ResultVerdictHelper::ExpectationComparison(TestCaseExpectation tcExpect, PrivateData respData, string command ){
    int ret, i;
    JsonFormatSimulation *tempData;
    vector<string> expectedVal;
    int expectedNumVal = -1;
    bool isIncluded = false;
    ret = VERDICT_RET_UNKNOWN;
    if ( 0 == command.compare("") ){ // CONFIGURATION
        // Get Expected value from test case;
        tempData    = NULL;
        for ( i = 0; i< tcExpect.numOfObject; i++){
            if (0 == tcExpect.expectedObjs[i].key.compare("value")){
                if (tcExpect.expectedObjs[i].value.size() != 0 ){
                    tempData = &tcExpect.expectedObjs[i];
                }else
                {
                    expectedNumVal = (int)tcExpect.expectedObjs[i].numValue;
                }
                break;
            }
        }
        if (expectedNumVal != respData.msgConf.value){
            return VERDICT_RET_FAILED;
        }
        else{
            return VERDICT_RET_SUCCESS;
        }
    }else // ASSOCIATION
    {
        // Get data from Set command input
        if (( NULL == tcExpect.expectedObjs)){
            return VERDICT_RET_INPUT_INVALID;
        }

        // Get data from expected info of test case
        tempData = NULL;
        if (NULL == tcExpect.expectedObjs){
            LOGCXX("Invalid input ResultVerdictHelper::ExpectationComparison");
            return VERDICT_RET_UNKNOWN;
        }
        for ( i = 0; i< tcExpect.numOfObject; i++){
            if (0 == tcExpect.expectedObjs[i].key.compare("nodefollow")){
                tempData = &tcExpect.expectedObjs[i];
                break;
            }
        }
        if (NULL != tempData){
            expectedVal = tempData->value;
        }
        if (expectedVal.size() == 0){
            return VERDICT_RET_INPUT_INVALID;
        }
        //Compare expected data and respond
        for (i = 0; i < respData.msgAssociate.nodefollow.size(); i++ ){
            if (0 == expectedVal.front().compare(respData.msgAssociate.nodefollow.at(i))){
                isIncluded = true;
            }
        }
        // make a verdict
        if (0 == command.compare("REMOVE")){
            if (isIncluded){
                return VERDICT_RET_FAILED;
            }
            return VERDICT_RET_SUCCESS;
        }else
        if (0 == command.compare("SET")){
            if (!isIncluded){
                return VERDICT_RET_FAILED;
            }
            return VERDICT_RET_SUCCESS;
        }
    }
    return ret;
}

int ResultVerdictHelper::InOutTestCaseComparison(TestCase tc, PrivateData respData, string addOrRemove){
    int index_of_write, index_of_read, i, inputVal;
    int ret = VERDICT_RET_UNKNOWN;
    bool isIncluded = false, isSecureRWSpec = false;
    std::vector<string> valInput;
    string sInput;

    index_of_write = -1;
    if (tc.numOfTestItem < 1){
        return VERDICT_RET_UNKNOWN;
    }

    // Found write_spec testItem
    for (i = 0 ; i < tc.numOfTestItem ; i++){
        if (0 == tc.testItemInfo[i].name.compare("write_spec")){
            index_of_write = i;
            break;
        }
        if (0 == tc.testItemInfo[i].name.compare("write_s_spec")){
            isSecureRWSpec = true;
            index_of_write = i;
            break;
        }
    }
    //read_spec must be next of write_spec
    index_of_read = index_of_write + 1;
    if ( index_of_read >= tc.numOfTestItem )
    {
        return VERDICT_RET_UNKNOWN;
    }
    if (isSecureRWSpec){
        if (0 != tc.testItemInfo[index_of_read].name.compare("read_s_spec")){
            return VERDICT_RET_UNKNOWN;
        }
    }else if (0 != tc.testItemInfo[index_of_read].name.compare("read_spec")){
        return VERDICT_RET_UNKNOWN;
    }
    if (index_of_write == -1){ // not found
        return VERDICT_RET_UNKNOWN;
    }else{ // found
        for (i = 0; i < tc.testItemInfo[index_of_write].numOfArg; i++)
        {
            if (0 == tc.testItemInfo[index_of_write].testItemArg[i].key.compare("data1")){
                // Find out the key of value
                valInput = tc.testItemInfo[index_of_write].testItemArg[i].value;
                break;
            }
        }
        if (0 == valInput.size()){ // Not found data1
            return VERDICT_RET_UNKNOWN;
        }
    }
    if ( 0 == addOrRemove.compare("") ){ // CONFIGURATION
        // compare input and output
        sInput = "";
        for (i = 0; i < valInput.size(); i++)
        {
            sInput += valInput.at(i);
        }
        if (string::npos != sInput.find("0x")){
            inputVal = stoi(sInput,NULL,16);
        }else{
            inputVal = stoi(sInput,NULL,10);
        }
        if (inputVal == respData.msgConf.value){
            return VERDICT_RET_SUCCESS;
        }else{
            return VERDICT_RET_FAILED;
        }
    }else
    {
        for (i = 0; i < respData.msgAssociate.nodefollow.size(); i++ ){
            if ( 0 ==  valInput.front().compare(respData.msgAssociate.nodefollow.at(i))){
                isIncluded = true;
                break;
            }
        }
        if (0 == addOrRemove.compare("REMOVE")){
            if (isIncluded){
                return VERDICT_RET_FAILED;
            }else{
                return VERDICT_RET_SUCCESS;
            }
        }
        else
        if (0 == addOrRemove.compare("SET")){
            if (!isIncluded){
                return VERDICT_RET_FAILED;
            }else{
                return VERDICT_RET_SUCCESS;
            }
        }
        else
        return VERDICT_RET_UNKNOWN;
    }

    return ret;
}

vector<string> ResultVerdictHelper::GetValueOfTestItem(TestItem ti, string key){
    std::vector<string> ret;
    int i, size;
    LOGCXX("Num of Args of Test item: "<< ti.numOfArg);
    size = ti.numOfArg;
    for (i = 0; i < size; i++){
        if( 0 == ti.testItemArg[i].key.compare(key))
        {
            ret = ti.testItemArg[i].value;
        }
    }
    return ret;
}

string ResultVerdictHelper::GetValueFromJSON(string input, string object){
    string retString = "";
    json_t *jsonrootB = NULL, *jsVal;
    json_error_t jsonErr;
    /*
        Processing
     */
    jsonrootB = json_loadb(input.c_str(), input.size(), 0, &jsonErr);
    if ( NULL == jsonrootB )
    {
        LOGCXX("Error while loading string "<< jsonErr.text << " at line: " << jsonErr.line << "ResultVerdictHelper::GetValueFromJSON");
        return retString;
    }

    jsVal = json_object_get(jsonrootB, object.c_str());
    if ( NULL == jsVal){
        json_decref(jsonrootB);
        return retString;
    }

    if (false == json_is_string(jsVal)){
        LOGCXX("This data is not a string");
        json_decref(jsonrootB);
        return retString;
    }

    retString = json_string_value(jsVal);

    /*
        Releasing
     */
    if (NULL != jsonrootB){
        json_decref(jsonrootB);
    }
    return retString;
}

vector<string> ResultVerdictHelper::GetArrayValueFromJSON(string input, string object){
    std::vector<string> retString;
    json_t *jsonrootB = NULL, *jsVal, *value;
    json_error_t jsonErr;
    size_t arraysz, index;
    /*
        Processing
     */
    jsonrootB = json_loadb(input.c_str(), input.size(), 0, &jsonErr);
    if ( NULL == jsonrootB )
    {
        LOGCXX("Error while loading string "<< jsonErr.text << " at line: " << jsonErr.line << "ResultVerdictHelper::GetArrayValueFromJSON");
        return retString;
    }

    jsVal = json_object_get(jsonrootB, object.c_str());
    if ( NULL == jsVal){
        json_decref(jsonrootB);
        return retString;
    }

    if (false == json_is_array(jsVal)){
        LOGCXX("This data is not a array");
        json_decref(jsonrootB);
        return retString;
    }
    arraysz =   json_array_size(jsVal);
    // Traversal all data in data of responded msg (which is an array)
    json_array_foreach(jsVal, index, value) {
        retString.push_back(json_string_value(value));
    }
    /*
        Releasing
     */
    if (NULL != jsonrootB){
        json_decref(jsonrootB);
    }
    return retString;
}

int ResultVerdictHelper::GetValueFromJSONInteger(string input, string key){
    int retInt = -1;
    json_t *jsonrootB = NULL, *jsVal;
    json_error_t jsonErr;
    /*
        Processing
     */
    jsonrootB = json_loadb(input.c_str(), input.size(), 0, &jsonErr);
    if ( NULL == jsonrootB )
    {
        LOGCXX("Error while loading string "<< jsonErr.text << " at line: " << jsonErr.line << "ResultVerdictHelper::GetValueFromJSONInteger");
        return retInt;
    }

    jsVal = json_object_get(jsonrootB, key.c_str());
    if ( NULL == jsVal){
        json_decref(jsonrootB);
        return retInt;
    }

    if (false == json_is_integer(jsVal)){
        LOGCXX("This data is not a number");
        json_decref(jsonrootB);
        return retInt;
    }

    retInt = json_integer_value(jsVal);

    /*
        Releasing
     */
    if (NULL != jsonrootB){
        json_decref(jsonrootB);
    }
    return retInt;
}
const char* ResultVerdictHelper::GetVerdictStringByCode(int verdictCode) {

	unsigned int size = sizeof(gverdictResult)/sizeof(CodeString);
	for (int i = 0; i < size; i++) {
		if (verdictCode == gverdictResult[i].Code) {
			return gverdictResult[i].String;
		}
	}
	return NULL;
}

#include "common_def.h"
#include "ResultVerdictHelper.h"
#include "JsonParser.h"


ResultVerdictHelper::ResultVerdictHelper(){
   SetInvalidAllData();
   mInfoLastPosTracker = THIRD_GET + 1;
}

ResultVerdictHelper::~ResultVerdictHelper(){

}

int ResultVerdictHelper::VerdictResult(json_t* expectedData, json_t* refValue){
    int ret = ERR_INVALID;
    ret = EvaluateOnSavedData();

    switch (ret) {
        case ERR_INVALID:
            LOGCXX("Invalid or undetermined error");
        break;
        case NOT_EQUAL:
            LOGCXX("They are diffrent");
        break;
        case IS_EQUAL:
            LOGCXX("They are the same");
        break;
        case ASSOCIATE_OK:
            LOGCXX("Add associated device is completed");
        break;
        case REMOVE_OK:
            LOGCXX("Remove associated dev is completed");
        break;
        default:
            LOGCXX("undetermined......");
        break;
    }
    LOGCXX(">>>>>>>>>>>>>>>-----------------------");
    LOGCXX("Compare to expected output: "<<EvaluateExpectationVSSavedData(expectedData));
    LOGCXX("----------------------->>>>>>>>>>>>>>>");
    // Clear data when verdict completely;
    SetInvalidAllData();
    return ret;
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
			cout << "verdict result " << ret << endl;

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
                switch (EvaluationTestCase(*test_case_t)) {
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

		cout << "load json response message failed: " << err.source << " >> " << err.text << endl;
		return VERDICT_RET_UNKNOWN;
	}

	/* Validate response message  then verdict the test item result */
	if ((0 == test_item_t.name.compare("read_spec")) || (0 == test_item_t.name.compare("read_s_spec")))	{
		string *device_type, *id, *cmd_class, *command, *type;
		device_type = GetTIArgumentValueByKey(test_item_t, "devicetype");

		if (NULL == device_type) {
			cout << "devicetype is invalid" << endl;
			ret = VERDICT_RET_INPUT_INVALID;
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
							ret = VERDICT_RET_INPUT_INVALID;
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
														cout << "reference = " << sensing_ref << "response = " << sensing_resp << "differential = " << differential << endl;

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
											cout << "debug" << endl;
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
													cout << "reference = " << sensing_ref << "response = " << sensing_resp << "differential = " << differential << endl;

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
    Function: Save test case info, this function support only read_spec and write_spec
              First parameter: save infor of test item
              Second Paramter: save responded msg of that case
    Return: none
*/
void ResultVerdictHelper::SaveInfoOfTestItem(const json_t *testInput, struct TestItemInfo *info, string matchedResponseMsg){
    enum EnumSettingPara i = MAXBUFF;
    json_t *localValue;
    string readcmd, writecmd, specClass;
    string data0, data1;
    data0 = data1 = "";
    int cnt;
    // Verify input parameters
    if (NULL == info )
        return;
    if (0 != info->Type.compare("zwave")){ // It does not support other signal now.
        return;
    }

    localValue = json_object_get(testInput, KEY_CLASS);
    if (NULL == localValue){
        return;
    }
    else{
        specClass = json_string_value(localValue);
    }

    if ((0 == specClass.compare("ASSOCIATION")) || (0 == specClass.compare("CONFIGURATION"))){ // f or CONFIGURATION
        if (true == mLocalTestItemInfo[THIRD_GET].isValid) // reset value which are saved
        {
            SetInvalidAllData();
        }

        // Signal comparation

        if (false == mLocalTestItemInfo[SECOND_SET].isValid){ // All signal info are empty
            if (0 == info->Signal.compare(SIG_W_SPEC)) {    // Handle write_spec
                i = SECOND_SET;
            }
            else if(0 == info->Signal.compare(SIG_R_SPEC)){ // Handle read_spec,
                SetInvalidAllData();
                i = FIRST_GET;
            }
            else{ //Not handle other signal
                SetInvalidAllData();
                return;
            }
        }
        else {
            if(0 == info->Signal.compare(SIG_R_SPEC)){// write_specs is existed, save read_specs to compare
                i = THIRD_GET;
            } else
            if(0 == info->Signal.compare(SIG_W_SPEC)){// write_specs is existed, save read_specs to compare
                SetInvalidAllData();
                i = SECOND_SET;
            }
            else{
                SetInvalidAllData();
                return;
            }
        }

        //Command class comparation
        if (true == mLocalTestItemInfo[FIRST_GET].isValid)
        {
            if (0 != mLocalTestItemInfo[FIRST_GET].funcClass.compare(specClass)) //Clear all buffer if it does not match to previous class
            {
                SetInvalidAllData();
            //    return;
            }
        }
        if (true == mLocalTestItemInfo[SECOND_SET].isValid)
        {
            if (0 != mLocalTestItemInfo[SECOND_SET].funcClass.compare(specClass)) //Clear all buffer if it does not match to previous class
            {
                SetInvalidAllData();
            //    return;
            }
        }
        // Data of class comand (group, configuration type) comparation


        // Parse data from input of test cases
        switch (i){
            case FIRST_GET:
            case THIRD_GET:
            {

                break;
            }
            case SECOND_SET:
            {
                localValue = json_object_get(testInput, "data0");
                if (NULL == localValue){
                    return;
                }
                else{
                    data0 = json_string_value(localValue);
                }

                localValue = json_object_get(testInput, "data1");
                if (NULL == localValue){
                    return;
                }
                else{
                    data1 = json_string_value(localValue);
                }
                localValue = json_object_get(testInput, "writecommand");
                if (NULL == localValue){
                    return;
                }
                else{
                    writecmd = json_string_value(localValue);
                }
                mLocalTestItemInfo[i].cmd              = writecmd;
                break;
            }
            default:
                break;
        }


        mLocalTestItemInfo[i].Signal                    = info->Signal;
        mLocalTestItemInfo[i].Type                      = info->Type;
        mLocalTestItemInfo[i].ID                        = info->ID;
        mLocalTestItemInfo[i].attributeID               = data0;
        mLocalTestItemInfo[i].value                     = data1;
        mLocalTestItemInfo[i].funcClass                 = specClass;
        mLocalTestItemInfo[i].responseMsg               = matchedResponseMsg;
        mLocalTestItemInfo[i].isValid                   = true;
    }
    else if ((0 == specClass.compare(METER_CLASS)) ||
			 (0 == specClass.compare(BATTERY_CLASS)) ||
			 (0 == specClass.compare(SENSOR_MULTILEVEL_CLASS))){
		/** Store test item information for specific zwave command class (METER, BATTERY, SENSORMULTILEVEL).
		 *	These type of infos initialize after  #THIRT_GET in @mLocalTestItemInfo array.
		 *	These infos will be verdicted based on references.
		 **/

		json_t *json_response_root;

		if (true == mLocalTestItemInfo[mInfoLastPosTracker].isValid) {


		}



    }

}
/*
    Function: saved data in mLocalTestItemInfo
                Support: configuration and association only
    Type of compare: exactly
    Return:
        3: removed successful[Association]
        2: included successful [Association]
        1: they are equal
        0: they are diffrent (failed in setting up)
        -1: undetermined/invalid/error
 */
int ResultVerdictHelper::EvaluateOnSavedData(){
    int ret = -1, arraysz, i, pos, posOfNotZero;
    size_t index;
    json_t *respRoot, *localValue, *localNodefollow, *value;
    json_error_t jserr;
    string respondedValue = "", temp;
    bool isMatched = false;

    i = 0;
    // Load responded msg
    respRoot = json_loads(mLocalTestItemInfo[THIRD_GET].responseMsg.c_str(), 0, &jserr);
    if ( NULL == respRoot ){
        return ERR_INVALID;
    }
    // Get data from responded Msg
    localValue = json_object_get(respRoot, "value");
    localNodefollow = json_object_get(respRoot, "nodefollow");
    if ( !localValue && !localNodefollow){
        if ( NULL != respRoot) json_decref(respRoot);
        return ERR_INVALID;
    }

    if (0 == mLocalTestItemInfo[THIRD_GET].funcClass.compare("CONFIGURATION")){ //CONFIGURATION command class
        arraysz =   json_array_size(localValue);
        // Traversal all data in data of responded msg (which is an array)
        json_array_foreach(localValue, index, value) {
            if (i++ < arraysz){
                // TO DO compare array between input and value get from responde message.
                LOGCXX("value:["<< i<<"] = "<<json_string_value(value));
                respondedValue += json_string_value(value);
            }
        }
        LOGCXX("value: "<< respondedValue);
        pos = respondedValue.length() - mLocalTestItemInfo[SECOND_SET].value.length();
        if (pos < 0){ // Invalid size of value input
            LOGCXX("Input value is is not correct, value input is: "<< mLocalTestItemInfo[SECOND_SET].value);
            return ERR_INVALID;
        }
        LOGCXX("Position found: "<< pos<< ":"<<respondedValue.substr(pos, string::npos));
        if ( 0 != respondedValue.compare(pos, std::string::npos, mLocalTestItemInfo[SECOND_SET].value.c_str()) ){
            return 0;
        }
        if ( 0 == pos){ // 2 string are the same size, so they are equal.
            return 1;
        }

        temp.assign(respondedValue.substr(0, pos));
        LOGCXX("Remained string: "<< temp <<" length: " << temp.length());
        posOfNotZero = temp.find_first_not_of('0',0);
        if ( std::string::npos == posOfNotZero ){
            ret = 1;
        }
        else
            ret = 0;

    }else if (0 == mLocalTestItemInfo[THIRD_GET].funcClass.compare("ASSOCIATION")){
        arraysz =   json_array_size(localNodefollow);
        // Traversal all data in data of responded msg (which is an array)
        json_array_foreach(localNodefollow, index, value) {
            if (i++ < arraysz){
                if (0 == mLocalTestItemInfo[SECOND_SET].value.compare(json_string_value(value))){
                    isMatched = true;
                }
            }
        }
        if (0 == mLocalTestItemInfo[SECOND_SET].cmd.compare("SET")){
            if (true == isMatched)
                ret = 2; // Associated sucess
            else
                ret = 0;
        } else
        if (0 == mLocalTestItemInfo[SECOND_SET].cmd.compare("REMOVE")){
            if (false == isMatched)
                ret = 3; // Remove success
            else
                ret = 0;
        } else
            return ERR_INVALID;
    }else{ // Not support
        LOGCXX("This kind of message is not supported");
        ret = ERR_INVALID;
    }

    if ( NULL != respRoot) json_decref(respRoot);
    return ret;
}
/*
    Function: compare 2 json string input
                target is compare respond message from CERES and expected result is listed in the test case.
    Type of compare: exactly
    Return:
        1: they are equal
        0: they are diffrent
        -1: undetermined/invalid/error
 */
int ResultVerdictHelper::Compare2JSONData(const string a, const string b){
    /*
        Local paramter
     */
    int ret = ERR_INVALID;
    json_error_t jsonErr;
    json_t *jsonrootA = NULL, *jsonrootB = NULL;
    /*
        Processing
     */
    jsonrootA = json_loadb(a.c_str(), a.size(), 0, &jsonErr);
    if ( NULL == jsonrootA )
    {
        LOGCXX("Error while loading string A: "<< jsonErr.text << " at line: " << jsonErr.line << "ResultVerdictHelper::Compare2JSONData");
        return ERR_INVALID;
    }
    jsonrootB = json_loadb(b.c_str(), b.size(), 0, &jsonErr);
    {
        LOGCXX("Error while loading string B: "<< jsonErr.text << " at line: " << jsonErr.line << "ResultVerdictHelper::Compare2JSONData");
        return ERR_INVALID;
    }

    Compare2JSONData(jsonrootA, jsonrootB);
    /*
        Releasing
     */
    if (NULL != jsonrootA){
        json_decref(jsonrootA);
    }
    if (NULL != jsonrootB){
        json_decref(jsonrootB);
    }
    return ret;
}
int ResultVerdictHelper::Compare2JSONData(json_t* a, const string b){
    int ret = ERR_INVALID;
    json_t *jsonrootB = NULL;
    json_error_t jsonErr;
    /*
        Processing
     */
    jsonrootB = json_loadb(b.c_str(), b.size(), 0, &jsonErr);
    if ( NULL == jsonrootB )
    {
        LOGCXX("Error while loading string A: "<< jsonErr.text << " at line: " << jsonErr.line << "ResultVerdictHelper::Compare2JSONData");
        return ERR_INVALID;
    }

    Compare2JSONData(a, jsonrootB);
    /*
        Releasing
     */
    if (NULL != jsonrootB){
        json_decref(jsonrootB);
    }
    return ret;
}

int ResultVerdictHelper::Compare2JSONData(json_t* a, json_t* b){
    int ret = ERR_INVALID;
    const char *key;
    json_t *valueA, *valueB;
    int total_obj_cnt;

    total_obj_cnt = json_object_size(a);
    if ( 0 == total_obj_cnt )
    {
        LOGCXX("There is no JSON object inside string A");
        return ERR_INVALID;
    }
    /*
        Scan all key of A and compare value of each key in A then compare to corresponding value of the key in B
     */
    json_object_foreach(a, key, valueA){
        valueB = json_object_get(b, key);
        ret = json_equal(valueA, valueB);
        if ( 1 != ret ){
            return 0; // There is an element is not equal.
        }
    };
    return ret;
}

/*
    Function: Compare expectedData vs Saved Data,
                Support: configuration and association only
    Type of compare: exactly
    Return:
        1: they are equal
        0: they are diffrent (failed in setting up)
        -1: undetermined/invalid/error
 */
int ResultVerdictHelper::EvaluateExpectationVSSavedData(json_t *expectedData){
    int ret = ERR_INVALID;
    json_error_t jsonErr;
    json_t *jsonRoot = NULL;
    json_t *jsObject;
    string nodefollow;

    if ( NULL == expectedData){
        return ERR_INVALID;
    }
    /*
        Processing
     */
    jsonRoot = json_loadb(mLocalTestItemInfo[THIRD_GET].responseMsg.c_str(), mLocalTestItemInfo[THIRD_GET].responseMsg.size(), 0, &jsonErr);
    if ( NULL == jsonRoot )
    {
        LOGCXX("Error while loading string A: "<< jsonErr.text << " at line: " << jsonErr.line << "ResultVerdictHelper::Compare2JSONData");
        return ERR_INVALID;
    }
    if( 0 == mLocalTestItemInfo[THIRD_GET].funcClass.compare("ASSOCIATION")){
        jsObject = json_object_get(expectedData,"nodefollow");
        if (jsObject == NULL){
            return ERR_INVALID;
        }
        nodefollow = json_string_value(jsObject);
        ret = EvaluateExpectationofAssociation(nodefollow, jsonRoot);
    }
    else if (0 == mLocalTestItemInfo[THIRD_GET].funcClass.compare("CONFIGURATION")){
        ret = Compare2JSONData(expectedData, jsonRoot);
    } else{
        // TODO other class (SENSOR_MULTILEVEL, BATTERY ...)
    }
    if ( NULL != jsonRoot){
        json_decref(jsonRoot);
        jsonRoot = NULL;
    }
    return ret;
}
/*
    Function: Compare nodeExpected ID and respondMsg,
                Support:  association only
    Type of compare: exactly
    Return:
        1: they are the same expected (Node is not existed if REMOVE, Node is included if SET)
        0: they are diffrent from expectation
        -1: undetermined/invalid/error
 */
int ResultVerdictHelper::EvaluateExpectationofAssociation(string nodeExpected, json_t* respondMsg){
    int ret = ERR_INVALID;
    bool isMatched = false;
    json_t*value, *nodefollowResp;
    size_t index;

    nodefollowResp = json_object_get(respondMsg, "nodefollow");
    if (NULL == nodefollowResp)
    {
        return ERR_INVALID;
    }
    json_array_foreach(nodefollowResp, index, value) {
        if (0 == nodeExpected.compare(json_string_value(value))){
            isMatched = true;
        }
    }
    if (0 == mLocalTestItemInfo[SECOND_SET].cmd.compare("SET")){
        if (true == isMatched)
            ret = IS_EQUAL;
        else
            ret = NOT_EQUAL;
    } else
    if (0 == mLocalTestItemInfo[SECOND_SET].cmd.compare("REMOVE")){
        if (false == isMatched)
            ret = IS_EQUAL;
        else
            ret = NOT_EQUAL;
    } else
        return ERR_INVALID;
    return ret;
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

void ResultVerdictHelper::SetInvalidAllData(){
    for ( int cnt = 0; cnt < MAXBUFF; cnt++)
    {
        mLocalTestItemInfo[cnt].Signal.clear();
        mLocalTestItemInfo[cnt].Type.clear();
        mLocalTestItemInfo[cnt].ID.clear();
        mLocalTestItemInfo[cnt].funcClass.clear();
        mLocalTestItemInfo[cnt].responseMsg.clear();
        mLocalTestItemInfo[cnt].isValid = false;
    }
}

void ResultVerdictHelper::DBGPrint(){
    for ( int i = 0; i < MAXBUFF; i++)
    {
        if (!mLocalTestItemInfo[i].isValid){
            continue;
        }
        LOGCXX("ResultVerdictHelper::DBGPrint mLocalTestItemInfo["<<i<<"]" );
        LOGCXX(mLocalTestItemInfo[i].Signal);
        LOGCXX(mLocalTestItemInfo[i].Type);
        LOGCXX(mLocalTestItemInfo[i].ID);
        LOGCXX(mLocalTestItemInfo[i].funcClass);
        LOGCXX(mLocalTestItemInfo[i].cmd);
        LOGCXX(mLocalTestItemInfo[i].value);
        LOGCXX(mLocalTestItemInfo[i].responseMsg);
        LOGCXX(mLocalTestItemInfo[i].isValid);
    }
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
    string matchedLogData;
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
         if (0 != GetValueFromJSON(matchedLogData,"status").compare("successful"))
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
	LOGCXX("size of gverdictResult: "<< size);

	for (int i = 0; i < size; i++) {
		if (verdictCode == gverdictResult[i].Code) {
			return gverdictResult[i].String;
		}
	}
	return NULL;
}

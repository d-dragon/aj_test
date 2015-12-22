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

int ResultVerdictHelper::VerdictResult(TestCase test_case_t) {

	int ret = ERR_INVALID;

	return ret;
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

    if ((0 == specClass.compare(ASSOCIATION)) || (0 == specClass.compare(CONFIGURATION))){ // ASSOCIATION or CONFIGURATION
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

    if (0 == mLocalTestItemInfo[THIRD_GET].funcClass.compare(CONFIGURATION)){ //CONFIGURATION command class
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

    }else if (0 == mLocalTestItemInfo[THIRD_GET].funcClass.compare(ASSOCIATION)){
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
    if( 0 == mLocalTestItemInfo[THIRD_GET].funcClass.compare(ASSOCIATION)){
        jsObject = json_object_get(expectedData,"nodefollow");
        if (jsObject == NULL){
            return ERR_INVALID;
        }
        nodefollow = json_string_value(jsObject);
        ret = EvaluateExpectationofAssociation(nodefollow, jsonRoot);
    }
    else if (0 == mLocalTestItemInfo[THIRD_GET].funcClass.compare(CONFIGURATION)){
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

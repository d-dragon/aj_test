#include "ResultVerdictHelper.h"

ResultVerdictHelper::ResultVerdictHelper(){
   SetInvalidAllData();
}

ResultVerdictHelper::~ResultVerdictHelper(){

}

int ResultVerdictHelper::VerdictResult(){
    int ret = 0;


    ret = ComapreSavedData();
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
int ResultVerdictHelper::ComapreSavedData(){
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
        return -1;
    }
    // Get data from responded Msg
    localValue = json_object_get(respRoot, "value");
    localNodefollow = json_object_get(respRoot, "nodefollow");
    if ( !localValue && !localNodefollow){
        if ( NULL != respRoot) json_decref(respRoot);
        return -1;
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
            return -1;
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
            return -1;
    }else{ // Not support
        LOGCXX("This kind of message is not supported");
        ret = -1;
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
    int ret = 0;
    int total_obj_cnt;
    json_error_t jsonErr;
    void *iter;
    const char *key;
    struct DeviceInfo devInfo;
    json_t *jsonrootA = NULL, *jsonrootB = NULL, *valueA, *valueB;

    /*
        Processing
     */
    jsonrootA = json_loadb(a.c_str(), a.size(), 0, &jsonErr);
    if ( NULL == jsonrootA )
    {
        LOGCXX("Error while loading string A: "<< jsonErr.text << " at line: " << jsonErr.line << "ResultVerdictHelper::Compare2JSONData");
        return -1;
    }
    total_obj_cnt = json_object_size(jsonrootA);
    if ( 0 == total_obj_cnt )
    {
        LOGCXX("There is no JSON object inside string A");
        return -1;
    }

    jsonrootB = json_loadb(b.c_str(), b.size(), 0, &jsonErr);
    {
        LOGCXX("Error while loading string B: "<< jsonErr.text << " at line: " << jsonErr.line << "ResultVerdictHelper::Compare2JSONData");
        return -1;
    }
    /*
        Scan all key of A and compare value of each key in A then compare to corresponding value of the key in B
     */
    json_object_foreach(jsonrootA, key, valueA){
        valueB = json_object_get(jsonrootB, key);
        ret = json_equal(valueA, valueB);
        if ( 1 != ret ){
            return 0; // There is an element is not equal.
        }
    }
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
/*
    Function: compare 2 json string input
                compare only specific value only, having measurement uncertainty is probabilityVal (%)
    Type of measurement: measurement uncertaint
    Return:
        1: they are equal
        0: they are diffrent
        -1: undetermined/invalid/error
 */
int ResultVerdictHelper::UncertaintCompare(const string a, const string b, double probabilityVal){
    int ret = 0;


    return ret;
}

/*
    Function: compare 2 json string input
            writespecsInput: input paramters from test cases (structure of test item information)
            readspecsOutput: CERES respond message from read_spec
    Type of measurement: measurement uncertaint
    Return:
        1: they are equal
        0: they are diffrent
        -1: undetermined/invalid/error
 */
int ResultVerdictHelper::ValidateWriteSpecResult(const string writespecsInput, const string readspecsOutput){
    int ret = 0;



    //compare configuration
    


    //compare association

    return ret;
}
/*
    Function: compare 2 value from test case value and from response association array
            testcaseInput: input paramters from test cases, eg: "A"
            nodefollowJSON: Array of followed nodes, eg: ["A","B",...]
    Type of measurement: include or not
    Return:
        1: it is included
        0: it is not included
        -1: undetermined/invalid/error
 */
int ResultVerdictHelper::AssociationSetGetCompare(const string testcaseInput, const json_t * nodefollowJSON){
    int ret = 0;
    size_t index;
    json_t *value;
    string value_cstr;

    json_array_foreach(nodefollowJSON, index, value) {
        value_cstr =  json_string_value(value);
        if( 0 == value_cstr.compare(testcaseInput)){
            ret = 1;
        }
    }
    return ret;
}

/*
    Function: compare 2 values: input from test case , response code from configuration
            testcaseInput: input paramters from test cases, eg: id, class, command, data0, data1, data2...
            nodefollowJSON: Array of followed nodes, eg: { "type": "zwave", "method": "read_specR", "deviceid": "58", "commandinfo": { "class": "CONFIGURATION", "command": "GET", "data0": "TIME_AUTO_REPORT" }, "status": "successful", "parameter": "6F", "size": 4, "value": [ "00", "28", "DE", "80" ] }
    Type of measurement: include or not
    Return:
        1: it is included
        0: it is not included
        -1: undetermined/invalid/error
 */
int ResultVerdictHelper::ConfigurationSetGetCompare(const void* testcaseInput, const json_t * responseMsg){
    int size = 0, ret = 0;
    int valueInt = 0x0;
    size_t index;
    json_t *value;

    // Get size of value which is responded in value field
    value = json_object_get(responseMsg, "size");
    if ( NULL != value){
        size = json_integer_value(value);
    }

    if ( 0 < size ){
         value = json_object_get(responseMsg, "value");
         switch(size){
            case 1:
                
                break;
            case 2: 
                break;
            case 4:

                break;
            default:
                LOGCXX("Size is not supported yet - ResultVerdictHelper::ConfigurationSetGetCompare");
                break;
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
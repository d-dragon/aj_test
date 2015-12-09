#include "ResultVerdictHelper.h"

ResultVerdictHelper::ResultVerdictHelper(){
    mLocalTestItemInfo = new LocalTestItemInfo;
}

ResultVerdictHelper::~ResultVerdictHelper(){
    if (mLocalTestItemInfo != NULL){
        delete mLocalTestItemInfo;
        mLocalTestItemInfo = NULL;
    }
}

void ResultVerdictHelper::SaveInfoOfTestItem(struct TestItemInfo *info){
    mLocalTestItemInfo->s_TIinfo.Signal           = info->Signal;
    mLocalTestItemInfo->s_TIinfo.Type             = info->Type;
    mLocalTestItemInfo->s_TIinfo.ID               = info->ID;
    mLocalTestItemInfo->s_TIinfo.StartLogIndex    = info->StartLogIndex;
    mLocalTestItemInfo->s_TIinfo.EndLogIndex      = info->EndLogIndex;
    mLocalTestItemInfo->s_TIinfo.MatchedLogIndex  = info->MatchedLogIndex;
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
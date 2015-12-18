#ifndef RESULTVERDICTHELPER_H_
#define RESULTVERDICTHELPER_H_
#include <iostream>
#include <string>
#include <vector>
#include <jansson.h>
#include "common_def.h"

using namespace std;


class ResultVerdictHelper{

public:
	/* This struct store response value of sensor unit */
	struct SensorOutputInfo {
		string unitName;
		double value;
	};
    struct LocalTestItemInfo{
        string              Signal;
        string              Type;
        string              ID;
        string              responseMsg;
        string              funcClass;
        string              cmd;
        string              value;
        string              force;
        string              attributeID;
		vector<SensorOutputInfo>	sensorInfo; /*!< Each sensor could report few type of sensing value */
        bool                isValid;
    };
    enum EnumSettingPara{
        FIRST_GET           = 0,
        SECOND_SET          = 1,
        THIRD_GET           = 2,
        MAXBUFF
    };
    /*
        Constructor
     */
    ResultVerdictHelper();
    /*
        Destructor
     */
    ~ResultVerdictHelper();
    /*
        Save info of current test case
     */
    void SaveInfoOfTestItem(const json_t *testInput, struct TestItemInfo *info, string matchedResponseMsg);
    /*
        Get addtional verdict information.
     */
    void GetAddtionalVerdictCondition(string condition);
    /*
        Save reference value
     */
    void SaveReferenceTC(string input, string response);
    /*
        Save write_specs Message: includes response and precondition
     */
    void SaveWriteSpecTC(string input, string response);
    /*
        Save read_spec test case info
     */
    void SaveReadSpecTC(string input, string response);
    /*
        Compare expected of test cases and saved Data.
    */
    int EvaluateExpectationVSSavedData(json_t *expectedData);
    /*
        Do a verdict
     */
    int VerdictResult(json_t*, json_t* refValue =0);
    void DBGPrint();
private:

    // Exactly comparing 2 Json data
    int Compare2JSONData(const string a, const string b);
    int Compare2JSONData(json_t* a, const string b);
    int Compare2JSONData(json_t* a, json_t* b);
	int mInfoLastPosTracker;
    // Compare 2 values, which measurement uncertainty is probabilityVal
    int UncertaintCompare(const string a, const string b, double probabilityVal);
    // To validate write Specs Result, we have to use read_spec to get information. To verify data was setting correct or not.
    int ValidateWriteSpecResult(const string writespecsInput, const string readspecsOutput);
    int AssociationSetGetCompare(const string testcaseInput, const json_t * nodefollowJSON);
    int ConfigurationSetGetCompare(const void* testcaseInput, const json_t * nodefollowJSON);
    /*
    Private members
     */
    struct LocalTestItemInfo mLocalTestItemInfo[MAXBUFF];
    void SetInvalidAllData();
    int EvaluateOnSavedData();
    int EvaluateExpectationofAssociation(string nodeExpected, json_t* respondMsg);

};



#endif //RESULTVERDICTHELPER_H_

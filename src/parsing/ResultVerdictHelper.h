#ifndef RESULTVERDICTHELPER_H_
#define RESULTVERDICTHELPER_H_
#include <iostream>
#include <string>
#include <vector>
#include <jansson.h>
//#include "common_def.h"

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

    struct SignalNameStructure{
        SignalNameEnum     val;
        string             name;
    };
    struct SignalTypeStructure{
        SignalTypeEnum     val;
        string             name;
    };
    struct RWSpecsCommandClassStructure{
        RWSpecsCmdClassEnum     val;
        string                  name;
    };
    SignalTypeStructure signalTypeStr[3] = {{ZWAVE, "zwave"}, {ZIGBEE, "zigbee"}, {UPNP, "upnp"}};
    SignalNameStructure signalNameStr[18] = \
                    {{ADD_DEV,"add_devices"}, {LIST_DEV, "list_devices"}, \
                    {GET_BIN, "get_binary"}, { SET_BIN, "set_binary"}, \
                    {READ_SPEC, "read_spec"}, {WRITE_SPEC, "write_spec"}, \
                    {READ_S_SPEC, "read_s_spec"}, {WRITE_S_SPEC, "write_s_spec"}, \
                    {REMOVE_DEV, "remove_device"}, {RESET, "reset"}, \
                    {UPDATE_FIRM, "update_firmware"}, {SET_RULE, "set_rule"}, \
                    {GET_RULE, "get_rule"}, {RULE_ACT, "rule_actions"}, \
                    {OPEN_CLOSE_NET, "open_closenetwork"}, {GET_SUBDEV, "get_subdevs"}, \
                    {ONBOARDING, "onboarding"}, {LISTEN_NOTIFY, "listen_notification"} };

    RWSpecsCommandClassStructure rwSpecsCommandClassStr[4] = {{CONFIGURATION, "CONFIGURATION"}, {ASSOCIATION, "ASSOCIATION"}, \
                                                        {SENSORMULTILEVEL, "SENSOR_MULTILEVEL"}, {BATTERY, "BATTERY"}};

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


	/**
	 * Verdict result of test case.
	 * This is the last step while run a test case. The result depend on
	 * defined expected output or an reference (in case of sensing).
	 * If verdict based on reference, must be pass the path of references.json.
	 *
	 * @param test_case_t		Test case information after processed all test item.
	 * @param reference_path	Path to references.json
	 *
	 * @return
	 *		#VERDICT_SUCCESS	If verdict success.
	 *		#VERDICT_FAILED		Verdict failed.
	 */
	int VerdictResult(TestCase *test_case_t, const char *reference_path);


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
    SignalTypeEnum GetSignalType(TestCase input, int index = 0);
    SignalNameEnum GetSignalName(TestCase input, int index = 0);
    RWSpecsCmdClassEnum GetRWSpecsClass(TestCase input, int index = 0);

	/**
	 * Validate/verdict test item result
	 *
	 * @param test_ref		Reference fields info must be validated.
	 * @param test_item_t	Struct of test item info.
	 *
	 * @return
	 *		void
	 */
	int ValidateTestItemResult(TestCaseReferenceUnit test_ref, TestItem test_item_t, json_t *ref_root);

	/**
	 * Get test item argument value by key name from test item input argument struct.
	 *
	 * @param test_item_t	Test item struct.
	 * @param key			Name of test item argument.
	 *
	 * @return
	 *		string value of argument.
	 */
	string* GetTIArgumentValueByKey(TestItem test_item_t, string key);

	void GetMsgRespRWSpec(PrivateData*, TestCase , int index = 0);
    int EvaluationTestCase(TestCase&);
    int ExpectationComparison(TestCaseExpectation, PrivateData, string cmdClass);
    int InOutTestCaseComparison(TestCase, PrivateData, string addOrRemove);
    vector<string> GetValueOfTestItem(TestItem, string key);
	string GetValueFromJSON(string json, string obj);
	vector<string> GetArrayValueFromJSON(string json, string obj);
	int GetValueFromJSONInteger(string json, string obj);
};



#endif //RESULTVERDICTHELPER_H_

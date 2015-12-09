#ifndef RESULTVERDICTHELPER_H_
#define RESULTVERDICTHELPER_H_
#include <iostream>
#include <string>
#include <jansson.h>
#include "common_def.h"

using namespace std;


class ResultVerdictHelper{

public:
    struct LocalTestItemInfo{
        struct TestItemInfo s_TIinfo;
        string              responseMsg;
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
    void SaveInfoOfTestItem(struct TestItemInfo *info);
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
        Do a verdict
     */
    int VerdictResult();
private:

    // Exactly comparing 2 Json data
    int Compare2JSONData(const string a, const string b);
    // Compare 2 values, which measurement uncertainty is probabilityVal
    int UncertaintCompare(const string a, const string b, double probabilityVal);
    // To validate write Specs Result, we have to use read_spec to get information. To verify data was setting correct or not.
    int ValidateWriteSpecResult(const string writespecsInput, const string readspecsOutput);
    int AssociationSetGetCompare(const string testcaseInput, const json_t * nodefollowJSON);
    int ConfigurationSetGetCompare(const void* testcaseInput, const json_t * nodefollowJSON);
    /*
    Private members
     */
    struct LocalTestItemInfo *mLocalTestItemInfo;


};



#endif //RESULTVERDICTHELPER_H_
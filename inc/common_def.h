#ifndef _COMMON_DEP_H_
#define _COMMON_DEP_H_
#include "alljoyn/onboarding/Onboarding.h"
#include <string>
#include <vector>

#define LOGCXX(msg)  (std::cout<< "DBG: "  << msg << std::endl )
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/*
*	Session of OnBoarding
*/

enum WifiCbFunc {
GET_VERSION				= 0x00000001,
GET_STATE				= 0x00000002,
GET_LAST_ERR			= 0x00000004,
GET_SCAN_INFO			= 0x00000008,
CONFIG_WIFI				= 0x00000010,
CONNECT_TO				= 0x00000020,
OFF_BOARD_FROM			= 0x00000040,
CONF_AND_CONNECT_WIFI	= 0x00000080
};

struct OnboardingWifiCb {
	enum WifiCbFunc ID;
	char JSKeyName[128];
};
struct WifiAuthenticationType{
    char Name[10];
    ajn::services::OBAuthType Val;

};
struct DeviceInfo{
			std::string Owner;
			std::string Serial;
			std::string FriendlyName;
			std::string ID;
			std::string Capability;
			std::string ProfileID;
			std::string EndPoint_num;
	void operator=(const DeviceInfo& rhs){ Owner=rhs.Owner; Serial=rhs.Serial; FriendlyName=rhs.FriendlyName; ID=rhs.ID; Capability=rhs.Capability;	ProfileID=rhs.ProfileID; EndPoint_num=rhs.EndPoint_num;};
	};
struct TestItemInfo
	{
		std::string Signal;
		std::string Type;
		std::string ID;
		unsigned int StartLogIndex;
		unsigned int EndLogIndex;
		unsigned int MatchedLogIndex;
	};

#define KEY_READSPEC_CMD 				"readcommand"
#define KEY_WRITESPEC_CMD 				"writecommand"
#define KEY_CLASS						"class"
#define SIG_R_SPEC						"read_spec"
#define SIG_W_SPEC						"write_spec"
#define CONFIGURATION					"CONFIGURATION"
#define ASSOCIATION						"ASSOCIATION"
/*
        3: removed successful[Association]
        2: included successful [Association]
        1: they are equal
        0: they are diffrent (failed in setting up)
        -1: undetermined/invalid/error
*/
enum RetCodeEnum{
		ERR_INVALID = -1,
		NOT_EQUAL	= 0,
		IS_EQUAL	= 1,
		ASSOCIATE_OK= 2,
		REMOVE_OK	= 3
};

// Data structure to save all information of a test case.
/*
	Simulate Json key and array of data
*/

struct JsonFormatSimulation{
	std::string 				key;
	std::vector<std::string> 	value;
};

struct  TestItem{
	std::string					name;
	unsigned short				numOfArg;
	JsonFormatSimulation		*testItemArg;
	std::vector<std::string>	testItemLogPool;
	std::string					matchedRespMsg;
};

struct TestCaseExectation{
	JsonFormatSimulation data;
};

struct TestCase{
	std::string  				name;
	unsigned short				numOfTestItem;
	TestItem 					*testItemInfo;
	TestCaseExectation			testExpect;
	std::string					testDesc;
};
#endif

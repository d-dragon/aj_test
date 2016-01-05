#ifndef _COMMON_DEP_H_
#define _COMMON_DEP_H_
#include "alljoyn/onboarding/Onboarding.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

//#define LOGCXX(msg)  (std::cout<<  msg << std::endl )
#define LOGCXX(msg)  do {std::streambuf *gFileBuf, *gConsoleBuf;\
std::ofstream gLogFileStream;\
	gLogFileStream.open("ConsoleLog.txt", std::fstream::out | std::fstream::app);\
	gFileBuf = gLogFileStream.rdbuf();\
	gConsoleBuf = std::cout.rdbuf();\
	std::cout.rdbuf(gFileBuf);\
							std::cout << msg << std::endl;\
							std::cout.rdbuf(gConsoleBuf);\
							std::cout << msg << std::endl;\
							gLogFileStream.close();\
							} while(0)
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
//#define CONFIGURATION					"CONFIGURATION"
//#define ASSOCIATION						"ASSOCIATION"
#define METER_CLASS						"METER"
#define	BATTERY_CLASS					"BATTERY"
#define SENSOR_MULTILEVEL_CLASS			"SENSOR_MULTILEVEL"



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

enum VerdictTypeEnum{
	VERDICT_UNKNOWN		= -1,
	VERDICT_EXPECTED	= 0,
	VERDICT_REFERENCE	= 1
};
enum VerdictReturnTypeEnum{

	VERDICT_RET_INPUT_INVALID =		-4, /*!< Test item input invalid */
	VERDICT_RET_RESP_INVALID,			/*!< Response message info invalid */
	VERDICT_RET_UNKNOWN,				/*!< Unknown error while verdict */
	VERDICT_RET_FAILED,					/*!< Verdict result is failed */
	VERDICT_RET_SUCCESS					/*!< Verdict result is success */
};

typedef struct codestring {
	int Code;
	const char String[128];
} CodeString;

const CodeString gverdictResult[] {
	{VERDICT_RET_INPUT_INVALID, "Input message invalid"},
	{VERDICT_RET_RESP_INVALID, "Response message invalid"},
	{VERDICT_RET_UNKNOWN, "Unknown"},
	{VERDICT_RET_FAILED, "Failed"},
	{VERDICT_RET_SUCCESS, "Success"}
};

// Data structure to save all information of a test case.

/**
 * Simulate Json key and array of data.
 * Each type of data (string, numeric) will be stored by corresponding variable
 */
struct JsonFormatSimulation{
	std::string 				key;
	std::vector<std::string> 	value;
	double						numValue;
};

struct  TestItem{
	std::string					name;
	unsigned short				numOfArg;
	JsonFormatSimulation		*testItemArg;
	std::vector<std::string>	testItemLogPool;
	unsigned int				matchedRespMsgIndex;
	int							verdictResult;
};

struct TestCaseExpectation{
	unsigned short			numOfObject;
	JsonFormatSimulation	*expectedObjs;
};

struct TestCaseReferenceUnit{
	unsigned short			numOfObject;
	JsonFormatSimulation	*referenceUnitObjs;
};

struct TestCase{
	std::string  				name;
	int							verdictType;
	unsigned short				numOfTestItem;
	TestItem 					*testItemInfo;
	TestCaseExpectation			testExpect;
	TestCaseReferenceUnit		testRef;
	std::string					testDesc;
	int							verdictResult;
};
enum SignalTypeEnum{
    ZWAVE = 0,
    ZIGBEE,
    UPNP,
    UNKNOWN
};
enum SignalNameEnum{
    ADD_DEV = 0,
    LIST_DEV,
    GET_BIN,
    SET_BIN,
    READ_SPEC,
    WRITE_SPEC,
    READ_S_SPEC,
    WRITE_S_SPEC,
    REMOVE_DEV,
    RESET,
    UPDATE_FIRM,
    SET_RULE,
    GET_RULE,
    RULE_ACT,
    OPEN_CLOSE_NET,
    GET_SUBDEV,
    ONBOARDING,
    LISTEN_NOTIFY,
    UNSUPPORTED
};
enum RWSpecsCmdClassEnum{
    CONFIGURATION = 0,
    ASSOCIATION,
    SENSORMULTILEVEL,
    BATTERY,
    RESERVED
};
/*Function pointers*/
struct ConfigurationRespMesg{
	std::string type;
	std::string method;
	std::string devID;
	std::string cmdInfo;// Not parse this info yet
	std::string status;
	std::string reason;
	std::string parameter;
	int value;
};
struct AssociationRespMesg{
	std::string type;
	std::string method;
	std::string devID;
	std::string cmdInfo;// Not parse this info yet
	std::string status;
	std::string reason;
	int groupid;
	int maxnode;
	std::vector<std::string> nodefollow;
};
struct PrivateData{
    ConfigurationRespMesg   msgConf;
    AssociationRespMesg     msgAssociate;
    RWSpecsCmdClassEnum     cmdClass;
};


#endif

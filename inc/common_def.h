#ifndef _COMMON_DEP_H_
#define _COMMON_DEP_H_


/*
*	Session of OnBoarding
*/

enum WifiCbFunc {
GET_VERSION		= 0x00000001,
GET_STATE		= 0x00000002,
GET_LAST_ERR	= 0x00000004,
GET_SCAN_INFO	= 0x00000008,
CONFIG_WIFI		= 0x00000010,
CONNECT_TO		= 0x00000020,
OFF_BOARD_FROM	= 0x00000040
};

struct OnboardingWifiCb {
	enum WifiCbFunc ID;
	char JSKeyName[128];
};

#endif

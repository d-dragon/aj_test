=============================================
Project name: Alljoyn Test Tool
	Developer: Tuan Ngo, Duy Phan
	Scum Master, Product Owner: Vinh Nguyen

Pupose: Testing alljoyn service functions

=============================================
A. Outline:
=============================================

1. Build guide/requirement
2. Running app
3. Limitation

=============================================
B. Description detail:
=============================================
1. Building
	1.1 Build requirement
		- Build environment x86_64, Linux environment
	1.2 Building
		$ make
	1.3 Clean all build
		$ make clean
	1.4 Directory structure
	prj-aj-test-tool/
	├── bin/ 					: Output executable binary of CERES test tool
	├── build/ 					: Build folder of Alljoyn Core source( Created while building Alljoyn)
	├── common_libs/			: Current store tar.gz source of Alljoyn
	├── inc/					: Header files
	├── lib/					: Lib files (alljoyn, onboarding ...)
	├── Makefile				: Makefile
	├── obj/					: Object dir while building the app
	├── README.md 				: This README.md file
	└── src/					: Source of CRES test tool
2. Running app
	2.1 Run Alljoyn onboarding
	Note: Authen type and corresponding values
	WPA2_AUTO = -3,                            //!< WPA2_AUTO authentication
    WPA_AUTO = -2,                           //!< WPA_AUTO authentication
    ANY = -1,                           //!< ANY authentication
    OPEN = 0,                          //!< OPEN authentication
    WEP = 1,                           //!< WEP authentication
    WPA_TKIP = 2,                           //!< WPA_TKIP authentication
    WPA_CCMP = 3,                           //!< WPA_CCMP authentication
    WPA2_TKIP = 4,                            //!<WPA2_TKIP authentication
    WPA2_CCMP = 5,                        //!<WPA2_CCMP authentication
    WPS = 6,                          //!<WPS authentication
	
	This sample app include below arguments:
	-s: input string SSID [string]
	-p: input passcode string (in hex, convert from string into hex http://string-functions.com/string-hex.aspx) [string]
	-a: authen type as mentioned above from -3 --> 6 [interger]
	-r: reset wifi [int]; value 1 will reset, others is not.

	2.1.1 Connect to wifi
		Then we can run it now, make on boarding, we have to make PC connect wifi info AP of device:

		$ LD_LIBRARY_PATH=./lib ./bin/OnboardingTestApp -s "VEriK2" -p "564572694b73797374656d733130" -a 4
		After send command successful, as below log:
		"Call to ConfigureWiFi succeeded 
		Call to ConnectTo succeeded"

		Please Ctrl+C to kill this app.
	2.1.2 Disconnect wifi
		To reset wifi connection, connect in to the network which devices has been connected then do:
		$ LD_LIBRARY_PATH=./lib ./bin/OnboardingTestApp -r 1

		Please press Ctrl+C to kill the app when we see the log as below: 
		"OBLastError code=0 message= Validated
		Call to OffboardFrom succeeded"
	2.1.3 Note:
		Commands at 2.1.1 and 2.1.2 were executed in terminal at prj-aj-test-tool/ folder


        2.2 Run Alljoyn client
                Navigate to root directory of project source
                $ export LD_LIBRARY_PATH=./lib
                $ ./bin/alljoynclient
3. Limitation:
	This guide is made for simple test function of onboarding, it may have others issue.

=============================================

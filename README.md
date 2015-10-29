=============================================
Project name: Alljoyn Test Tool
	Developer: Tuan Ngo, Duy Phan
	Scum Master, Product Owner: Vinh Nguyen

Pupose: Testing alljoyn service functions
History: 
	1. Oct 22: Modify 2.1 due to update Parsing Module JSON
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
	├── src/					: Source of CRES test tool
	└── wifi.json				: Sample configuration wifi
2. Running app
	2.1 Run Alljoyn onboarding
	This app will read configuration from wifi.json to get information of wifi configuration
	and it also configure which functions would be called when neccessary.
	Example wifi.json:
		{
		"VEriK_Belkin": {
					"ssid": "VEriK_Belkin",
					"passcode": "766572696b73797374656d7332303135",
					"auth": 4,
					"getVersion": 1,
					"getState": 0,
					"getLastErr": 1,
					"getScanInfo": 0,
					"configAndConnectWifi": 0,
					"offBoard": 0
			}
		}
			
	Note of suitable value:
	"ssid" : string name of SSID dev would be connected
	"passcode": string password of wifi was encoded in hex; to convert from string into hex, you can use the tool  http://string-functions.com/string-hex.aspx
	"auth": interger value, authentication type and corresponding values
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
	"getVersion", ... "offBoard":	those keys stand for functions could be call from Onboardee (this Onboarding test app).
									Set 	1: if you want to call it.
											0: means do not call it. 	

	Please fill suitable value before run the test App
	2.1.1 Connect to wifi
		Then we can run it now, make on boarding, we have to make PC connect wifi info AP of device:
		Requirement: correct value of ssid, passcode, auth, configAndConnectWifi=1 , and do not enable offBoard.

		$ LD_LIBRARY_PATH=./lib ./bin/OnboardingTestApp

		After send command successful, as below log:
		"Call to ConfigureWiFi succeeded 
		Call to ConnectTo succeeded"

		Please Ctrl+C to kill this app.
	2.1.2 Disconnect wifi
		To reset wifi connection, connect in to the network which devices has been connected then do:
		Requirement: offBoard must be 1.
		$ LD_LIBRARY_PATH=./lib ./bin/OnboardingTestApp 

		"OBLastError code=0 message= Validated
		Call to OffboardFrom succeeded"
	2.1.3 Note:
		Commands at 2.1.1 and 2.1.2 were executed in terminal at prj-aj-test-tool/ folder

    2.2 Run Alljoyn client
		Navigate to root directory of project source
        $ export LD_LIBRARY_PATH=./lib
        $ ./bin/alljoynclient
3. Limitation:
	This guide is made for simple test function of onboarding/about, it may still contain other issue(s).

====================THE END=========================

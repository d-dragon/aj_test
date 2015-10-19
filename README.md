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

=============================================
Description detail
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
		LD_LIBRARY_PATH=./lib ./bin/OnboardingTestApp
	2.2
=============================================

=============================================
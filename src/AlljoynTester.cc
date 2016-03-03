/*main app*/
#include <iostream>
#include <string>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include "JsonParser.h"
#include "TestWorker.h"
#include "common_def.h"

#define TRUE 1
#define FALSE 0
#define PROGRAM_NAME "AlljoynTester"
#define VERSION "1.0"
using namespace std;


int g_update_config = TRUE;
int g_run_all = FALSE;
int g_test_suite_number = 0;
int g_reference_enabled = FALSE;
JsonParser *tester; 

streambuf *gFileBuf, *gConsoleBuf;
ofstream gLogFileStream;

void MoveConsoleLogToOutputDir() {

	char cmd_buf[256];

	snprintf(cmd_buf, 256, "mv ConsoleLog.txt %s/", tester->aReporter.mReportDirPath);
	system(cmd_buf);
}
static void SignalHander(int signum) {

	switch (signum) {
		case SIGINT:
			LOGCXX("------catched interupt signal-----------");
			MoveConsoleLogToOutputDir();
			exit(1);
			break;
		case SIGSEGV:
			LOGCXX("------catched SIGSEGV signal-----------");
			MoveConsoleLogToOutputDir();
			exit(1);
			break;
		defaut:
			break;
	}

}
void SetSignalHandler(void) {
	struct sigaction act, old_act;
	act.sa_flags = (SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART | SA_SIGINFO);
	act.sa_handler = SignalHander;

	if (sigaction(SIGSEGV, &act, &old_act)) {
		cout << "sigaction failed... errno: " << errno << endl;
	} else {
		if (old_act.sa_handler == SIG_IGN) {

			cout << "oldact RTMIN: SIGIGN" << endl;
		}
		if (old_act.sa_handler == SIG_DFL) {

			cout << "oldact RTMIN: SIGDFL" << endl;
		}
	}
	if (sigaction(SIGINT, &act, &old_act)) {

		cout << "sigaction failed... errno: " << errno;
	} else {
		if (old_act.sa_handler == SIG_IGN) {

			cout << "oldact RTMIN: SIGIGN" << endl;
		}
		if (old_act.sa_handler == SIG_DFL) {

			cout << "oldact RTMIN: SIGDFL" << endl;
		}
	}




}
static void program_help(char *program_name) {
	
	cout << PROGRAM_NAME << " - Version " << VERSION << endl;
	cout << "Usage: " << PROGRAM_NAME << " [OPTION]... [PATH]..." << endl;
	cout << "Supported options: " << endl;
	cout << "\t-a, --all\t\tRun all test suite follow prefix number of file name." << endl;
	cout << "\t-t, --testsuite\t\tChoose specific test suite number." << endl;
//	cout << "\t-u, --update\t\tUpdate test suite configuration. Default use current value of configuration.json" << endl;
	cout << "\t-r, --reference\t\tEnable run test suite as a reference" << endl;
	cout << "\t-v, --version\t\tPrint Version." << endl;
	cout << "\t-h, --help\t\tPrint Helper." << endl;
}

static void parse_options(int argc, char* argv[]) {

	int c;
	if(argc == 1) {
		program_help(argv[0]);
		exit(1);
	}
	while(1) {
		
		int option_index = 0;
		static struct option long_options[] = {
//			{"update",		no_argument,		0,	'u'},
			{"all",			no_argument,		0,	'a'},
			{"testsuite",	required_argument,	0,	't'},
			{"reference",	no_argument,		0,	'r'},
			{"version",		no_argument,		0,	'v'},
			{"help",		no_argument,		0,	'h'},
			{0,				0,					0,	0}

		};

		c =getopt_long(argc, argv,"uart:vh0", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch(c) {
			case 'a':
				g_run_all = TRUE;
				break;
			case 't':
				g_test_suite_number = atoi(optarg);
				break;
/*
			case 'u':
				g_update_config = FALSE;
				break;
*/
			case 'r':
				g_reference_enabled = TRUE;
				break;
			case 'h':
				program_help(argv[0]);
				exit(1);
			case 'v':
				cout << "Version " << VERSION << endl;
				exit(0);
			case '?':
				if(optopt == 't') {
					cout << " Option " << optopt << "requires an argument." << endl;
				} else if (isprint(optopt)) {
					cout << "Unknown option " << optopt << endl;
				} else {
					cout << "Unkown option character" << endl;
				}
				exit(1);
			default:
				cout << "getopt returned???" << endl;
				exit(1);
		}
	}
/*
	if (optind < argc) {
		while(optind < argc) {
			cout << argv[optind++] << endl;
		}
	}
*/
}
#if 0
void InitLogStream() {
	gLogFileStream.open("console.txt");
	gFileBuf = gLogFileStream.rdbuf();
	gConsoleBuf = cout.rdbuf();
}
#endif
int main(int argc, char *argv[]){

	int found_config_flag = 0;
	int ret;
	char ts_path[LEN_256B];
	char tc_path[LEN_256B];
	char config_path[LEN_256B];
	char reference_path[LEN_256B];
	vector<string> ts_list;


	SetSignalHandler();
	parse_options(argc, argv);
	//cout << g_update_config << g_run_all << g_test_suite_number << argv[optind] << endl;
	tester = new JsonParser(argv[1], argv[2], "src/testcases/testitem.json", "src/testcases/configuration.json");

	gLogFileStream.open("ConsoleLog.txt", std::fstream::out | std::fstream::app);
	gFileBuf = gLogFileStream.rdbuf();
	gConsoleBuf = std::cout.rdbuf();
	
	
	const char * dir_path = argv[optind++];
	if (NULL == dir_path) {
		cout <<  "Found no test suite path :( :(" << endl;
		program_help(argv[0]);
		exit(1);
	}

	tester->GetTestSuiteFileList(dir_path);
	cout << "number of file: " << tester->mFileList.size() << endl;
	for (int i = 0; i < tester->mFileList.size(); i++) {
		cout << "\t" << tester->mFileList[i] << endl;
		if (0 == tester->mFileList[i].compare("configuration.json")) {
			found_config_flag = 1;
		} else if (0 == tester->mFileList[i].compare("testcase.json")) {
			/* ignore this file */
			snprintf(tc_path, LEN_256B, "%s/%s", dir_path, tester->mFileList[i].c_str());

		} else if (0 == tester->mFileList[i].compare("references.json")) {
			snprintf(reference_path, LEN_256B, "%s/%s", dir_path, tester->mFileList[i].c_str());
			cout<<"Found references: "<<reference_path<<endl;
		} else {
			ts_list.push_back(tester->mFileList[i]);
		}
	}
	
	/* If there is configuration file in test suite directory, use it or update value. 
	 * If not-> create one
	 */
	if (0 == found_config_flag) {
		cout << "found no configuration.json -> create one" << endl;
		exit(1);
	} else{
		cout << "found configuration file" << endl;
		snprintf(config_path, LEN_256B, "%s/%s", dir_path, CONFIG_FILE);
		tester->PrintConfigurationInfo(config_path);

		char option;
		cout << "Do you want to update configuration?(y/n)";
		cin.get(option);
		if (option == 'y') {
			/* Update configuration value*/

			cin.get(); //clean cin buffer
			ret = tester->UpdateConfiguration(config_path);
			if (ERROR == ret) {

				cout << "Program terminated:(" << endl;
				exit(1);
			}
			tester->PrintConfigurationInfo(config_path);
		}else {
			cout << "Keep current configuration!!!" << endl;
		}
	}

	/* Choose the test suite will be executed */
	cout << "*************Test Suites*****************" << endl;
	for (int i = 0; i < ts_list.size(); i++) {
		cout << "*\t" << i+1 << " - " << ts_list[i] << endl;
	}
	cout << "*****************************************" << endl;
	if (g_test_suite_number == 0) {
		
		cout << "Choose test suite: ";
		cin >> g_test_suite_number;
		LOGCXX("------------------------------------------------------------");
		cout << "Parsing " << ts_list[g_test_suite_number - 1] << endl;

	}
	snprintf(ts_path, LEN_256B, "%s/%s", dir_path, ts_list[g_test_suite_number -1].c_str());

	tester->ApplyPaths(ts_path, tc_path, config_path, reference_path);
	tester->startParser(g_reference_enabled);
	MoveConsoleLogToOutputDir();
	gLogFileStream.close();
	return 1;
}

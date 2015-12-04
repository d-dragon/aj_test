/*main app*/
#include <iostream>
#include <string>
#include <string.h>
#include "JsonParser.h"
#include "TestWorker.h"

using namespace std;
int main(int argc, char *argv[]){

	int found_config_flag = 0;
	int ret;
	const char * dir_path = argv[1];
	char ts_path[LEN_256B];
	char tc_path[LEN_256B];
	char config_path[LEN_256B];
	vector<string> ts_list;


	JsonParser *tester = new JsonParser(argv[1], argv[2], "src/testcases/testitem.json", "src/testcases/configuration.json");
	tester->GetTestSuiteFileList(dir_path);
	cout << "number of file: " << tester->mFileList.size() << endl;
	for (int i = 0; i < tester->mFileList.size(); i++) {
		cout << "\t" << tester->mFileList[i] << endl;
		if (0 == tester->mFileList[i].compare("configuration.json")) {
			found_config_flag = 1;
		} else if (0 == tester->mFileList[i].compare("testcase.json")) {
			/* ignore this file */
			snprintf(tc_path, LEN_256B, "%s/%s", dir_path, tester->mFileList[i].c_str());

		} else {
			ts_list.push_back(tester->mFileList[i]);
		}
	}
	
	/* If there is configuration file in test suite directory, use it or update value. 
	 * If not-> create one
	 */
	if (0 == found_config_flag) {
		cout << "found no configuration.json -> create one" << endl;
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
	cout << "Choose test suite: ";
	int ts_index;
	cin >> ts_index;
	cout << ts_list[ts_index - 1] << endl;
	snprintf(ts_path, LEN_256B, "%s/%s", dir_path, ts_list[ts_index -1].c_str());

	tester->ApplyPaths(ts_path, tc_path, config_path);
	tester->startParser();
	return 1;
}

#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "Reporter.h"



Reporter::Reporter(){
};

Reporter::~Reporter(){
	mFullReport.close();
	mCvsReport.close();
};
int Reporter::InitOutputReportDir(const char *rDeviceName) {

	int status = 0;

	time_t raw_time;
	struct tm *time_info;
	char time_buff[128];
	char full_report_path[256];
	char cvs_report_path[256];

	time(&raw_time);
	time_info = localtime(&raw_time);

	snprintf(time_buff, 128, "%d-%d-%d_%d:%d:%d", time_info->tm_mon, time_info->tm_mday, (time_info->tm_year - 100 + 2000), time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
	snprintf(mReportDirPath, 256, "output/%s_%s", time_buff, rDeviceName);

	if (-1 == mkdir(mReportDirPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
		status = RET_ERR;
	} else {

		snprintf(full_report_path, 256, "%s/%s", mReportDirPath, FULL_REPORT_NAME);
		snprintf(cvs_report_path, 256, "%s/%s", mReportDirPath, SUMMARY_REPORT_NAME);
		
		status = CreateReportFile(REPORT_TYPE_FULL, full_report_path);
		if (RET_ERR == status) {
			rmdir(mReportDirPath);
			return status;
		}
		status = CreateReportFile(REPORT_TYPE_SUMMARY, cvs_report_path);
		if (RET_ERR == status) {
			rmdir(mReportDirPath);
			return status;
		}
	}
	
	return status;
};

int Reporter::CreateReportFile(int rReportType, const char *rFilePath) {
	
	int status = RET_OK;
	
	if (NULL == rFilePath) {
		cout << "File name is null"	<< endl;
		return -1;
	}

	switch(rReportType) {
		case REPORT_TYPE_FULL:
		mFullReport.open(rFilePath, fstream::out | fstream::app);
		if(!mFullReport.is_open()){
			cout << "can not open file " << rFilePath <<  endl;
			status = RET_ERR;
		}
		break;

		case REPORT_TYPE_SUMMARY:
		mCvsReport.open(rFilePath, fstream::out | fstream::app);
		if(!mCvsReport.is_open()){
			cout << "can not open file " << rFilePath <<  endl;
			status = RET_ERR;
		}
		break;

		case REPORT_TYPE_TEST_SUITE:
		mTestSuiteReport.open(rFilePath, fstream::out | fstream::app);
		if(!mTestSuiteReport.is_open()){
			cout << "can not open file " << rFilePath <<  endl;
			status = RET_ERR;
		}
		break;

		default:
		cout << "File type is invalid" << endl;
		status = RET_ERR;
	}

	return status;
}

int Reporter::CreateTestSuiteReport(const char *rTestSuiteName) {

	int status = RET_OK;
	char ts_report_path[256];
	
	snprintf(ts_report_path, 256, "%s/%s.html", mReportDirPath, rTestSuiteName);

	status = CreateReportFile(REPORT_TYPE_TEST_SUITE, ts_report_path);
	if (RET_ERR == status) {
		cout << "cannot open file " << ts_report_path << endl;
	}
	return status;
}

int Reporter::WriteContentToReport(int rReportType, string rContent) {
	int status = RET_OK;
	
	switch (rReportType) {
		case REPORT_TYPE_FULL:
			mFullReport << rContent; 
			mFullReport << std::flush; /*!<Force write data to file*/
			break;
		case REPORT_TYPE_SUMMARY:
			mCvsReport << rContent;
			mCvsReport << std::flush; /*!<Force write data to file*/
			break;
		case REPORT_TYPE_TEST_SUITE:
			mTestSuiteReport << rContent;
			mTestSuiteReport << std::flush;
			break;
		default:
			status = RET_ERR;

	}

	return status;
}

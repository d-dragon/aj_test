#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include "Reporter.h"
#include "common_def.h"



Reporter::Reporter(){
};

Reporter::~Reporter(){
	aFullReport.close();
	aCvsReport.close();
};
int Reporter::InitOutputReportDir(const char *pdeviceName) {

	int status = 0;

	time_t raw_time;
	struct tm *time_info;
	char time_buff[128];
	char full_report_path[256];
	char cvs_report_path[256];
	char test_suites_path[256];

	time(&raw_time);
	time_info = localtime(&raw_time);

	snprintf(time_buff, 128, "%d-%d-%d_%d:%d:%d", time_info->tm_mon + 1, time_info->tm_mday, (time_info->tm_year - 100 + 2000), time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
	snprintf(mReportDirPath, 256, "output/%s_%s", time_buff, pdeviceName);
	snprintf(test_suites_path, 256, "%s/testsuites", mReportDirPath);

	if (-1 == mkdir(mReportDirPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
		status = RET_ERR;
	} else {

		if (-1 == mkdir(test_suites_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
			rmdir(mReportDirPath);
			return RET_ERR;
		}
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

int Reporter::CreateReportFile(int reportType, const char *pfilePath) {
	
	int status = RET_OK;
	
	if (NULL == pfilePath) {
		cout << "File name is null"	<< endl;
		return -1;
	}

	switch(reportType) {
		case REPORT_TYPE_FULL:
		aFullReport.open(pfilePath, fstream::out | fstream::app);
		if(!aFullReport.is_open()){
			cout << "can not open file " << pfilePath <<  endl;
			status = RET_ERR;
		}
		break;

		case REPORT_TYPE_SUMMARY:
		aCvsReport.open(pfilePath, fstream::out | fstream::app);
		if(!aCvsReport.is_open()){
			cout << "can not open file " << pfilePath <<  endl;
			status = RET_ERR;
		}
		break;

		case REPORT_TYPE_TEST_SUITE:
		aTestSuiteReport.open(pfilePath, fstream::out | fstream::app);
		if(!aTestSuiteReport.is_open()){
			cout << "can not open file " << pfilePath <<  endl;
			status = RET_ERR;
		}
		break;

		default:
		cout << "File type is invalid" << endl;
		status = RET_ERR;
	}

	return status;
}

int Reporter::CreateTestSuiteReport(const char *ptestSuiteName) {

	int status = RET_OK;
	char ts_report_path[256];
	
	snprintf(ts_report_path, 256, "%s/testsuites/%s.html", mReportDirPath, ptestSuiteName);

	status = CreateReportFile(REPORT_TYPE_TEST_SUITE, ts_report_path);
	if (RET_ERR == status) {
		cout << "cannot open file " << ts_report_path << endl;
	}
	return status;
}


int Reporter::WriteContentToReport(int reportType, const char *pcontent, ...) {
	int status = RET_OK;
	char content_buff[MAX_CONTENT_BUFF];
	
	va_list args;
	va_start(args, pcontent);
	vsnprintf(content_buff, MAX_CONTENT_BUFF, pcontent, args);
	va_end(args);

	switch (reportType) {
		case REPORT_TYPE_FULL:
			aFullReport << content_buff; 
			aFullReport << std::flush; /*!<Force write data to file*/
			break;
		case REPORT_TYPE_SUMMARY:
			aCvsReport << content_buff;
			aCvsReport << std::flush; /*!<Force write data to file*/
			break;
		case REPORT_TYPE_TEST_SUITE:
			aTestSuiteReport << content_buff;
			aTestSuiteReport << std::flush;
			break;
		default:
			status = RET_ERR;

	}

	return status;
}
void Reporter::CloseTestSuiteReport() {
	aTestSuiteReport.close();
}

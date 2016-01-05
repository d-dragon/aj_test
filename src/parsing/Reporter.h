#include <iostream>
#include <fstream>
#include <string>

enum ReportTypeE{
	REPORT_TYPE_FULL,
	REPORT_TYPE_SUMMARY,
	REPORT_TYPE_TEST_SUITE
};

enum ReportStatusE{
	RET_ERR = -1,
	RET_OK
};

const char * const FULL_REPORT_NAME = "Full_Report.html";
const char * const SUMMARY_REPORT_NAME = "Summary.csv";
const int MAX_CONTENT_BUFF = 1024;

using namespace std;
class Reporter {

	public:

		Reporter();
		~Reporter();
		int InitOutputReportDir(const char *pdeviceName);
		int CreateTestSuiteReport(const char *ptestSuiteName);
		void CloseTestSuiteReport();
		int WriteContentToReport(int reportType, const char *pcontent, ...);
		char mReportDirPath[256];

	private:

		fstream aFullReport;
		fstream aCvsReport;
		fstream aTestSuiteReport;
		int CreateReportFile(int reportType, const char *pfilePath);
};

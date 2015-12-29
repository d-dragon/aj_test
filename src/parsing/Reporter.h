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
const char * const SUMMARY_REPORT_NAME = "Summary.cvs";

using namespace std;
class Reporter {

	public:

		Reporter();
		~Reporter();
		int InitOutputReportDir(const char *rDeviceName);
		int CreateTestSuiteReport(const char *rTestSuiteName);
		int WriteContentToReport(int rReportType, string rContent);

	private:

		fstream mFullReport;
		fstream mCvsReport;
		fstream mTestSuiteReport;
		char mReportDirPath[256];
		int CreateReportFile(int rReportType, const char *rFilePath);
};

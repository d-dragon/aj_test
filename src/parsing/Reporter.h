#include <iostream>
#include <fstream>
#include <string>

using namespace std;
class Reporter {

	public:

		Reporter();
		~Reporter();
		int InitOutputReportDir(const char *rDeviceName);
		int WriteContentToReport(int rReportType, string rContent);

	private:

		fstream mHtmlReport;
		fstream mCvsReport;
		int CreateHtmlReportFile(const char *rFileName);
		int CreateCvsReporeFile(const char *rFileName);
};

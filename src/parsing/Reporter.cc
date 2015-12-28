#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include "Reporter.h"



Reporter::Reporter(){
};

Reporter::~Reporter(){
};
int Reporter::InitOutputReportDir(const char *rDeviceName) {

	int status = 0;

	time_t raw_time;
	struct tm *time_info;
	char time_buff[128];
	char report_dir[128];
	char html_report_path[128];
	char cvs_report_path[128];

	time(&raw_time);
	time_info = localtime(&raw_time);

	snprintf(time_buff, 128, "%d-%d-%d_%d:%d:%d", time_info->tm_mon, time_info->tm_mday, time_info->tm_year, time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
	snprintf(report_dir, 256, "output/%s_%s", rDeviceName, time_buff);

	if (-1 == mkdir(report_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
		status = -1;
	} else {

	}
	
	return status;
};

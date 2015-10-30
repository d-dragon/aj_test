#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

#include <iostream>
#include <string>
#include <jansson.h>

using namespace std;

class JsonParser {

	public:
		JsonParser(const char *tsPath, const char *tcPath, const char *tiPath);
		~JsonParser();
		int startParser();

	private:
		json_t *testSuitRoot;
		json_error_t err;
		const char *dfTSPath;
		const char *dfTCPath;
		const char *dfTIPath;


		int TestsuitParser(json_t *tsObj);
		int TestCaseCollector(json_t *tcRoot);
		int TestItemProcessor(json_t *inputArg, json_t *tiObj);
		json_t *getTestItemTemplateObj(const char *tiName);

};
#endif


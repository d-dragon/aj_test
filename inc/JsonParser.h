#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

#include <iostream>
#include <string>
#include <jansson.h>
#include <typeinfo>
#include "TestWorker.h"

using namespace std;

class JsonParser {

	public:
		JsonParser(const char *tsPath, const char *tcPath, const char *tiPath, const char *configPath);
		~JsonParser();
		int startParser();

		//return string length
		
		void JSONGetObjectValue(string *inputString, string objectName, string *output);
		void JSONGetObjectValue(json_t *inputObj, string objectName, string *output);
		json_t *JSONGetObjectFromString(string *inputString, string objectName);


		template<class Type1> 
		void JSONGetStuff(Type1 *input,string objectName, int index, Type1 *output);
		static int GetDevIDInJSMsg(string *input, vector<DeviceInfo> *devList);

	private:
		json_t *testSuitRoot;
		json_error_t err;
		TestWorker *worker;
		json_t *tcTemplateRoot;
		json_t *tiRoot;

		const char *dfTSPath;
		const char *dfTCPath;
		const char *dfTIPath;
		const char *dfConfigPath;

		int TestsuitParser(json_t *tsObj);
		int TestCaseCollector(json_t *tcRoot);
		int TestItemProcessor(json_t *inputArg, json_t *tiObj);
		json_t *getTestItemTemplateObj(const char *tiName);
		int UpdateWorkerConfiguration(TestWorker *worker, const char *dfConfigPath);

};

template<class Type1>
void JsonParser::JSONGetStuff(Type1 *input, string objectName, int index, Type1 *output){
	json_t *rootObj;
	json_t *targetObj;
	const char *retString;
	int strLen;

	if(strcmp(typeid(input).name(), "PSs") == 0){
//	if(!(json_is_array(input)) && !(json_is_object(input))){


		rootObj = json_loads(input->c_str(), 0, &err);
		if(!json_is_object(rootObj)){
			cout << "json format invalid" << endl;
		}

	
		targetObj = json_object_get(rootObj, objectName.c_str());
		if (targetObj == NULL){
			cout << "found no status object" << endl;
		}
		retString = json_string_value(targetObj);
		if(retString != NULL){
			output->assign(retString);
		}
	}else{
/*		
		if(json_is_array(input)){
			output = json_array_get(input, index);
		}else{
			output = json_object_get(input, objectName.c_str());
		}
*/
	}

		
	json_decref(rootObj);
	json_decref(targetObj);	

}
#endif


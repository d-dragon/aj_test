#ifndef PARSINGMODULE_H_
#define PARSINGMODULE_H_

#include <jansson.h>

class ParsingModule{

public:
	/*
		Constructor and Destructor
	 */
	ParsingModule();
	~ParsingModule();

private:
	int LoadJSON();
	json_t *mJSONroot;
};

#endif
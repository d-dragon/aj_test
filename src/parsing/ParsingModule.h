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
	/* Load JSON from input file
	 * Input: const char* location of file
	 * Output: 0: success; other: failed
	 */
	
	int LoadJSONFromFile(const char* path);
	void DumpJSONFile();

private:
	int LoadJSON();
	json_t *mJSONroot;
	FILE *mInputFile;
	const char* filePath;
};

#endif

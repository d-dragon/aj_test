#ifndef PARSINGMODULE_H_
#define PARSINGMODULE_H_

#include <jansson.h>

class ParsingModule{

public:
	/*
	 * Constructor and Destructor
	 */
	ParsingModule();
	~ParsingModule();
	/* Load JSON from input file
	 * Input: const char* location of file
	 * Output: 0: success; other: failed
	 */
	
	int LoadJSONFromFile(const char* path);
	/*
	 * Dump all JSON file into console
	 */
	void DumpJSONFile();
	/*
	 * Get type of JSON input type
	 */
	json_type GetJSONType(json_t*);
	/*
	 * Searching in JSON root
	 */
	void TraversalJSONData();
		

private:
	/*
	 * JSON handle pointer
	 */
	json_t *mJSONroot;
	void TraversalJSONObject(json_t *);
	void TraversalJSONDataLocal(json_t *);
	void TraversalJSONArray(json_t*);
};

#endif

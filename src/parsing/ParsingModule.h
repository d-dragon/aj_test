#ifndef PARSINGMODULE_H_
#define PARSINGMODULE_H_

#include <jansson.h>
#define MAX_KEY_SZ 1024
#define MAX_STR_SZ 1024*4
struct JSONSearchInfo{
	json_type type;
	char inKey[MAX_KEY_SZ];
	char outString[MAX_STR_SZ];
	int outInt;
	double outReal;
	bool outBool;
	char* outNULL;
	
};

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
	 * Searching in JSON root
	 */
	void TraversalJSONData();
		

	/*
	 * Get value of a key
	 * If key existed, return 0
	 * Other return 1
	 */
	int GetValueOfKey(const char *input, void*output[]);
private:

	/*
	 * JSON handle pointer
	 */
	json_t *mJSONroot;
	/*
	 * Key and value to return for searching
	 */
	bool isMatch;
	struct JSONSearchInfo mSearchResponse;

	/*
	 * Get type of JSON input type
	 */
	json_type GetJSONType(json_t*);
	/*
	 * Scan all data inside Object
	 */
	void TraversalJSONObject(json_t *);
	/*
	 * Scan all element in json_root
	 */
	void TraversalJSONDataLocal(json_t *);
	/*
	 * Scan to get data in an array
	 */
	void TraversalJSONArray(json_t*);
};

#endif

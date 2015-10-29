#ifndef PARSINGMODULE_H_
#define PARSINGMODULE_H_
#include <alljoyn/onboarding/Onboarding.h>

#include <jansson.h>
#include <vector>
#define MAX_KEY_SZ 128
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

     // Onboarding parsing part
    int GetNumOfWifiConfiguration();
    // Get configuration of of wifi at pos
    int GetWifiConfiguration(ajn::services::OBInfo*, long*, int pos);
    
private:
	// Get key at index of test cases
    const char *GetKeyAtIndex(int pos);

	/*
	 * JSON handle pointer
	 */
	json_t *mJSONroot;
    std::vector<void *> mJSONiter;

};

#endif

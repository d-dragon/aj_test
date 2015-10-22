#ifndef PARSINGMODULE_H_
#define PARSINGMODULE_H_
#include <alljoyn/onboarding/Onboarding.h>

#include <jansson.h>
#define MAX_KEY_SZ 128
#define MAX_STR_SZ 1024*4

#define LOGCXX(msg)  (std::cout<< "DBG: " << __FILE__ << "(" << __LINE__ << ") "  << msg << std::endl )
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
	 * Traversal all data in JSON root
	 */
	void TraversalJSONData();
		

	/*
	 * Get value of a key
	 * If key existed, return 0
	 * Other return 1
	 */
	int GetValueOfKey(const char *input, void*output);
    /*
     *
    */
    json_t * GetObjectOfKey(const char *key);
    //Get number number of elements match the keys
    int GetNumberOfKey(const char *key);

     // Onboarding parsing part
    int GetNumOfWifiConfiguration();
    int GetNextWifiConfiguration(ajn::services::OBInfo*, long*);
private:

	/*
	 * JSON handle pointer
	 */
	json_t *mJSONroot;
// Point to object which store searchKey
    json_t *mJSONcurrent;
// Store key search
    char searchKey[MAX_KEY_SZ];
// Number of elements inside of key
    int mNumberOfElement;
    void *mJSONiter;
	/*
	 * Key and value to return for searching
	 */
	bool isMatch;
//	struct JSONSearchInfo *mSearchResponse;

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

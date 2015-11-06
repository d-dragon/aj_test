/*main app*/
#include <iostream>
#include <string>
#include "JsonParser.h"

int main(int argc, char *argv[]){

	JsonParser *tester = new JsonParser(argv[1], "src/testcases/testcase.json", "src/testcases/testitem.json");
	
	tester->startParser();
	return 1;
}

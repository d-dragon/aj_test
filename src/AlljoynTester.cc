/*main app*/
#include <iostream>
#include <string>
#include "JsonParser.h"

int main(int argc, char *argv[]){

	JsonParser *tester = new JsonParser(argv[1], argv[2], "src/testcases/testitem.json", "src/testcases/configuration.json");
	
	tester->startParser();
	return 1;
}

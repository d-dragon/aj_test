/*main app*/
#include <iostream>
#include <string>
#include "JsonParser.h"

int main(int argc, char *argv[]){

	JsonParser *tester = new JsonParser("src/testcases/testsuit.json", "src/testcases/testcase.json", "src/testcases/testitem.json");
	
	tester->startParser();
	return 1;
}

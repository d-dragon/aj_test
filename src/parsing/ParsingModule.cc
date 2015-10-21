/*
 * Simple example of parsing and printing JSON using jansson.
 *
 * SYNOPSIS:
 * $ examples/simple_parse
 * Type some JSON > [true, false, null, 1, 0.0, -0.0, "", {"name": "barney"}]
 * JSON Array of 8 elements:
 *   JSON True
 *   JSON False
 *   JSON Null
 *   JSON Integer: "1"
 *   JSON Real: 0.000000
 *   JSON Real: -0.000000
 *   JSON String: ""
 *   JSON Object of 1 pair:
 *     JSON Key: "name"
 *     JSON String: "barney"
 *
 * Copyright (c) 2014 Robert Poor <rdpoor@gmail.com>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include "ParsingModule.h"

ParsingModule::ParsingModule(){
    mJSONroot	= NULL;
}

ParsingModule::~ParsingModule(){
    if (mJSONroot != NULL)
        json_decref(mJSONroot);
}

int ParsingModule::LoadJSONFromFile(const char* path){
	json_error_t jsonErr;
	mJSONroot	= json_load_file(path, 0, &jsonErr);
	if (NULL == mJSONroot)
	{
		std::cout<<"Error while loading file: "<< jsonErr.text << " at line: " << jsonErr.line << std::endl;
		return -1;
	}
	std::cout<< "Load JSON successful"<<std::endl;
	return 0;
}

void ParsingModule::DumpJSONFile(){
	if (mJSONroot)
		std::cout << json_dumps(mJSONroot, JSON_COMPACT)<< std::endl;
	else
		std::cout << "Invalid JSON" << std::endl;
}

json_type ParsingModule::GetJSONType(json_t *element){
	if (element)
		return json_typeof(element);
}

void ParsingModule::TraversalJSONObject(json_t *element){
	size_t size;
	const char *key;
	json_t *value;		
	
	size = json_object_size(element);
    std::cout << "JSON object of: " << size << " pair(s)" << std::endl;
	json_object_foreach(element, key, value){
		std::cout << "JSON key: " << key << std::endl;
		TraversalJSONDataLocal(value);
	}
}	
void ParsingModule::TraversalJSONArray(json_t *element){
	size_t size, i;
	size = json_array_size(element);

	for (i = 0; i < size; i++){
		TraversalJSONDataLocal(json_array_get(element, i));

	}
}

void ParsingModule::TraversalJSONData(){
	TraversalJSONDataLocal(mJSONroot);
}

void ParsingModule::TraversalJSONDataLocal(json_t *element){
	switch(GetJSONType(element))
	{
		case JSON_OBJECT:
			std::cout << "JSON OBJECT" <<std::endl;
			TraversalJSONObject(element);
			break;
		case JSON_ARRAY:
			std::cout << "JSON ARRAY" << std::endl;
			TraversalJSONArray(element);
			break;
		case JSON_STRING:
			std::cout<< "Value of string: " << json_string_value(element) << std::endl;
			break;
		case JSON_INTEGER:
			std::cout<< "Value of int: " << json_integer_value(element) << std::endl;
			break;
		case JSON_REAL:
			std::cout << "Value of real: " << std::endl;
			break;
		case JSON_TRUE:
			std::cout << "Value of true: " << std::endl;
			break;
		case JSON_FALSE:
			std::cout << "Value of false: " << std::endl;
			break;
		case JSON_NULL:
			std::cout << "Value of null: " << std::endl;
			break;
		default:
			std::cout<< "Unrecognized JSON type: " << GetJSONType(element) << std::endl;
			return;
	}

}

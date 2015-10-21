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
#include <string.h>
#include "ParsingModule.h"

ParsingModule::ParsingModule(){
    mJSONcurrent = NULL;
    mJSONroot	= NULL;
	isMatch		= false;
}

ParsingModule::~ParsingModule(){
    if (mJSONroot != NULL)
        json_decref(mJSONroot);
}

int ParsingModule::LoadJSONFromFile(const char* path){
	json_error_t jsonErr;
	mJSONroot	= json_load_file(path, 0, &jsonErr);
    mJSONcurrent= mJSONroot;
	if (NULL == mJSONroot)
	{
		LOGCXX("Error while loading file: "<< jsonErr.text << " at line: " << jsonErr.line << std::endl);
		return -1;
	}
    LOGCXX("Load JSON successful"<<std::endl);

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

json_t * ParsingModule::GetObjectOfKey(const char *key){
    if (NULL != key){
        strncpy(searchKey, key, MAX_KEY_SZ);
    }
    TraversalJSONDataLocal(mJSONroot);
    if (NULL != mJSONcurrent){
        TraversalJSONDataLocal(mJSONcurrent);
    }
    return mJSONcurrent;
}

int ParsingModule::GetNumOfWifiConfiguration(){
    const char *key;
    json_t *value;
    int cnt = 0;
    void *iter = json_object_iter(mJSONroot);
//Save iter
    mJSONiter = iter;
    while(iter)
    {
        key = json_object_iter_key(iter);
        LOGCXX("Key: " << key << " count: " << cnt << std::endl);
        value = json_object_iter_value(iter);
        cnt++;
        /* use key and value ... */
        iter = json_object_iter_next(mJSONroot, iter);
    }
    
    return cnt;

}

int ParsingModule::GetNextWifiConfiguration(ajn::services::OBInfo *info){
    int ret =0;
    json_t *value;
    const char *key;
    void *iter;
    if (mJSONiter){
        value = json_object_iter_value(mJSONiter);
        // Parse data of wifi
        iter = json_object_iter(value);
        while(iter)
        {
            key = json_object_iter_key(iter);
            if (0 == strcmp(key, "ssid")) {
                info->SSID.assign( json_string_value(json_object_iter_value(iter)));
            }
            if (0 == strcmp(key,"passcode")){
                info->passcode.assign(json_string_value(json_object_iter_value(iter)));
            }
            if (0 == strcmp(key,"auth")) {
                info->authType = (ajn::services::OBAuthType)json_integer_value(json_object_iter_value(iter));
            }
            iter = json_object_iter_next(value, iter);
        }

        /* use key and value ... */
        mJSONiter = json_object_iter_next(mJSONroot, mJSONiter);
    }else {
        ret = -1;
    }
    return ret;
}

void ParsingModule::TraversalJSONObject(json_t *element){
	size_t size;
	const char *key;
	json_t *value;		
	
	size = json_object_size(element);
    std::cout << "JSON object of: " << size << " pair(s)" << std::endl;
	json_object_foreach(element, key, value){
    LOGCXX( "key: " << key );
    // Get current location of Json object has match key	
	if ((mJSONroot == mJSONcurrent) && ( 0 == strcmp(searchKey,  key)))
		{
            LOGCXX("Find out the key" << std::endl);
            mJSONcurrent = value;
            return;
		}
    // Get number of Element of Object has match key
        if ( element == mJSONcurrent) mNumberOfElement = size;

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

int ParsingModule::GetNumberOfKey(const char *key){
    mNumberOfElement = 0;
     if (NULL != key){
        strncpy(searchKey, key, MAX_KEY_SZ);
    }
    TraversalJSONDataLocal(mJSONroot);
    TraversalJSONDataLocal(mJSONcurrent);
    return mNumberOfElement;
}

void ParsingModule::TraversalJSONData(){
	TraversalJSONDataLocal(mJSONroot);
}

void ParsingModule::TraversalJSONDataLocal(json_t *element){
	switch(GetJSONType(element))
	{
		case JSON_OBJECT:
			TraversalJSONObject(element);
			break;
		case JSON_ARRAY:
			TraversalJSONArray(element);
			break;
		case JSON_STRING:
			LOGCXX( "Value of string: " << json_string_value(element) << std::endl);
			break;
		case JSON_INTEGER:
			LOGCXX( "Value of int: " << json_integer_value(element) << std::endl);
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

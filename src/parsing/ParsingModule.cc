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
#include "common_def.h"

using namespace ajn;
using namespace services;

struct OnboardingWifiCb sOnboardingCb[] ={
       {GET_VERSION, "getVersion"},
       {GET_STATE, "getState"},
       {GET_LAST_ERR, "getLastErr"},
       {GET_SCAN_INFO, "getScanInfo"},
       {CONFIG_WIFI, "configWifi"},
       {CONNECT_TO, "connectTo"},
       {CONF_AND_CONNECT_WIFI,"configAndConnectWifi"},
       {OFF_BOARD_FROM, "offBoard"}
};

struct WifiAuthenticationType sWifiAuthType[]={
    {"WPA2_AUTO",   WPA2_AUTO},                     //!< WPA2_AUTO authentication
    {"WPA_AUTO",    WPA_AUTO},                      //!< WPA_AUTO authentication
    {"ANY",         ANY},                           //!< ANY authentication
    {"OPEN",        OPEN},                          //!< OPEN authentication
    {"WEP",         WEP},                           //!< WEP authentication
    {"WPA_TKIP",    WPA_TKIP},                      //!< WPA_TKIP authentication
    {"WPA_CCMP",    WPA_CCMP},                      //!< WPA_CCMP authentication
    {"WPA2_TKIP",   WPA2_TKIP},                     //!<WPA2_TKIP authentication
    {"WPA2_CCMP",   WPA2_CCMP},                     //!<WPA2_CCMP authentication
    {"WPS",         WPS}                            //!<WPS authentication

};

ParsingModule::ParsingModule(){
    mJSONroot	= NULL;
    mJSONiter.clear();
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

int ParsingModule::GetNumOfWifiConfiguration(){
    const char *key;
    json_t *value;
    int cnt = 0;
    void *iter = json_object_iter(mJSONroot);
//Save iter
//    mJSONiter = iter;
    while(iter)
    {
        mJSONiter.push_back(iter);
        iter = json_object_iter_next(mJSONroot, iter);
    }
    
    return mJSONiter.size() - 1;

}

int ParsingModule::GetWifiConfiguration(ajn::services::OBInfo *info,long  *flgs,int pos){
    int ret =0, i;
    json_t *value= NULL;
    const char *key;
    void *iter;
    //Reset output of wifi
    info->SSID.assign("");
    info->passcode.assign("");
    info->authType  = OPEN;
    info->state     = NOT_CONFIGURED;
    //Start to get information
    if (GetKeyAtIndex(pos)){
        value = json_object_get(mJSONroot, GetKeyAtIndex(pos));
        if( value == NULL ){
            LOGCXX(GetKeyAtIndex(pos) << ": is not exist in test case" );
            return -1;
        }
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
			for (i = 0; i < sizeof(sOnboardingCb)/sizeof(OnboardingWifiCb); i++){
				if (0 == strcmp(key,sOnboardingCb[i].JSKeyName)) {
					if( json_integer_value(json_object_iter_value(iter)) == 1)
						*flgs |= sOnboardingCb[i].ID;
					}
			}
            iter = json_object_iter_next(value, iter);
        }
    }else {
        ret = -1;
    }
    return ret;
}


const char* ParsingModule::GetKeyAtIndex(int pos){
    int size, ret = 0, i;
    json_t *array;
    array = json_object_get(mJSONroot, "testlist");
    if (json_is_array(array)){
        size = json_array_size(array);
        if (pos < size && pos >= 0 )
        {
            return json_string_value(json_array_get(array,pos));
        }
    }
    LOGCXX("Invalid index= " << pos);
    return NULL;
}

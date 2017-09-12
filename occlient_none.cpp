
//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "iotivity_config.h"
#include "ocstack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#include <logger.h>
#include "ocpayload.h"
#include "payload_logging.h"

#define TAG ("occlient")
#define DEFAULT_CONTEXT_VALUE 0x99

static const char *DEVICE_DISCOVERY_QUERY = "%s/oic/d";
static const char *PLATFORM_DISCOVERY_QUERY = "%s/oic/p";
static const char *RESOURCE_DISCOVERY_QUERY = "/oic/res";


//global
static OCConnectivityType ConnType = CT_ADAPTER_IP;
static OCDevAddr serverAddr;
static char discoveryAddr[100];
static char * coapServerResource = "/a/light";

int gQuitFlag = 0;

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum) {
    if (signum == SIGINT) {
        gQuitFlag = 1;
    }
}


const char *getResult(OCStackResult result)
{
    switch (result)
    {
        case OC_STACK_OK:
            return "OC_STACK_OK";
        case OC_STACK_RESOURCE_CREATED:
            return "OC_STACK_RESOURCE_CREATED";
        case OC_STACK_RESOURCE_DELETED:
            return "OC_STACK_RESOURCE_DELETED";
        case OC_STACK_RESOURCE_CHANGED:
            return "OC_STACK_RESOURCE_CHANGED";
        case OC_STACK_INVALID_URI:
            return "OC_STACK_INVALID_URI";
        case OC_STACK_INVALID_QUERY:
            return "OC_STACK_INVALID_QUERY";
        case OC_STACK_INVALID_IP:
            return "OC_STACK_INVALID_IP";
        case OC_STACK_INVALID_PORT:
            return "OC_STACK_INVALID_PORT";
        case OC_STACK_INVALID_CALLBACK:
            return "OC_STACK_INVALID_CALLBACK";
        case OC_STACK_INVALID_METHOD:
            return "OC_STACK_INVALID_METHOD";
        case OC_STACK_NO_MEMORY:
            return "OC_STACK_NO_MEMORY";
        case OC_STACK_COMM_ERROR:
            return "OC_STACK_COMM_ERROR";
        case OC_STACK_INVALID_PARAM:
            return "OC_STACK_INVALID_PARAM";
        case OC_STACK_NOTIMPL:
            return "OC_STACK_NOTIMPL";
        case OC_STACK_NO_RESOURCE:
            return "OC_STACK_NO_RESOURCE";
        case OC_STACK_RESOURCE_ERROR:
            return "OC_STACK_RESOURCE_ERROR";
        case OC_STACK_SLOW_RESOURCE:
            return "OC_STACK_SLOW_RESOURCE";
        case OC_STACK_NO_OBSERVERS:
            return "OC_STACK_NO_OBSERVERS";
        case OC_STACK_UNAUTHORIZED_REQ:
            return "OC_STACK_UNAUTHORIZED_REQ";
        case OC_STACK_NOT_ACCEPTABLE:
            return "OC_STACK_NOT_ACCEPTABLE";
#ifdef WITH_PRESENCE
        case OC_STACK_PRESENCE_STOPPED:
            return "OC_STACK_PRESENCE_STOPPED";
        case OC_STACK_PRESENCE_TIMEOUT:
            return "OC_STACK_PRESENCE_TIMEOUT";
#endif
        case OC_STACK_ERROR:
            return "OC_STACK_ERROR";
        default:
            return "UNKNOWN";
    }
}

// This is a function called back when a device is discovered
OCStackApplicationResult getReqCB(void* ctx, OCDoHandle /*handle*/,
	OCClientResponse * clientResponse)
{
	if (clientResponse == NULL)
	{
		OIC_LOG(INFO, TAG, "getReqCB received NULL clientResponse");
		return   OC_STACK_DELETE_TRANSACTION;
	}
	if (ctx == (void*)DEFAULT_CONTEXT_VALUE)
	{
		OIC_LOG(INFO, TAG, "Callback Context for GET query recvd successfully");
	}
    
	OIC_LOG_V(INFO, TAG, "StackResult: %s", getResult(clientResponse->result));
	OIC_LOG_V(INFO, TAG, "SEQUENCE NUMBER: %d", clientResponse->sequenceNumber);
	OIC_LOG_PAYLOAD(INFO, clientResponse->payload);
	OIC_LOG(INFO, TAG, ("=============> Get Response"));

    return OC_STACK_DELETE_TRANSACTION;
    //return OC_STACK_KEEP_TRANSACTION;
}

OCStackApplicationResult discoveryReqCB(void* ctx,
                                                OCDoHandle /*handle*/,
                                                OCClientResponse * clientResponse)
{
    if (ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        OIC_LOG(INFO, TAG, "Callback Context for Platform DISCOVER query recvd successfully");
    }

    if (clientResponse)
    {
        OIC_LOG(INFO, TAG, ("Discovery Response:"));
        OIC_LOG_PAYLOAD(INFO, clientResponse->payload);

                ConnType = clientResponse->connType;
                serverAddr = clientResponse->devAddr;
    }
    else
    {
        OIC_LOG_V(INFO, TAG, "PlatformDiscoveryReqCB received Null clientResponse");
    }

    OIC_LOG(INFO, TAG, "Starting get Req");
    OCDoHandle handle;
    OCCallbackData cbDataGet;

    cbDataGet.cb = getReqCB;
    cbDataGet.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbDataGet.cd = NULL;
    /* Start a get query*/
    if (OCDoRequest(&handle, OC_REST_GET, coapServerResource, &serverAddr, 0,
            CT_DEFAULT, OC_LOW_QOS, &cbDataGet, NULL, 0) != OC_STACK_OK) {
            OIC_LOG(ERROR, TAG, "OCStack resource error");
            return OC_STACK_DELETE_TRANSACTION;
    }
    return OC_STACK_KEEP_TRANSACTION;
}

int main() {
    OIC_LOG_V(INFO, TAG, "Starting occlient");

    /* Initialize OCStack*/
    if (OCInit(NULL, 0, OC_CLIENT) != OC_STACK_OK) {
        OIC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }
    
    OIC_LOG(INFO, TAG, "Starting discovery Req");
    OCCallbackData cbData;

    cbData.cb = discoveryReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    /* Start a discovery query*/
    if (OCDoRequest(NULL, OC_REST_DISCOVER, RESOURCE_DISCOVERY_QUERY, NULL, 0,
            CT_DEFAULT, OC_LOW_QOS, &cbData, NULL, 0) != OC_STACK_OK) {
        OIC_LOG(ERROR, TAG, "OCStack resource error");
        return 0;
    }
/*
    OIC_LOG(INFO, TAG, "Starting get Req");
	OCDoHandle handle;
	OCCallbackData cbDataGet;

	cbDataGet.cb = getReqCB;
	cbDataGet.context = (void*)DEFAULT_CONTEXT_VALUE;
	cbDataGet.cd = NULL;
	// Start a get query
	if (OCDoRequest(&handle, OC_REST_GET, coapServerResource, &serverAddr, 0,
		CT_DEFAULT, OC_LOW_QOS, &cbDataGet, NULL, 0) != OC_STACK_OK) {
		OIC_LOG(ERROR, TAG, "OCStack resource error");
		return 0;
	}
*/
    // Break from loop with Ctrl+C
    OIC_LOG(INFO, TAG, "Entering occlient main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag) {

        if (OCProcess() != OC_STACK_OK) {
            OIC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }

        sleep(1);
    }

    OIC_LOG(INFO, TAG, "Exiting occlient main loop...");

    if (OCStop() != OC_STACK_OK) {
        OIC_LOG(ERROR, TAG, "OCStack stop error");
    }

    return 0;
}


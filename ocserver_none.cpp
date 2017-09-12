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
#include "ocpayload.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#include <signal.h>
#include <stdbool.h>
#include <logger.h>

#define TAG ("ocserver")

//global
char *gResourceUri = (char *)"/a/light";

int gQuitFlag = 0;
OCStackResult createLightResource();

typedef struct LIGHTRESOURCE{
    OCResourceHandle handle;
    bool power;
} LightResource;

static LightResource Light;



/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum) {
    if (signum == SIGINT) {
        gQuitFlag = 1;
    }
}

int main() {
    OIC_LOG_V(INFO, TAG, "Starting ocserver");
    if (OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK) {
        OIC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    /*
     * Declare and create the example resource: Light
     */
    if(createLightResource() != OC_STACK_OK)
    {
        OIC_LOG(ERROR, TAG, "OCStack cannot create resource...");
    }

    // Break from loop with Ctrl-C
    OIC_LOG(INFO, TAG, "Entering ocserver main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag) {

        if (OCProcess() != OC_STACK_OK) {
            OIC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }

        sleep(1);
    }

    OIC_LOG(INFO, TAG, "Exiting ocserver main loop...");

    if (OCStop() != OC_STACK_OK) {
        OIC_LOG(ERROR, TAG, "OCStack process error");
    }

    return 0;
}

OCRepPayload* getPayload(const char* uri, bool power)
{
	OCRepPayload* payload = OCRepPayloadCreate();
	if (!payload)
	{
		OIC_LOG(ERROR, TAG, PCF("Failed to allocate Payload"));
		return nullptr;
	}

	OCRepPayloadSetUri(payload, uri);
	OCRepPayloadSetPropBool(payload, "power", power);
	//OCRepPayloadSetPropInt(payload, "power", power);

	return payload;
}

OCRepPayload* constructResponse(OCEntityHandlerRequest *ehRequest)
{
	if (ehRequest->payload && ehRequest->payload->type != PAYLOAD_TYPE_REPRESENTATION)
	{
		OIC_LOG(ERROR, TAG, PCF("Incoming payload not a representation"));
		return nullptr;
	}

	OCRepPayload* input = reinterpret_cast<OCRepPayload*>(ehRequest->payload);

	LightResource *currLightResource = &Light;

	return getPayload(gResourceUri, currLightResource->power);
}

OCEntityHandlerResult ProcessGetRequest(OCEntityHandlerRequest *ehRequest,
	OCRepPayload **payload)
{
	OCEntityHandlerResult ehResult;

	OCRepPayload *getResp = constructResponse(ehRequest);
	if (!getResp)
	{
		OIC_LOG(ERROR, TAG, "constructResponse failed");
		return OC_EH_ERROR;
	}

	*payload = getResp;
	ehResult = OC_EH_OK;

	return ehResult;
}

OCEntityHandlerResult OCEntityHandlerCb(OCEntityHandlerFlag flag,
	OCEntityHandlerRequest *entityHandlerRequest, void* /*callback*/) {

	OIC_LOG_V(INFO, TAG, "Inside entity handler - flags: 0x%x", flag);

	OCEntityHandlerResult ehResult = OC_EH_OK;
	OCEntityHandlerResponse response = { 0, 0, OC_EH_ERROR, 0, 0,{},{ 0 }, false };

	// Validate pointer
	if (!entityHandlerRequest)
	{
		OIC_LOG(ERROR, TAG, "Invalid request pointer");
		return OC_EH_ERROR;
	}

	// Initialize certain response fields
	response.numSendVendorSpecificHeaderOptions = 0;
	memset(response.sendVendorSpecificHeaderOptions,
		0, sizeof response.sendVendorSpecificHeaderOptions);
	memset(response.resourceUri, 0, sizeof response.resourceUri);
	OCRepPayload* payload = nullptr;

	if (flag & OC_REQUEST_FLAG)
	{
		OIC_LOG(INFO, TAG, "Flag includes OC_REQUEST_FLAG");

		if (OC_REST_GET == entityHandlerRequest->method)
		{
			OIC_LOG(INFO, TAG, "Received OC_REST_GET from client");
			ehResult = ProcessGetRequest(entityHandlerRequest, &payload);
		}
		else
		{
			OIC_LOG_V(INFO, TAG, "Received unsupported method %d from client",
				entityHandlerRequest->method);
			ehResult = OC_EH_ERROR;
		}
	}

	if (!((ehResult == OC_EH_ERROR) || (ehResult == OC_EH_FORBIDDEN)))
	{
		// Format the response.  Note this requires some info about the request
		response.requestHandle = entityHandlerRequest->requestHandle;
		response.resourceHandle = entityHandlerRequest->resource;
		response.ehResult = ehResult;
		response.payload = reinterpret_cast<OCPayload*>(payload);
		// Indicate that response is NOT in a persistent buffer
		response.persistentBufferFlag = 0;

		// Send the response
		if (OCDoResponse(&response) != OC_STACK_OK)
		{
			OIC_LOG(ERROR, TAG, "Error sending response");
			ehResult = OC_EH_ERROR;
		}
	}

	OCPayloadDestroy(response.payload);
	return ehResult;
}


OCStackResult createLightResource() {
	Light.power = false;
	OCStackResult res = OCCreateResource(&Light.handle,
		"core.light",
		"core.rw",
		"/a/light",
		OCEntityHandlerCb,	// cb 만들어서 연결하기.
		NULL,
		OC_DISCOVERABLE);
	return res;
}
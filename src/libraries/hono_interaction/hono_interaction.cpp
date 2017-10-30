/*
 * Copyright (c) 2017 FH Dortmund and others
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Description:
 *    Hono interaction library for Rover
 *
 * Authors:
 *    M. Ozcelikors,            R.Hottger
 *    <mozcelikors@gmail.com>   <robert.hoettger@fh-dortmund.de>
 *
 * Version Summary:
 *     v1.2: Compliance changes for Hono Instance 0.5-M9
 *
 * Usage:
 * 		To test the hono interaction library:
 *
 *		registerDeviceToHonoInstance("idial.institute",8080,"DEFAULT_TENANT", 4771);
 *		sendEventDataToHonoInstance("idial.institute",8080,"DEFAULT_TENANT", 4771,"Bearing",0.5);
 *
 */

#include <libraries/hono_interaction/hono_interaction.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Buffers to work with strings
char buffer[256];
char num_buffer[33];

/**
 * Function Name:		handleCode
 * Description:			Handles code that is returned with HTTP response
 * Arguments:			int code
 * Returns:				Generates a status value
 * 						If the action completed -> returns 1
 * 						If the action failed ->    returns 0
 */
int handleCode(int code)
{
	int status = 0;
	switch (code)
	{
		case 200: //Accepted
			printf("200 Accepted\n");
			printf("Registration of device to Hono instance accepted.\n");
			status = 1;
			break;
		case 201: //Created
			printf("201 Created\n");
			printf("Registration of device to Hono instance is done.\n");
			status = 1;
			break;
		case 202:
			status = 1; //Telemetry data accepted
			break;
		case 203:
		case 204:
		case 205:
		case 206:
		case 207:
		case 208:
		case 209:
			status = 1;
			break;
		case 400:
			fprintf(stderr, ("400 Bad Request\n"));
			status = 0;
			break;
		case 403:
			fprintf(stderr, ("403 Forbidden\n"));
			status = 0;
			break;
		case 409:
			fprintf(stderr, ("409 Conflict. A device with this ID is registered already. \n"));
			status = 0;
			break;
		case 503:
			fprintf(stderr, ("503 Service Unavailable. You may need to create a consumer."));
			status = 0;
			break;
		case 401:
		case 402:
		case 404:
		case 405:
		case 406:
		case 407:
		case 408:
			fprintf(stderr, ("Error in Hono connection. \n"));
			status = 0;
			break;
		default:
			status = 0;
			break;
	}
	return status;
}

/**
 * Function Name:		registerDeviceToHonoInstance
 * Description:			Registers a device to Eclipse Hono instance.
 * 						A device should be registered only once.
 * Arguments:			char * host_name
 * 						int port
 * 						char * tenant_name
 * 						char * device_id
 * Returns:				Successfully returns status data.
 * 						If the action completed -> returns 1
 * 						If the action failed ->    returns 0
 */
int registerDeviceToHonoInstance (char * host_name, int port, char * tenant_name, char * device_id)
{
	FILE *fp;
	int code;
	int status;

	//Prepare command as string
	//Example: "curl -X POST -i -H 'Content-Type: application/json' -d '{"device-id": "4711"}' http://idial.institute:28080/registration/DEFAULT_TENANT"
	sprintf(buffer, "curl -X POST -i -H 'Content-Type: application/json' -d '{\"device-id\":\"");
	strcat(buffer, device_id);
	strcat(buffer, "\"}' http://");
	strcat(buffer, host_name);
	strcat(buffer, ":");
	snprintf(num_buffer, sizeof(num_buffer), "%d", port);
	strcat(buffer, num_buffer);
	num_buffer[0] = 0; //Clear array
	strcat(buffer, "/registration/");
	strcat(buffer, tenant_name);

	//To redirect pipe to prevent stdout showing all outputs generated by curl
	strcat(buffer, " 2>/dev/null"); //2>&1 would redirect to stderr, we choose to be able to parse returned code

#ifdef DEBUG_DEVICE_REGISTRATION
	//Print the command that is created
	printf("Command=%s\n",buffer);
#endif

	//Execute the command
	fp = popen(buffer,"r");

	//Get and Parse the output
	fgets(buffer, 13, fp); //Get the string HTTP/1.1 XXX

	//Prepare the response code
	sscanf(buffer, "HTTP/1.1 %d", &code);

#ifdef DEBUG_DEVICE_REGISTRATION
	//Print the code
	printf("Code=%d\n",code);

	//Print the response
	printf("Response=%s\n",buffer);
#endif

	//Handle the code and return status: 1-succeeded 0-failed
	status = handleCode(code);

	//Close the file
	fclose(fp);

	//Return status
	return status;
}


/**
 * Function Name:		sendTelemetryDataToHonoInstance
 * Description:			Sends telemetry data to a hono instance with given
 * 						host name and port
 * Arguments:			char * host_name
 * 						int port
 * 						char * tenant_name
 * 						char * device_id
 * 						char * field
 * 						double value
 * Returns:				Successfully returns status data.
 * 						If the action completed -> returns 1
 * 						If the action failed ->    returns 0
 */
int sendTelemetryDataToHonoInstance (char * host_name, int port, char * tenant_name, char * device_id, char * field, double value)
{
	FILE *fp;
	int code;
	int status;

	//Prepare command as string
	//Example: "curl -X PUT -i -H 'Content-Type: application/json' --data-binary '{"Bearing": 0.5}' http://idial.institute:8080/telemetry/DEFAULT_TENANT/4711"

	//For Hono 0.5-M9
	//Example: "curl -X POST -i -u sensor1@DEFAULT_TENANT:hono-secret -H 'Content-Type: application/json' --data-binary '{"temp": 5}' http://idial.institute:8080/telemetry"

	//To get the information in dashboard, we use device ID as the entry name, and "value" as field.
	//Example: "curl -X PUT -i -H 'Content-Type: application/json' --data-binary '{"value": 0.5}' http://idial.institute:8080/telemetry/DEFAULT_TENANT/roverRearSensor"

	sprintf(buffer, "curl -X POST -i -u sensor1@");
	strcat(buffer, tenant_name);
	strcat(buffer, ":hono-secret -H 'Content-Type: application/json' --data-binary '{\"");

	strcat(buffer, field);
	strcat(buffer, "\": ");
	snprintf(num_buffer, sizeof(num_buffer), "%f", value);
	strcat(buffer, num_buffer);
	num_buffer[0] = 0; //Clear array

	strcat(buffer, "}' http://");
	strcat(buffer, host_name);
	strcat(buffer, ":");
	snprintf(num_buffer, sizeof(num_buffer), "%d", port);
	strcat(buffer, num_buffer);
	num_buffer[0] = 0; //Clear array
	strcat(buffer, "/telemetry");

	//To redirect pipe to prevent stdout showing all outputs generated by curl
	strcat(buffer, " 2>/dev/null"); //2>&1 would redirect to stderr, we choose to be able to parse returned code

#ifdef DEBUG_HTTP_RESPONSE
	//Print the command that is created
	printf("Command=%s\n",buffer);
#endif

	//Execute the command
	fp = popen(buffer,"r");

	//Get and Parse the output
	fgets(buffer, 13, fp); //Get the string HTTP/1.1 XXX

	//Prepare the response code
	sscanf(buffer, "HTTP/1.1 %d", &code);

#ifdef DEBUG_HTTP_RESPONSE
	//Print the code
	printf("Code=%d\n",code);

	//Print the response
	printf("Response=%s\n",buffer);
#endif

	//Handle the code and return status: 1-succeeded 0-failed
	status = handleCode(code);

	//Close the file
	fclose(fp);

	//Return status
	return status;
}


/**
 * Function Name:		sendEventDataToHonoInstance
 * Description:			Sends event data to a hono instance with given
 * 						host name and port
 * Arguments:			char * host_name
 * 						int port
 * 						char * tenant_name
 * 						char * device_id
 * 						char * field
 * 						double value
 * Returns:				Successfully returns status data.
 * 						If the action completed -> returns 1
 * 						If the action failed ->    returns 0
 */
int sendEventDataToHonoInstance (char * host_name, int port, char * tenant_name, char * device_id, char * field, double value)
{
	FILE *fp;
	int code;
	int status;

	//Prepare command as string
	//Example: "curl -X PUT -i -H 'Content-Type: application/json' --data-binary '{"Bearing": 0.5}' http://idial.institute:8080/event/DEFAULT_TENANT/4711"

	//For Hono 0.5-M9
	//Example: "curl -X POST -i -u sensor1@DEFAULT_TENANT:hono-secret -H 'Content-Type: application/json' --data-binary '{"temp": 5}' http://idial.institute:8080/event"

	//To get the information in dashboard, we use device ID as the entry name, and "value" as field.
	//Example: "curl -X PUT -i -H 'Content-Type: application/json' --data-binary '{"value": 0.5}' http://idial.institute:8080/event/DEFAULT_TENANT/roverRearSensor"

	sprintf(buffer, "curl -X POST -i -u sensor1@");
	strcat(buffer, tenant_name);
	strcat(buffer, ":hono-secret -H 'Content-Type: application/json' --data-binary '{\"");

	strcat(buffer, field);
	strcat(buffer, "\": ");
	snprintf(num_buffer, sizeof(num_buffer), "%f", value);
	strcat(buffer, num_buffer);
	num_buffer[0] = 0; //Clear array

	strcat(buffer, "}' http://");
	strcat(buffer, host_name);
	strcat(buffer, ":");
	snprintf(num_buffer, sizeof(num_buffer), "%d", port);
	strcat(buffer, num_buffer);
	num_buffer[0] = 0; //Clear array
	strcat(buffer, "/event");

	//To redirect pipe to prevent stdout showing all outputs generated by curl
	strcat(buffer, " 2>/dev/null"); //2>&1 would redirect to stderr, we choose to be able to parse returned code

#ifdef DEBUG_HTTP_RESPONSE
	//Print the command that is created
	printf("Command=%s\n",buffer);
#endif

	//Execute the command
	fp = popen(buffer,"r");

	//Get and Parse the output
	fgets(buffer, 13, fp); //Get the string HTTP/1.1 XXX

	//Prepare the response code XXX
	sscanf(buffer, "HTTP/1.1 %d", &code);

#ifdef DEBUG_HTTP_RESPONSE
	//Print the code
	printf("Code=%d\n",code);

	//Print the response
	printf("Response=%s\n",buffer);
#endif

	//Handle the code and return status: 1-succeeded 0-failed
	status = handleCode(code);

	//Close the file
	fclose(fp);

	//Return status
	return status;
}

/**
 * Function Name:		registerSensorsToHonoInstance
 * Description:			If non registered already, this function
 * 						registers all the sensors and other entries
 * 						of APP4MC Rover as devices to Hono instance.
 * 						for visualization of Raw data in Granafa/InfluxDB.
 */
int registerEntriesToHonoInstance (void)
{
	/*registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "roverFront");
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "roverFrontLeft");
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "roverFrontRight");
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "roverRear");
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "roverRearLeft");
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "roverRearRight");
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "roverBearing");
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "roverUtilCpu1");
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "roverUtilCpu2");
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "roverUtilCpu3");
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "roverUtilCpu4");
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "connectionCheck");*/
	registerDeviceToHonoInstance("idial.institute",28080,"DEFAULT_TENANT", "4711");

	return 1;
}


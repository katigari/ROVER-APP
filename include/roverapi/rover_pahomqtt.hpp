/*
 * Copyright (c) 2017 FH Dortmund and others
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Description:
 *    Rover PahoMQTT API - Interfaces for Rover PahoMQTT cloud application development - Header file
 *
 * Contributors:
 *    M.Ozcelikors <mozcelikors@gmail.com>, created PahoMQTT API 07.12.2017
 *                                          improved PahoMQTT API 10.12.2017
 *
 */

#ifndef ROVERAPI_ROVER_PAHOMQTT_HPP_
#define ROVERAPI_ROVER_PAHOMQTT_HPP_

#include <roverapi/rover_cloud.hpp>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "MQTTClient.h"
#include "MQTTAsync.h"

using namespace std;

namespace rover
{

	/**
	 * @brief Data type to store initial configuration information for MQTT communication.
	 */
	typedef struct
	{
		char * clientID;
		char * topic;
		char * payload;
		char * username;
		char * password;
		int			qos;		// 1
		long int	timeout;	// Such as 10000L usec
	} RoverMQTT_Configure_t;

	/**
	 * @brief Data type to store subscribed data
	 */
	typedef struct
	{
		int data_ready;
		char * data;
	} RoverMQTT_SubscribeData_t;

	/**
	 * @brief Data type to store internal flags for MQTT communication
	 */
	typedef struct
	{
		int f_mqtt_disconnect_finished;
		int f_mqtt_finished;
		int f_mqtt_subscribed;
		int f_mqtt_publish_successful;
                int f_mqtt_connected;
	} RoverMQTT_StatusFlags_t;

	/**
	 * @brief Data type to store return codes that belong to member functions in RoverPahoMQTT class.
	 */
	typedef struct
	{
		int rc_publish;
		int rc_subscribe;
		int rc_unsubscribe;
	} RoverMQTT_ReturnCodes_t;

	/**
	 * @brief RoverPahoMQTT contains member functions to use rover as a client and to publish / subscribe to Eclipse Paho MQTT server topics.
	 *
         *	\warning For oncreate examples please go to: https://github.com/app4mc-rover/rover-app/tree/master/src/examples
	 */
	class RoverPahoMQTT : public RoverCloud
	{
		protected:
			/**
			 * @brief Host name used for connecting to the Eclipse Paho MQTT server
			 */
			char * HOST_NAME;

			/**
			 * @brief Port used for connecting to the Eclipse Paho MQTT server
			 */
			int PORT;

			/**
			 * @brief Member attribute for subscribed data
			 */
			RoverMQTT_SubscribeData_t defaultRoverSubscribeData;

			/**
			 * @brief Member attribute for configuration data
			 */
			RoverMQTT_Configure_t defaultRoverMQTTConfigure;

			/**
			 * @brief Member attribute for flags
			 */
			RoverMQTT_StatusFlags_t defaultRoverMQTTFlags;

			/**
			 * @brief Member attribute for return codes
			 */
			RoverMQTT_ReturnCodes_t defaultReturnCodes;

			/**
			 * @brief Client object from MQTTAsync.
			 */
                        MQTTAsync client = NULL;

			/**
			 * @brief Client object from MQTTClient.
			 */
			MQTTClient client2;

			/**
			 * @brief Connect options
			 */
			MQTTAsync_connectOptions conn_opts;

			/**
			 * @brief Disconnect options
			 */
			MQTTAsync_disconnectOptions disc_opts;

			/**
			 * @brief Delivered token
			 */
			volatile MQTTAsync_token deliveredtoken;

			/**
			 * @brief Create the MQTT client object
			 */
			void createClient(void);
			
			/**
			 * @brief Destroy the MQTT client object
			 */
			void destroyClient(void);


                public:
			/**
			 * @brief Destructor for RoverPahoMQTT class.
			 */
			~RoverPahoMQTT ();

			/**
			 * @brief Default constructor for RoverPahoMQTT class
			 */
			RoverPahoMQTT();


			/**
			 * @brief Copy constructor for RoverPahoMQTT
			 */
			RoverPahoMQTT (char * host_name, int port, RoverMQTT_Configure_t MQTT_Configure);

			/**
			 * @brief Sets private attribute HOST_NAME
			 * @param host_name
			 */
			void setHostName (char * host_name);

			/**
			 * @brief Sets private attribute PORT
			 * @param port Port
			 */
			void setPort (const int port);

			/**
			 * @brief Sets payload
			 * @param payload Payload
			 * @param payloadLen Payload length
			 */
			void setPayload (const char * payload, int payloadLen);

			/**
			 * @brief Sets topic name
			 * @param topic Topic name
			 */
			void setTopic (char * topic);

			/**
			 * @brief Public member function for reading subscribed data. This function requires the RoverPahoMQTT to be subscribed first.
			 * @param data char* Data pointer that will be modified.
			 * @return status: 0-> Successful read,  1-> No data available
			 */
			int read (char * data);

			/**
			 * @brief Public member function checking if the subscribed data is ready. 0-> data ready -1-> data not ready
			 */
			int isDataReady (void);

			/**
			 * @brief Used for publishing to a topic in an MQTT-broker using asynchronous method
			 * @return Return code 0-> success; others-> fail return codes \n
			 * 	**1**: Connection refused: Unacceptable protocol version \n
			 *	**2**: Connection refused: Identifier rejected \n
			 *	**3**: Connection refused: Server unavailable \n
			 *	**4**: Connection refused: Bad user name or password \n
			 *	**5**: Connection refused: Not authorized \n
			 *	**6-255**: Reserved for future use \n
			 *	Reference: \cite paho.mqtt.c Asynchronous MQTT Client Documentation
			 */
			int publish (void);

                        /**
                         * @brief Used for connecting to MQTT broker asynchronously
                         * @return Return code 0-> success; others-> fail return codes \n
                         * 	**1**: Connection refused: Unacceptable protocol version \n
                         *	**2**: Connection refused: Identifier rejected \n
                         *	**3**: Connection refused: Server unavailable \n
                         *	**4**: Connection refused: Bad user name or password \n
                         *	**5**: Connection refused: Not authorized \n
                         *	**6-255**: Reserved for future use \n
                         *	Reference: \cite paho.mqtt.c Asynchronous MQTT Client Documentation
                         */
                        int connect (void);

			/**
			 * @brief Used for subscribing to a topic in an MQTT-broker. To unsubscribe use: RoverPahoMQTT::unsubscribe()
			 * @return Return code 0-> success; others-> fail return codes
			 *  **1**: Connection refused: Unacceptable protocol version \n
			 *	**2**: Connection refused: Identifier rejected \n
			 *	**3**: Connection refused: Server unavailable \n
			 *	**4**: Connection refused: Bad user name or password \n
			 *	**5**: Connection refused: Not authorized \n
			 *	**6-255**: Reserved for future use \n
			 *	Reference: \cite paho.mqtt.c Asynchronous MQTT Client Documentation
			 */
			int subscribe (void);

			/**
			 * @brief Used for unsubscribing and disconnecting from a subscription to a topic in an MQTT-broker.
			 * @return Return code 0-> success; others-> fail sreturn codes
			 *  **1**: Connection refused: Unacceptable protocol version \n
			 *	**2**: Connection refused: Identifier rejected \n
			 *	**3**: Connection refused: Server unavailable \n
			 *	**4**: Connection refused: Bad user name or password \n
			 *	**5**: Connection refused: Not authorized \n
			 *	**6-255**: Reserved for future use \n
			 *	Reference: \cite paho.mqtt.c Asynchronous MQTT Client Documentation
			 */
			int unsubscribe (void);

                        /**
                         * @brief Returns connected flag
                         * @return connected flag 1:connected 0: not connected
                         */
                        int getConnected(void);

		protected:
			/**
			 * @brief Private member function to flush the connection flags for new operation.
			 * @return void
			 */
			void flushFlags (void);

			/**
			 * @brief Used for constructing the connection address by writing to private attribute my_address
			 * @return void
			 */
			void constructAddress (void);

			/* Internal callbacks */
			/* Related to publisher */
			/**
			 * @brief Static function responsible for calling RoverPahoMQTT::onPublisherConnect in a given RoverPahoMQTT context.
			 * @param context Given context for RoverPahoMQTT. Internal use: this
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
                        static void onConnect_Redirect (void* context, MQTTAsync_successData* response);

			/**
			 * @brief Static function responsible for calling RoverPahoMQTT::onPublisherSend in a given RoverPahoMQTT context.
			 * @param context Given context for RoverPahoMQTT. Internal use: this
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
			static void onPublisherSend_Redirect (void* context, MQTTAsync_successData* response);

			/**
			 * @brief Member callback function for onPublisherConnect actions
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
                        void onConnect (MQTTAsync_successData* response);


			/**
			 * @brief Member callback function for onPublisherSend actions
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
			void onPublisherSend (MQTTAsync_successData* response);


			/* Related to subscriber */
			/**
			 * @brief Static function responsible for calling RoverPahoMQTT::onSubscriberMessageArrived in a given RoverPahoMQTT context.
			 * @param context Given context for RoverPahoMQTT. Internal use: this
			 * @param topicName received topic name
			 * @param topicLen received topic name length
			 * @param message received message
			 * @return int
			 */
			static int onSubscriberMessageArrived_Redirect (void *context, char *topicName, int topicLen, MQTTAsync_message *message);

			/**
			 * @brief Static function responsible for calling RoverPahoMQTT::onSubscribe in a given RoverPahoMQTT context.
			 * @param context Given context for RoverPahoMQTT. Internal use: this
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
			static void onSubscribe_Redirect (void* context, MQTTAsync_successData* response);

			/**
			 * @brief Static function responsible for calling RoverPahoMQTT::onSubscribeFailure in a given RoverPahoMQTT context.
			 * @param context Given context for RoverPahoMQTT. Internal use: this
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
			static void onSubscribeFailure_Redirect (void* context, MQTTAsync_failureData* response);


			/**
			 * @brief Member callback function for onSubscriberMessageArrived actions
			 * @param topicName received topic name
			 * @param topicLen received topic name length
			 * @param message received message
			 * @return int
			 */
			int onSubscriberMessageArrived (char *topicName, int topicLen, MQTTAsync_message *message);

			/**
			 * @brief Member callback function for onSubscribe actions
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
			void onSubscribe (MQTTAsync_successData* response);

			/**
			 * @brief Member callback function for onSubscribeFailure actions
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
			void onSubscribeFailure (MQTTAsync_failureData* response);

			/* Related to both */
			/**
			 * @brief Static function responsible for calling RoverPahoMQTT::onConnectFailure in a given RoverPahoMQTT context.
			 * @param context Given context for RoverPahoMQTT. Internal use: this
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
			static void onConnectFailure_Redirect (void* context, MQTTAsync_failureData* response);

			/**
			 * @brief Static function responsible for calling RoverPahoMQTT::onConnectionLost in a given RoverPahoMQTT context.
			 * @param context Given context for RoverPahoMQTT. Internal use: this
			 * @param cause Cause of connection loss
			 * @return void
			 */
			static void onConnectionLost_Redirect (void* context, char *cause);

			/**
			 * @brief Member callback function for onConnectFailure actions
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
			void onConnectFailure (MQTTAsync_failureData* response);

			/**
			 * @brief Static function responsible for calling RoverPahoMQTT::onDisconnect in a given RoverPahoMQTT context.
			 * @param context Given context for RoverPahoMQTT. Internal use: this
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
			static void onDisconnect_Redirect (void* context, MQTTAsync_successData* response);

			/**
			 * @brief Member callback function for onConnectionLost actions
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
			void onConnectionLost (char *cause);

			/**
			 * @brief Member callback function for onDisconnect actions
			 * @param response Response data from MQTTAsync instance
			 * @return void
			 */
			void onDisconnect (MQTTAsync_successData* response);

	private:
			/**
			 * @brief Payload pointer to manage memory
			 */
			char * payload = {};
			
			/**
			 * @brief MQTT address
			 */
                        char my_address[100] = {};

                        int connected = 0;

	};
}



#endif /* ROVERAPI_ROVER_PAHOMQTT_HPP_ */

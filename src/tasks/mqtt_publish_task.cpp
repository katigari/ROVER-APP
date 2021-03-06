/*
 * Copyright (c) 2017 FH Dortmund and others
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Description:
 *    MQTT Publish Task
 *
 * Contributors:
 *    M.Ozcelikors <mozcelikors@gmail.com>, created MQTT Publish task 11.12.2017
 *
 */

#include <tasks/mqtt_publish_task.h>

#include <unistd.h>
#include <ctime>
#include <libraries/timing/timing.h>
#include <interfaces.h>
#include <pthread.h>

#include <roverapp.h>
#include <roverapi/rover_pahomqtt.hpp>
#include <roverapi/rover_mqttcommand.hpp>

#include <wiringPi.h>

void *MQTT_Publish_Task (void * arg)
{
	timing mqtt_publish_task_tmr;
	int rc = 1;

	mqtt_publish_task_tmr.setTaskID((char*)"MQTTPublish");
	mqtt_publish_task_tmr.setDeadline(0.1);
	mqtt_publish_task_tmr.setPeriod(0.1);

	// The following is now initialized only once in main
//	RoverMQTTCommand rover_mqtt = RoverMQTTCommand ( rover_config_obj.MQTT_BROKER_C,
//													 rover_config_obj.MQTT_BROKER_PORT_C,
//													 rover_config_obj.ROVER_IDENTITY_C,
//													 rover_config_obj.ROVER_MQTT_QOS_C,
//													 rover_config_obj.MQTT_USERNAME_C,
//													 rover_config_obj.MQTT_PASSWORD_C,
//													 "rover_mqtt_publisher");

	RoverSensorData_t sensor_data;
	float core_usages[4] = {};

	printf ("Trying to connect to MQTT Broker... \n");
	if (rover_mqtt->getRoverConnected() != 1)
	{
		rc = rover_mqtt->connectRover();
	}
	else
	{
		printf ("Connected.....\n");
		rc = 0;
	}

	while (running_flag.get())
	{
		mqtt_publish_task_tmr.recordStartTime();
		mqtt_publish_task_tmr.calculatePreviousSlackTime();

		//Task content starts here -----------------------------------------------

		if (rc != 0)
		{
			printf ("Publisher did not connect\n");
			rc = rover_mqtt->connectRover();
			continue;
		}

		sensor_data.ultrasonic_front = distance_sr04_front_shared.get();
		sensor_data.ultrasonic_rear = distance_sr04_back_shared.get();
		sensor_data.hmc5883l_bearing = bearing_shared.get();
		sensor_data.infrared[0] = infrared_shared.get(0);
		sensor_data.infrared[1] = infrared_shared.get(1);
		sensor_data.infrared[2] = infrared_shared.get(2);
		sensor_data.infrared[3] = infrared_shared.get(3);
		sensor_data.gy521_accel_x = accelerometerdata_shared.accel_x;
		sensor_data.gy521_accel_y = accelerometerdata_shared.accel_y;
		sensor_data.gy521_accel_z = accelerometerdata_shared.accel_z;
		sensor_data.gy521_gyro_x = accelerometerdata_shared.gyro_x;
		sensor_data.gy521_gyro_y = accelerometerdata_shared.gyro_y;
		sensor_data.gy521_gyro_z = accelerometerdata_shared.gyro_z;
		sensor_data.gy521_angle_x = accelerometerdata_shared.angle_x;
		sensor_data.gy521_angle_y = accelerometerdata_shared.angle_y;
		sensor_data.gy521_angle_z = accelerometerdata_shared.angle_z;
		sensor_data.core[0] = cpu_util_shared.get(0);
		sensor_data.core[1] = cpu_util_shared.get(1);
		sensor_data.core[2] = cpu_util_shared.get(2);
		sensor_data.core[3] = cpu_util_shared.get(3);

		// Publishing with redirected topics
		//   Rover --->telemetry---> EclipseHono ---> rover/<roverid>/telemetry ---> Rover Telemetry UI
		//
		// Publishing with non- redirected topics
		//   Rover --> rover/<roverid>/telemetry --> EclipseHono --> rover/<roverid>/telemetry --> Rover Telemetry UI
		//
		if (rover_config_obj.USE_REDIRECTED_TOPICS_C == 1)
		{
			pthread_mutex_lock(&mqtt_client_lock);
				if (rover_mqtt->publishToTelemetryTopic(sensor_data) == 0)
					printf ("Client rover_mqtt_publisher: Redirected Publishing successful!\n");
				else
					printf ("Client rover_mqtt_publisher: Redirected Publishing unsuccessful!\n");
			pthread_mutex_unlock(&mqtt_client_lock);
		}
		else // 0
		{
			pthread_mutex_lock(&mqtt_client_lock);
				if (rover_mqtt->publishToTelemetryTopicNonRedirected(sensor_data) == 0)
					printf ("Client rover_mqtt_publisher: Nonredirected Publishing successful!\n");
				else
					printf ("Client rover_mqtt_publisher: Nonredirected Publishing unsuccessful!\n");
			pthread_mutex_unlock(&mqtt_client_lock);
		}


		//Task content ends here -------------------------------------------------

		mqtt_publish_task_tmr.recordEndTime();
		mqtt_publish_task_tmr.calculateExecutionTime();
		mqtt_publish_task_tmr.calculateDeadlineMissPercentage();
		mqtt_publish_task_tmr.incrementTotalCycles();
		pthread_mutex_lock(&mqtt_publish_task_ti.mutex);
			mqtt_publish_task_ti.deadline = mqtt_publish_task_tmr.getDeadline();
			mqtt_publish_task_ti.deadline_miss_percentage = mqtt_publish_task_tmr.getDeadlineMissPercentage();
			mqtt_publish_task_ti.execution_time = mqtt_publish_task_tmr.getExecutionTime();
			mqtt_publish_task_ti.period = mqtt_publish_task_tmr.getPeriod();
			mqtt_publish_task_ti.prev_slack_time = mqtt_publish_task_tmr.getPrevSlackTime();
			mqtt_publish_task_ti.task_id = mqtt_publish_task_tmr.getTaskID();
			mqtt_publish_task_ti.start_time = mqtt_publish_task_tmr.getStartTime();
			mqtt_publish_task_ti.end_time = mqtt_publish_task_tmr.getEndTime();
		pthread_mutex_unlock(&mqtt_publish_task_ti.mutex);
		mqtt_publish_task_tmr.sleepToMatchPeriod();
	}

	/* the function must return something - NULL will do */
	return NULL;
}

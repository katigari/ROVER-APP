/*
 * Copyright (c) 2018 Eclipse Foundation, FH Dortmund and others
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Description:
 *    Configuration file reader for the rover
 *    	Header file
 *    Configuration file will be placed to /opt/rover-app/config/rover_config.txt in the target. Please modify this file in case you want to change the configuration.
 *
 * Author:
 *    M. Ozcelikors,  <mozcelikors@gmail.com> - created 18.01.2018
 */

#ifndef LIBRARIES_CONFIG_READER_CONFIG_READER_H_
#define LIBRARIES_CONFIG_READER_CONFIG_READER_H_


#define ROVER_CONFIG_MAXBUF 1024
#define DELIM "="
#define ROVER_CONFIG_FILE "/opt/rover-app/config/rover_config.txt"

typedef struct rover_config
{
	int ROVER_IDENTITY_C;
	char MQTT_BROKER_C [ROVER_CONFIG_MAXBUF];
	int MQTT_BROKER_PORT_C;
	int ROVER_MQTT_QOS_C;
	int USE_GROOVE_SENSOR_C;
};

rover_config getRoverConfig (char *filename);

#endif /* LIBRARIES_CONFIG_READER_CONFIG_READER_H_ */

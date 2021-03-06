/*
 * Copyright (c) 2017 Fraunhofer IEM, Eclipse Foundation and others
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Description:
 *    HMC588L compass reading and calibration task (x,y plane) with with wiringPi and pThreads
 *
 * Authors:
 *     David Schmelter
 *     <david.schmelter@iem.fraunhofer.de>
 *
 * Contributors:
 * 		Mustafa Ozcelikors <mozcelikors@gmail.com> Calibration via web-page, changes regarding schedulability
 *
 * Update History:
 *    22.03.2017 - initial revision
 *
 */

#include <tasks/compass_sensor_task.h>
#include <stdint.h>
#include <ctime>
#include <unistd.h>
#include <libraries/timing/timing.h>
#include <interfaces.h>
#include <pthread.h>

#include <roverapp.h>

#include <roverapi/rover_hmc5883l.hpp>


int EndCalibrationMode (void)
{
	keycommand_shared = 'f';
	return 1;
}

void *CompassSensor_Task(void * arg)
{
	timing compass_task_tmr;
	char local_command = 'f';

	compass_task_tmr.setTaskID((char*)"Compass-Sensor");
	compass_task_tmr.setDeadline(0.1);
	compass_task_tmr.setPeriod(0.1);

	RoverHMC5883L r_hmc5883l = RoverHMC5883L();
	r_hmc5883l.initialize();
	pthread_mutex_lock(&i2c_lock);
		r_hmc5883l.calibrate();
	pthread_mutex_unlock(&i2c_lock);

	while (running_flag.get()) {
		compass_task_tmr.recordStartTime();
		compass_task_tmr.calculatePreviousSlackTime();

		//Task content starts here -----------------------------------------------
		local_command = keycommand_shared.get();

		//Calibration routine
		if (local_command == 'u')
		{
			printf("Starting compass calibration for 5 seconds. Please rotate me 360 degrees.\n");
			EndCalibrationMode();
			//r.inRoverSensors().calibrateBearingSensor();
			//Calibration state..
			//At the end of calibration to go to the end of calibration mode call EndCalibrationMode()
		}
		//Asynchronous end to calibration mode --> Call EndCalibrationMode()

		pthread_mutex_lock(&i2c_lock);
			bearing_shared = r_hmc5883l.read();
		pthread_mutex_unlock(&i2c_lock);

		//Task content ends here -------------------------------------------------
		compass_task_tmr.recordEndTime();
		compass_task_tmr.calculateExecutionTime();
		compass_task_tmr.calculateDeadlineMissPercentage();
		compass_task_tmr.incrementTotalCycles();
		pthread_mutex_lock(&compass_task_ti.mutex);
			compass_task_ti.deadline = compass_task_tmr.getDeadline();
			compass_task_ti.deadline_miss_percentage = compass_task_tmr.getDeadlineMissPercentage();
			compass_task_ti.execution_time = compass_task_tmr.getExecutionTime();
			compass_task_ti.period = compass_task_tmr.getPeriod();
			compass_task_ti.prev_slack_time = compass_task_tmr.getPrevSlackTime();
			compass_task_ti.task_id = compass_task_tmr.getTaskID();
			compass_task_ti.start_time = compass_task_tmr.getStartTime();
			compass_task_ti.end_time = compass_task_tmr.getEndTime();
		pthread_mutex_unlock(&compass_task_ti.mutex);
		compass_task_tmr.sleepToMatchPeriod();
	}

	/* the function must return something - NULL will do */
	return NULL;
}

/*
 * Copyright (c) 2021-2024 LAAS-CNRS
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGLPV2.1
 */

/**
 * @brief  This file it the main entry point of the
 *         OwnTech Power API. Please check the OwnTech
 *         documentation for detailed information on
 *         how to use Power API: https://docs.owntech.org/
 *
 * @author Clément Foucher <clement.foucher@laas.fr>
 * @author Luiz Villa <luiz.villa@laas.fr>
 * @author Ayoub Farah Hassan <ayoub.farah-hassan@laas.fr>
 */


//--------------OWNTECH APIs----------------------------------
#include "DataAPI.h"
#include "TaskAPI.h"
#include "TwistAPI.h"
#include "SpinAPI.h"
#include "CommunicationAPI.h"

#include <math.h>

#define SAMPLE_RATE 10000.0 // Frequency of ctrl task in Hz
#define SIGNAL_FREQ   50.0    // Frequency of the sinewave in Hz
#define AMPLITUDE 0.5       // Amplitude of the sinewave
#define SERVER
#define RECORD_SIZE 2047

//--------------SETUP FUNCTIONS DECLARATION-------------------
void setup_routine(); // Setups the hardware and software of the system

//--------------LOOP FUNCTIONS DECLARATION--------------------
void loop_background_task();   // Code to be executed in the background task
void loop_critical_task();     // Code to be executed in real time in the critical task

//--------------USER VARIABLES DECLARATIONS-------------------

#ifdef SERVER
float32_t sine_ref;
#endif

#ifdef CLIENT
float32_t sine_ref_analog;
#endif

const uint16_t num_samples = (int)(SAMPLE_RATE / SIGNAL_FREQ); // Number of sampling per sinewave period
uint16_t counter_sinus = 0;
static float32_t t;

static uint32_t counter;

#endif

//--------------SETUP FUNCTIONS-------------------------------

/**
 * This is the setup routine.
 * It is used to call functions that will initialize your spin, twist, data and/or tasks.
 * In this example, we setup the version of the spin board and a background task.
 * The critical task is defined but not started.
 */
void setup_routine()
{
    // Setup the hardware first
    spin.version.setBoardVersion(SPIN_v_1_0);

    data.enableTwistDefaultChannels();

    twist.setVersion(shield_TWIST_V1_2);
    twist.initAllBuck();

    communication.analog.init();

    #ifdef SERVER
    communication.analog.setAnalogCommValue(2000);
    #endif

    #ifdef CLIENT
    communication.sync.initSlave(); // start the synchronisation
    #endif

    // Then declare tasks
    uint32_t background_task_number = task.createBackground(loop_background_task);
    task.createCritical(loop_critical_task, 100); // Uncomment if you use the critical task

    // Finally, start tasks
    task.startBackground(background_task_number);
    task.startCritical(); // Uncomment if you use the critical task
}

//--------------LOOP FUNCTIONS--------------------------------

/** * This is the code loop of the background task
 * It is executed second as defined by it suspend task in its last line.
 * You can use it to execute slow code such as state-machines.
 */
void loop_background_task()
{
    #ifdef SERVER
    printk("%f\n", sine_ref);
    #endif

    #ifdef CLIENT
    printk("%.2f\n", sine_ref_analog);
    #endif

    // Pause between two runs of the task
    task.suspendBackgroundMs(100);
}

/**
 * This is the code loop of the critical task
 * It is executed every 500 micro-seconds defined in the setup_software function.
 * You can use it to execute an ultra-fast code with the highest priority which cannot be interruped.
 * It is from it that you will control your power flow.
 */
void loop_critical_task()
{
    #ifdef SERVER

    t = counter_sinus / (SAMPLE_RATE);
    sine_ref = AMPLITUDE * sin(2 * PI * SIGNAL_FREQ * t) + 0.5; // Calcul de l'échantillon
    counter_sinus++;
    if(counter_sinus > num_samples) counter_sinus = 0;

    communication.analog.setAnalogCommValue(sine_ref*4000);

    #endif

    #ifdef CLIENT
    sine_ref_analog = communication.analog.getAnalogCommValue()/4000;
    #endif
}

/**
 * This is the main function of this example
 * This function is generic and does not need editing.
 */
int main(void)
{
    setup_routine();

    return 0;
}

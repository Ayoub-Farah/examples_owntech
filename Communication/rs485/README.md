# Send information with rs485 communication

## Overview

RS-485 is a standard communication protocol used for serial communication over long distances in various industrial and commercial applications. It serves as a reliable method for transmitting digital data between multiple devices in a network. RS-485 enables robust and bidirectional communication, allowing for the exchange of information between devices such as sensors, controllers, and other peripherals.

In this example we will see how to use rs485 to send very precise information.

## Hardware setup and requirements

![schema](Image/schema_rs485_TWIST.png)

You will need :

- Two TWIST
- A power supply (10 to 100V, 30V preferable)
- A RJ45 cable

Connect the two twist with the RJ45, then supply the two twist with the DC supply.

## Software

The two boards have different roles, one is the server the other is the client. The server send the a value via rs485 communication and the client receive it.

### Server code

The server will send a sinewave varying  between 0 and 1. To flash the server make sure that line 44 is defined as :

```cpp
#define SERVER
```

We initalize the rs485 communication with :

```cpp
struct consigne_struct
{
    float32_t Sine_ref;    // sinewave reference
};

struct consigne_struct tx_consigne;
struct consigne_struct rx_consigne;

uint8_t* buffer_tx = (uint8_t*)&tx_consigne;
uint8_t* buffer_rx =(uint8_t*)&rx_consigne;

communication.rs485.configure(buffer_tx, buffer_rx, sizeof(consigne_struct), reception_function, SPEED_20M); // configuration for RS485 to have a 20Mbits/s speed communication
```

We have two structure, one for sending message (tx_consigne) and the other for receiving message (rx_consigne). You only need to work on these structures to use the rs485. Currently, these structures only contain one information wich is Sine_ref, but you can add more information in consigne_struct if you wish.

buffer_tx and buffer_rx are the pointer casted as uint8_t so that the DMA (direct access) can write/read directly on the structure.

The function rs485Communication.configure uses several parameter :
- buffer_tx and buffer_rx to have the address of the structure to read the data to send and write the data receveid.
- The size of the data we want to send and receive. For example here we want to send a sinewave reference in the case of ther server, and receive this sinewave reference in the case of the client. A float represents 4 byte generally. So the size of the data is 4. It is important that the size of the received and sent data are the same i.e. you can't have the server sending 2 floats and expeting the client to receive 1 float. 
- Then we have the reception function, you need to pass either a function of the type `void reception_function(void) ` or ` NULL `. The reception function alllows you to get in this function whenever you received data from RS485 and perform some data processing. 
- Finally, you have to choose the trnasmission speed. In this example the speed is 20 Mbits/s but you can also choose 10, 5 and 2 Mbits/s. It is crucial that the server and client have the same communication speed.


Server is sending a sinewave reference from the critical task :

```cpp
t = counter_sinus / (SAMPLE_RATE);
sine_ref = AMPLITUDE * sin(2 * PI * SIGNAL_FREQ * t) + 0.5;
counter_sinus++;
if(counter_sinus > num_samples) counter_sinus = 0;

tx_consigne.Sine_ref = sine_ref;

communication.rs485.startTransmission();
```

When you use `startTransmission` the current content of the structure tx_consigne will be send to the client.

### Client code
The client is following the same step but instead of sending values, it is receiving and reading it in the reception function when server sends datas:

```cpp
void reception_function(void)
{
    sine_ref_rs485 = rx_consigne.Sine_ref;
}
```
Then sine_ref_rs485 is dsiplayed on the serial monitor.


## Expected result

Use [ownplot](https://github.com/owntech-foundation/OwnPlot) and connect it to the client. You should see the sinewave.

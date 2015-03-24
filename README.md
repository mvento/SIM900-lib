# SIM900 Basic request library
Simplest way to make GET and POST request over GSM network.

### Description
SIM900 basic request library provides an easy way to communicate with your REST API or web service using the SIM900 GSM module and GET and POST requests. 

This library also works on SIM900A but remember the [SIM900A **area restrictions**](http://www.blog.zapro.dk/?p=368)!


### Requirements
You can run this library over all arduino boards, but it is almost obligatory to increase the Arduino Serial buffer (it's very easy, [try it](http://goo.gl/K3thRR)). The minimum recommended buffer size is 128kb, but it depends on your application requirements (bigger buffer allows bigger url and bigger requests). 

I recommend to use 128KB buffer size for Arduino Uno (Atmega 328p) and 256KB for Arduno Mega (Atmega 2560). 

**Important!!**

Serial buffer is stored in RAM memory. When you increase this buffer, the totally RAM available for our program decrements.

### Limitations
The Serial Buffer size restricts the maximum length of the url (host+path+url) and the size of the reply. 

Maximum url length: _MaxBufferSize_ - 25
Maximum reply size: _MaxBufferSize_ - 36

### Installation
It doesn't require any special action for install. Haven't you ever installed a library? [Try it](http://arduino.cc/en/guide/libraries)


### Using SIM900 library
You can make request in few lines of code. Check the library examples for more information.

#### Init:
```Arduino
Connection * SIM900 = new Connection( pinCode, apn, apnUser, apnPassword, enablePin, serialNumber );
SIM900->Configuration();
```
#### Get Request:
```Arduino
char * bodyReply = NULL;
SIM900->Get( host, path, url, headerHttpReply, bodyReply );
// Do something with bodyReply
delete bodyReply;
```

#### Post Request:
```Arduino
char dataToSend[] = {"temperature":20};
SIM900->Post( host, path, url, dataToSend, headerHttpReply );
```

### SIM900 boards compatibility
The library isn't subjected to any particular SIM900 board/shield. You can configurate the board pin layout easily. 

I tested the library in these boards:
* SIM900 MINI   ([Needs a hack!](http://www.emevento.com/blog/sim900-mini-hack))
* [CookingHacks (libelium) GPRS/GSM QUADBAND MODULE](http://goo.gl/mZYEM9)

**Important!!**

If your SIM900 shield wakes up at the same time as Arduino (it doesn't have any button or pin to wake up, like SIM900 MINI), it's necessary to wait at least 1000ms before run Configure() method. This is because SIM900 sends garbage data through Serial when it wakes up.

If your SIM900 board wakes up with the power button/pin or it is already running, this action is not necessary.

### License
Released under MIT license.
# Comelit Simplebus 1 compatible smart audio intercom
If you live in an older building with an audio-only door phone based on the SimpleBus version 1, you may have no choice for upgrading to a smart alternative. This project aims to build an open implementation of a smart doorphone compatible with the comelit simplebus 1 audio protocol, to replace ordinary units such as 2408w/a, 2708W and 2xx8W in general.

```diff
- If you are interested in the project, please get in touch! Your expertise is appreciated.
```

## Usage

Wire according to the wiring diagram; **mind the polarity of the bus!**

![](wiring.gif)

The test sketch contains a SimpleBus class, which allows to receive and send commands on the bus.
The test sketch will print a log on the serial of the messages received on the bus.

Example of a communication
```
time: [41.404s] message: [000011 10110000 1010 - CALL 13 chkOK]
time: [41.457s] ack
time: [47.914s] message: [100010 10110000 1010 - PUP1 13 chkOK]
time: [47.961s] ack
time: [48.128s] message: [100011 10110000 0110 - PUP2 13 chkOK]
time: [48.172s] ack
time: [49.524s] message: [000010 10110000 0010 - OPEN 13 chkOK]
time: [49.571s] ack
time: [51.316s] message: [000010 10110000 0010 - OPEN 13 chkOK]
time: [51.362s] ack
time: [53.545s] message: [000010 10110000 0010 - OPEN 13 chkOK]
time: [53.592s] ack
time: [58.221s] message: [010010 10110000 1010 - HUP 13 chkOK]
time: [58.256s] ack
time: [68.616s] message: [111111 11111111 0111 - CLEAR 255 chkOK]
time: [68.952s] message: [111111 11111111 0111 - CLEAR 255 chkOK]
time: [69.289s] message: [111111 11111111 0111 - CLEAR 255 chkOK]
time: [69.626s] message: [111111 11111111 0111 - CLEAR 255 chkOK]
time: [69.963s] message: [111111 11111111 0111 - CLEAR 255 chkOK]
```


## State of the project
Tested on Wemos D1 mini (esp8266 arduino core 2.4.2).

TODO list:
- [ ] ~~Getting power from the bus~~ (it appears that there is not enough power)
- [x] Receiving messages from the bus
- [x] Sending messages on the bus
- [x] Listening to analog audio on the bus
- [ ] Transmitting analog audio on the bus
- [x] Porting on Wemos D1
- [x] Adding wifi communication
- [x] Implementing sleep
- [ ] Intercom protocol implementation (to be tested)
- [ ] Recording audio through the microcontroller ADC
- [ ] Transmitting audio through a DAC
- [ ] Web/cloud interface


## Acknowledgements
Thank you to Antonio Avallone for being the mantainer of the electronics side of this project

## Previous work
* http://stdio.be/blog/2014-08-17-Fixing-the-intercom-with-logic-analyzer/1
* https://hackaday.com/2019/10/27/reverse-engineering-a-two-wire-intercom/

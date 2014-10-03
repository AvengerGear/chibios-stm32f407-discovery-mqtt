STM32F407 Discovery MQTT Demo
=============================

Target
------

The demo runs on an STM32F407-Discovery board, with an external PHY attached
using [a method described in TKJ Electronics Website][1].

The board file is located in `boards` directory. Please copy the
`NONSTANDARD_STM32F4_DISCOVERY_DP83848` into `boards` directory of ChibiOS/RT
installation. And, please modify `Makefile` to match your environment. Also if
you are using different board from STM32F407-Discovery, modifying `Makefile`
and `mcuconf.h` is mandatory.

Current code is based on ChibiOS/RT 2.6.5.

The Demo
--------

This demo connects to a MQTT broker at `192.168.1.1:1883`, and listens message
to its own channel.  By sending `on` and `off` to the channel, the board
switches the state of D0 and LED5.

Also, this board has its own web server running at `192.168.1.20:80`, and the
user button activates che ChibiOS/RT test suite, output on SD6.

Notes
-----

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library [on the ST web site][2]

This demo also relies on [Paho Embedded MQTT C Client][3] for MQTT
communication.

Licenses
--------

This demo, other than files under `mqtt` directory, is licensed under Apache
2.0 License, [according to the declaration on ChibiOS/RT website][4].

The files under `mqtt` directory is dual-licensed under Eclipse Public License
and Eclipse Distribution License, which its license file are included as
`epl-10` and `edl-10`.

[1]: http://blog.tkjelectronics.dk/2012/08/ethernet-on-stm32f4discovery-using-external-phy/
[2]: http://www.st.com
[3]: http://www.eclipse.org/paho/
[4]: http://www.chibios.org/dokuwiki/doku.php?id=chibios:license

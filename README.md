dbus-dsmr-p1
============

This application reads data from a (dsmr) P1 compatible grid meter and publishes it to the D-Bus.
The published D-bus objects are compatible with the applications running on the Venus platform,
used by Victron Energy on their Color Control (CCGX).

Caveats:
* This application has been tested on a single grid meter (KFM5KAIFA-METER).
* There has not been any integration testing on the Venus platform.

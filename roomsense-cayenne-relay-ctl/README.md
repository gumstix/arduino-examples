# The Gumstix RoomSense Christmas IoTree
*This is an example sketch for the Gumstix RoomSense ATSAMW25 USB dongle* 

## Introduction

This RoomSense sketch is designed to activate a [power relay](http://digitalloggers.com) with passive IR occupancy detection or by remote control via [MyDevices](http://www.mydevices.com)  Cayenne.

## Files

There are three files in this sketch.  The first is this readme.  Cayenne_Xmas_Tree.ino is the main application and settings.h.example is a template of the manual settings required for WiFi, OTA, and Cayenne connections.

## Circuit

* `Pin 10` is the relay control pin, connected to the positive input terminal of the relay
* `GND` is connected to the negative terminal of the relay

## Info

The tree is activated by the PIR motion sensor when the room is occupied.  After a vairiable interval (in minutes) of inactivity, the lights are turned off.  The interval can be set with a slider input within the Cayenne project on channel 4 and the tree can be maually activated by a Cayenne button on channel 7.

The sensors on the RoomSense also deliver temperature, humidity and barometric pressure.
# Telemetry Tracker Capstone Design Group
# Ryan Blushke, ryb861, 11177824
# Scott Seidle, ses832, 11182580
# Alex McNabb
# Torban Peterson
# CME/EE 495
# November 11, 2019
SHELL = /bin/sh
CC = gcc
CPPFLAGS = -std=gnu90 -Wall -pedantic
CFLAGS = -g
LDFLAGS = -g

.PHONY: all clean

all: tracker

tracker: tracker.o
	$(CC) $(LDFLAGS) -o $@ tracker.o

tracker.o: tracker.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

clean:
	rm -f tracker.o tracker

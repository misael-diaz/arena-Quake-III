#!/usr/bin/make
#
# Quake-III						June 07, 2024
#
# source: src/system/Makefile
# author: @misael-diaz
#
# Synopsis:
# Defines the Makefile for building the program with GNU make.
#
# Copyright (c) 2024 Misael Díaz-Maldonado
# This file is released under the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#

include make-inc

all: $(SYSTEM_O)

$(SYSTEM_O): $(HEADERS) $(SYSTEM_C)
	$(CC) $(INC) $(CCOPT) -c $(SYSTEM_C) -o $(SYSTEM_O)

clean:
	/bin/rm -f *.o

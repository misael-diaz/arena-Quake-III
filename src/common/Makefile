#!/usr/bin/make
#
# Quake-III						June 07, 2024
#
# source: src/common/Makefile
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

all: $(OBJS_O)

$(MAIN_O): $(HEADERS) $(MAIN_C)
	$(CC) $(INC) $(CCOPT) -c $(MAIN_C) -o $(MAIN_O)

$(FS_O): $(HEADERS) $(FS_C)
	$(CC) $(INC) $(CCOPT) -c $(FS_C) -o $(FS_O)

$(CONTROL_VARIABLES_O): $(HEADERS) $(CONTROL_VARIABLES_C)
	$(CC) $(INC) $(CCOPT) -c $(CONTROL_VARIABLES_C) -o $(CONTROL_VARIABLES_O)

clean:
	/bin/rm -f *.o

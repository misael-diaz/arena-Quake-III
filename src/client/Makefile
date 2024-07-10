#!/usr/bin/make
#
# Quake-III						June 07, 2024
#
# source: src/client/Makefile
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

$(SCREEN_O): $(HEADERS) $(SCREEN_C)
	$(CC) $(INC) $(CCOPT) -c $(SCREEN_C) -o $(SCREEN_O)

$(VIEW_O): $(HEADERS) $(VIEW_C)
	$(CC) $(INC) $(CCOPT) -c $(VIEW_C) -o $(VIEW_O)

$(ENTITY_O): $(HEADERS) $(ENTITY_C)
	$(CC) $(INC) $(CCOPT) -c $(ENTITY_C) -o $(ENTITY_O)

$(MAIN_O): $(HEADERS) $(MAIN_C)
	$(CC) $(INC) $(CCOPT) -c $(MAIN_C) -o $(MAIN_O)

clean:
	/bin/rm -f *.o

#!/usr/bin/make
#
# Quake-III						June 07, 2024
#
# source: src/graphics/Makefile
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

$(BSP_O): $(HEADERS) $(BSP_C)
	$(CC) $(INC) $(CCOPT) -c $(BSP_C) -o $(BSP_O)

$(DRAW_O): $(HEADERS) $(DRAW_C)
	$(CC) $(INC) $(CCOPT) -c $(DRAW_C) -o $(DRAW_O)

$(MODEL_O): $(HEADERS) $(MODEL_C)
	$(CC) $(INC) $(CCOPT) -c $(MODEL_C) -o $(MODEL_O)

$(SURFACE_O): $(HEADERS) $(SURFACE_C)
	$(CC) $(INC) $(CCOPT) -c $(SURFACE_C) -o $(SURFACE_O)

$(GRAPHICS_O): $(HEADERS) $(GRAPHICS_C)
	$(CC) $(INC) $(CCOPT) -c $(GRAPHICS_C) -o $(GRAPHICS_O)

$(MAIN_O): $(HEADERS) $(MAIN_C)
	$(CC) $(INC) $(CCOPT) -c $(MAIN_C) -o $(MAIN_O)

clean:
	/bin/rm -f *.o

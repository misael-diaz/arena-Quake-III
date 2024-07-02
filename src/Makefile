#!/usr/bin/make
#
# Quake-III				June 07, 2024
#
# source: Makefile
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

all: Quake

Util:
	@$(MAKE) -C util
Video:
	@$(MAKE) -C video
Graphics: Video
	@$(MAKE) -C graphics
Common: Util Graphics
	@$(MAKE) -C common
System: Util
	@$(MAKE) -C system
Quake: Common System
	@$(MAKE) -C main
clean:
	@$(MAKE) -C util clean
	@$(MAKE) -C video clean
	@$(MAKE) -C graphics clean
	@$(MAKE) -C system clean
	@$(MAKE) -C main clean

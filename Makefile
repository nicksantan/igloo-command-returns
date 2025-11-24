# Makefile for SGDK project

# Project name (output ROM file will be named this)
BIN = missile_command.bin

# SGDK installation path - adjust this to your SGDK location
# Common locations:
# Windows: C:/sgdk or C:/SGDK
# Linux/Mac: ~/sgdk or /opt/sgdk
GDK = $(SGDK)

# Source and resource directories
SRC_DIR = src
RES_DIR = res
INC_DIR = inc
OUT_DIR = out

# Compiler flags
FLAGS = -m68000 -Wall -Wextra -std=c99 -ffreestanding

# Include SGDK makefile
include $(GDK)/makefile.gen

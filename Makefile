F_CPU = 8000000
ARCH = AVR8
MCU = atmega8

VPATH = ../libjymcu3208/include

TARGET = tetris

SRC = tetris.c music.c

all: tune.h lookup.h

tune.h: create_music.py
	python create_music.py ${F_CPU} > tune.h || (rm tune.h && false)

lookup.h: compute_lookup.py
	python compute_lookup.py > lookup.h || (rm lookup.h && false)

include rules.mk

EXTRALIBDIRS = ../libjymcu3208
LDFLAGS += -ljymcu3208

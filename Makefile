F_CPU = 8000000
ARCH = AVR8
MCU = atmega8

TARGET = tetris

SRC = tetris.c mq.c ht1632c.c music.c timers.c

all: tune.h lookup.h

tune.h: create_music.py
	python create_music.py ${F_CPU} > tune.h || (rm tune.h && false)

lookup.h: compute_lookup.py
	python compute_lookup.py > lookup.h || (rm lookup.h && false)

include rules.mk

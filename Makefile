F_CPU = 8000000
ARCH = AVR8
MCU = atmega8

TARGET = tetris

SRC = tetris.c mq.c ht1632c.c

include rules.mk

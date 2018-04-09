##
 # Mieszko Mazurek <mimaz@gmx.com>
 #
 # Apr 2018
 ##

BUILD_DIR = /tmp/mavrt

ELF = ${BUILD_DIR}/mavrt.elf
HEX = ${BUILD_DIR}/mavrt.hex
F_CPU = 16000000
MCU = atmega328p
PROGRAMMER = arduino
PORT = /dev/ttyACM0


CFLAGS = -Os -Wall -std=c99 -mmcu=${MCU} \
		 -DF_CPU=${F_CPU} -MMD

LDFLAGS = -mmcu=${MCU}


SRC = kernel.S scheduler.c test.c


OBJ = ${SRC:%=${BUILD_DIR}/%.o}
DEP = ${OBJ:%.o=%.d}


all: ${HEX}

clean: 
	rm -rf ${BUILD_DIR}

run: ${HEX}
	avrdude -c ${PROGRAMMER} -p ${MCU} -P ${PORT} -Uflash:w:$<

size: ${ELF}
	avr-size $<

.PHONY: all clean run

${HEX}: ${ELF}
	avr-objcopy -j .text -j .data -Oihex $< $@

${ELF}: ${OBJ}
	avr-gcc ${LDFLAGS} $^ -o $@

${BUILD_DIR}/%.o: %
	@mkdir -p ${dir $@}
	avr-gcc ${CFLAGS} -c $< -o $@

-include ${DEP}

# Force rpath, not rpath-link.
RELEASE = 0
TARGET = simulation

IPATH = .
IPATH += ${SIMAVR_DIR}/examples/parts
IPATH += ${SIMAVR_DIR}/simavr/sim

VPATH = .
VPATH += ${SIMAVR_DIR}/examples/parts

LDFLAGS += -lpthread

build: obj ${TARGET}

include ${SIMAVR_DIR}/Makefile.common

# We need to fix these settings because Makefile.common sets wrong path.
LIBDIR = ${SIMAVR_DIR}/simavr/${OBJ}
#LDFLAGS += -L${LIBDIR} -lsimavr

BOARD = ${OBJ}/${TARGET}.elf

${BOARD} : ${OBJ}/button.o
${BOARD} : ${OBJ}/${TARGET}.o

${TARGET} : ${BOARD}
	@echo $@ done

clean: clean-${OBJ}
	rm -rf *.a *.axf ${TARGET} *.vcd *.hex

run: build
	exec ./${OBJ}/${TARGET}.elf

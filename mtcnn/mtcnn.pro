TEMPLATE = lib

DESTDIR = $${_PRO_FILE_PWD_}/../lib

CONFIG += dll
CONFIG += c++17
CONFIG -= app_bundle
CONFIG -= qt

HEADERS += ../mtcnn.h

SOURCES += ../mtcnn.cpp

LIBS += -lncnn

QMAKE_CXXFLAGS += -O3
QMAKE_CXXFLAGS += -march=native
QMAKE_CXXFLAGS += -std=c++17

nasm.name = nasm ${QMAKE_FILE_IN}
nasm.input = NASM_INPUT
nasm.variable_out = OBJECTS
nasm.commands = nasm -f elf64 -I${QMAKE_FILE_IN_PATH} ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
nasm.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_IN_BASE}$${first(QMAKE_EXT_OBJ)}
nasm.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS  += nasm

NASM_INPUT += ../mtcnn_p_model.asm
NASM_INPUT += ../mtcnn_p_param.asm
NASM_INPUT += ../mtcnn_r_model.asm
NASM_INPUT += ../mtcnn_r_param.asm
NASM_INPUT += ../mtcnn_o_model.asm
NASM_INPUT += ../mtcnn_o_param.asm

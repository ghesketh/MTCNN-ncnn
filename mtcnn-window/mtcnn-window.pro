TEMPLATE = app

DESTDIR = $${_PRO_FILE_PWD_}/../bin

CONFIG += console
CONFIG += c++17
CONFIG -= app_bundle
CONFIG -= qt

HEADERS += ../main.h \
    ../stb_image.h
HEADERS += ../mtcnn.h

LIBS += -L$${_PRO_FILE_PWD_}/../lib -lmtcnn

LIBS += -lSDL2
LIBS += -lSDL2_image

SOURCES += ../main.cpp
SOURCES += ../mtcnn-window.cpp

QMAKE_CXXFLAGS += -O3
QMAKE_CXXFLAGS += -march=native
QMAKE_CXXFLAGS += -std=c++17

#-------------------------------------------------
#
# Project created by QtCreator 2012-07-22T22:03:53
#
#-------------------------------------------------

QT       += core gui

TARGET = LP2
TEMPLATE = app


SOURCES += main.cpp\
    SerialDevice.cpp \
    SequentialDevice.cpp \
    Fft.cpp \
    LP2.cpp \
    ../../LuminousPassage2/pattern.cpp \
    ../../LuminousPassage2/audio.cpp \
    Lights.cpp \
    PatternThread.cpp \
    ../../LuminousPassage2/trig.cpp

HEADERS  += \
    SerialDevice.h \
    SequentialDevice.h \
    Fft.h \
    LP2.h \
    ../../LuminousPassage2/pattern.h \
    ../../LuminousPassage2/global.h \
    ../../LuminousPassage2/audio.h \
    Lights.h \
    PatternThread.h \
    ../../LuminousPassage2/com.h \
    ../../LuminousPassage2/trig.h

DEFINES += SOFTWARE

INCLUDEPATH += ../../LuminousPassage2

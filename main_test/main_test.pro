QT -= gui

CONFIG += c++17
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

! include( ../common.pri ) {
    error( "Couldn't find the common.pri file!" )
}

LIBS += -lpcosynchro
LIBS += -lgtest
LIBS += -lpthread

SOURCES += \
    main_test.cpp

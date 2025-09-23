QT -= gui

QMAKE_CXXFLAGS += -O3

CONFIG += c++17
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

! include( ../common.pri ) {
    error( "Couldn't find the common.pri file!" )
}

DEFINES += LOGGING_BE_SILENT

LIBS += -lpcosynchro
LIBS += -lbenchmark
LIBS += -lpthread

SOURCES += \
    main_benchmark.cpp

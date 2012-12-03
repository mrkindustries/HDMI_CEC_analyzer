TARGET = HdmiCecAnalyzer

SALEAE_SDK_PATH= /opt/SaleaeAnalyzerSdk

TEMPLATE = lib
CONFIG -= qt
CONFIG += plugin debug_and_release
DEFINES -= QT_WEBKIT

# Saleae libs and includes
INCLUDEPATH += $${SALEAE_SDK_PATH}/include
LIBS += -L$${SALEAE_SDK_PATH}/lib
contains(QMAKE_HOST.arch, x86_64): { INCLUDEPATH += -Analyzer64 }
!contains(QMAKE_HOST.arch, x86_64): { INCLUDEPATH += -Analyzer }

Release:DESTDIR = release
Release:OBJECTS_DIR = release

Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug

QMAKE_CXX = g++

QMAKE_CXXFLAGS_DEBUG = -O0 -w -fpic -g
QMAKE_CXXFLAGS_RELEASE = -O3 -w -fpic

QMAKE_LFLAGS_DEBUG = -O0 -w -fpic -g
QMAKE_LFLAGS_RELEASE = -O0 -w -fpic -g

SOURCES += \
          source/HdmiCecAnalyzer.cpp \
          source/HdmiCecAnalyzerResults.cpp \
          source/HdmiCecAnalyzerSettings.cpp \
          source/HdmiCecSimulationDataGenerator.cpp

HEADERS += \
          source/HdmiCecAnalyzer.h \
          source/HdmiCecAnalyzerResults.h \
          source/HdmiCecAnalyzerSettings.h \
          source/HdmiCecSimulationDataGenerator.h

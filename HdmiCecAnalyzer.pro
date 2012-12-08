TARGET = HdmiCecAnalyzer

SALEAE_LOGIC_PATH= /opt/SaleaeLogic
SALEAE_INCLUDE_PATH= /opt/SaleaeAnalyzerSdk/include

TEMPLATE = lib
CONFIG -= qt
CONFIG += plugin debug_and_release
DEFINES -= QT_WEBKIT

# Use includes from the SDK
INCLUDEPATH += $${SALEAE_INCLUDE_PATH}
# Use the same libAnalyzer that Logic uses
LIBS += -L$${SALEAE_LOGIC_PATH} -lAnalyzer
#contains(QMAKE_HOST.arch, x86_64): { LIBS += -lAnalyzer64 }
#!contains(QMAKE_HOST.arch, x86_64): { LIBS += -lAnalyzer }

Release:DESTDIR = release
Release:OBJECTS_DIR = release

Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug

QMAKE_CXXFLAGS_DEBUG = -O0 -Wall -fpic -g
QMAKE_CXXFLAGS_RELEASE = -O3 -Wall -fpic

QMAKE_LFLAGS_DEBUG = -O0 -Wall -fpic -g
QMAKE_LFLAGS_RELEASE = -O0 -Wall -fpic -g

SOURCES += \
    source/HdmiCecAnalyzer.cpp \
    source/HdmiCecAnalyzerResults.cpp \
    source/HdmiCecAnalyzerSettings.cpp \
    source/HdmiCecProtocol.cpp \
    source/HdmiCecSimulationDataGenerator.cpp

HEADERS += \
    source/HdmiCecAnalyzer.h \
    source/HdmiCecAnalyzerResults.h \
    source/HdmiCecAnalyzerSettings.h \
    source/HdmiCecProtocol.h \
    source/HdmiCecSimulationDataGenerator.h

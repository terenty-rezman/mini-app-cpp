# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

TEMPLATE = app
TARGET = mini-app
DESTDIR = ../Release
CONFIG += release
LIBS += -L"."
DEPENDPATH += .
MOC_DIR += .
OBJECTS_DIR += release
UI_DIR += .
RCC_DIR += .
QT += widgets
CONFIG += c++17
QMAKE_CXXFLAGS += /std:c++17
win32:RC_ICONS += ./ics/water_pump_1.ico
include(mini-app.pri)

#-------------------------------------------------
#
# Project created by QtCreator 2016-02-05T14:00:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = turtyl
TEMPLATE = app

CONFIG += static

VERSION = 0.1.0

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += src/main.cpp\
    src/mainwindow.cpp \
    src/commandrunner.cpp \
    src/basiccommands.cpp \
    src/preferencesdialog.cpp \
    src/helpdialog.cpp \
    src/turtlegraphicswidget.cpp

HEADERS  += src/mainwindow.h \
    src/commandrunner.h \
    src/basiccommands.h \
    src/preferencesdialog.h \
    src/helpdialog.h \
    src/turtlegraphicswidget.h

FORMS    += forms/mainwindow.ui \
    forms/preferencesdialog.ui \
    forms/helpdialog.ui

# Lua sources
INCLUDEPATH += src/lua
SOURCES += src/lua/lapi.c \
    src/lua/lcode.c \
    src/lua/lctype.c \
    src/lua/ldebug.c \
    src/lua/ldo.c \
    src/lua/ldump.c \
    src/lua/lfunc.c \
    src/lua/lgc.c \
    src/lua/llex.c \
    src/lua/lmem.c \
    src/lua/lobject.c \
    src/lua/lopcodes.c \
    src/lua/lparser.c \
    src/lua/lstate.c \
    src/lua/lstring.c \
    src/lua/ltable.c \
    src/lua/ltm.c \
    src/lua/lundump.c \
    src/lua/lvm.c \
    src/lua/lzio.c \
    src/lua/lauxlib.c \
    src/lua/lbaselib.c \
    src/lua/lbitlib.c \
    src/lua/lcorolib.c \
    src/lua/ldblib.c \
    src/lua/liolib.c \
    src/lua/lmathlib.c \
    src/lua/loslib.c \
    src/lua/lstrlib.c \
    src/lua/ltablib.c \
    src/lua/lutf8lib.c \
    src/lua/loadlib.c \
    src/lua/linit.c

OTHER_FILES += \
    scripts/turtle.lua

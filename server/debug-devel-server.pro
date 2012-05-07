# -------------------------------------------------
# Project created by QtCreator 2009-06-16T12:00:07
# -------------------------------------------------
QT += network
TARGET = debug-devel-server
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    qtiocompressor.cpp \
    lz4hc.c \
    lz4.c
HEADERS += mainwindow.h \
    qtiocompressor.h \
    lz4hc.h \
    lz4.h
FORMS += mainwindow.ui
win32:RC_FILE += resources-windows.rc
RESOURCES += resources.qrc

# mainwindows.h
# -------------------
# Begin : Mon Nov 29 2008 18:20 alpha_one_x86
# Project : debug-devel
# Email : alpha.super-one@laposte.net
# Note : See doc for copyright and developer
# Target : Class for do testing of communication
QT += network
TARGET = debug-devel
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    qtiocompressor.cpp \
    lz4hc.c \
    lz4.c
HEADERS += mainwindow.h \
    rs232-mode.h \
    qtiocompressor.h \
    lz4hc.h \
    lz4.h
FORMS += MainWindow.ui
RESOURCES += resources.qrc
win32:RC_FILE += resources-windows.rc
#unix:DEFINES = _TTY_POSIX_
#win32:DEFINES = _TTY_WIN_
#DEPENDPATH += .
#INCLUDEPATH += qextserialport
#QMAKE_LIBDIR += qextserialport/build
#CONFIG(debug, debug|release):LIBS += -lqextserialportd
#else:LIBS += -lqextserialport


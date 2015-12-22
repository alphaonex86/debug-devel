QT       += gui network xml core widgets
TARGET = debug-devel-client
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    CompressionTcpSocket/zlib/qtiocompressor.cpp \
    CompressionTcpSocket/lz4/lz4hc.c \
    CompressionTcpSocket/lz4/lz4.c \
    CompressionTcpSocket/lz4/Lz4CompressionTcpSocket.cpp \
    CompressionTcpSocket/lz4/Lz4HcCompressionTcpSocket.cpp \
    CompressionTcpSocket/zlib/ZlibCompressionTcpSocket.cpp \
    CompressionTcpSocket/zlib/GzipCompressionTcpSocket.cpp \
    CompressionTcpSocket/NoCompressionTcpSocket.cpp \
    CompressionTcpSocket/CompressionTcpSocketInterface.cpp \
    CompressionTcpSocket/zlib/ZlibAbstractCompressionTcpSocket.cpp \
    CompressionTcpSocket/HeaderCompressedSizeCompressionTcpSocket.cpp \
    CompressionTcpSocket/HeaderUncompressedSizeCompressionTcpSocket.cpp
HEADERS += mainwindow.h \
    rs232-mode.h \
    CompressionTcpSocket/zlib/qtiocompressor.h \
    CompressionTcpSocket/lz4/lz4hc.h \
    CompressionTcpSocket/lz4/lz4.h \
    CompressionTcpSocket/lz4/Lz4CompressionTcpSocket.h \
    CompressionTcpSocket/lz4/Lz4HcCompressionTcpSocket.h \
    CompressionTcpSocket/zlib/ZlibCompressionTcpSocket.h \
    CompressionTcpSocket/zlib/GzipCompressionTcpSocket.h \
    CompressionTcpSocket/NoCompressionTcpSocket.h \
    CompressionTcpSocket/CompressionTcpSocketInterface.h \
    CompressionTcpSocket/zlib/ZlibAbstractCompressionTcpSocket.h \
    CompressionTcpSocket/HeaderCompressedSizeCompressionTcpSocket.h \
    CompressionTcpSocket/HeaderUncompressedSizeCompressionTcpSocket.h
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
LIBS += -lz

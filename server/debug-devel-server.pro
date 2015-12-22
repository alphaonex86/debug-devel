QT       += gui network xml core widgets
TARGET = debug-devel-server
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    CompressionTcpSocket/NoCompressionTcpSocket.cpp \
    CompressionTcpSocket/HeaderUncompressedSizeCompressionTcpSocket.cpp \
    CompressionTcpSocket/HeaderCompressedSizeCompressionTcpSocket.cpp \
    CompressionTcpSocket/CompressionTcpSocketInterface.cpp \
    CompressionTcpSocket/lz4/Lz4HcCompressionTcpSocket.cpp \
    CompressionTcpSocket/lz4/lz4hc.c \
    CompressionTcpSocket/lz4/Lz4CompressionTcpSocket.cpp \
    CompressionTcpSocket/lz4/lz4.c \
    CompressionTcpSocket/zlib/ZlibCompressionTcpSocket.cpp \
    CompressionTcpSocket/zlib/ZlibAbstractCompressionTcpSocket.cpp \
    CompressionTcpSocket/zlib/qtiocompressor.cpp \
    CompressionTcpSocket/zlib/GzipCompressionTcpSocket.cpp
HEADERS += mainwindow.h \
    CompressionTcpSocket/NoCompressionTcpSocket.h \
    CompressionTcpSocket/HeaderUncompressedSizeCompressionTcpSocket.h \
    CompressionTcpSocket/HeaderCompressedSizeCompressionTcpSocket.h \
    CompressionTcpSocket/CompressionTcpSocketInterface.h \
    CompressionTcpSocket/lz4/Lz4HcCompressionTcpSocket.h \
    CompressionTcpSocket/lz4/lz4hc.h \
    CompressionTcpSocket/lz4/Lz4CompressionTcpSocket.h \
    CompressionTcpSocket/lz4/lz4.h \
    CompressionTcpSocket/zlib/ZlibCompressionTcpSocket.h \
    CompressionTcpSocket/zlib/ZlibAbstractCompressionTcpSocket.h \
    CompressionTcpSocket/zlib/qtiocompressor.h \
    CompressionTcpSocket/zlib/GzipCompressionTcpSocket.h
FORMS += mainwindow.ui
win32:RC_FILE += resources-windows.rc
RESOURCES += resources.qrc
LIBS += -lz

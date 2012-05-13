#-------------------------------------------------
#
# Project created by QtCreator 2012-05-11T15:56:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QTcpCompressionBenchmark
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    CompressionTcpSocket/NoCompressionTcpSocket.cpp \
    CompressionTcpSocket/HeaderUncompressedSizeCompressionTcpSocket.cpp \
    CompressionTcpSocket/HeaderCompressedSizeCompressionTcpSocket.cpp \
    CompressionTcpSocket/CompressionTcpSocketInterface.cpp \
    CompressionTcpSocket/zlib/ZlibCompressionTcpSocket.cpp \
    CompressionTcpSocket/zlib/ZlibAbstractCompressionTcpSocket.cpp \
    CompressionTcpSocket/zlib/qtiocompressor.cpp \
    CompressionTcpSocket/zlib/GzipCompressionTcpSocket.cpp \
    CompressionTcpSocket/lz4/Lz4HcCompressionTcpSocket.cpp \
    CompressionTcpSocket/lz4/lz4hc.c \
    CompressionTcpSocket/lz4/Lz4CompressionTcpSocket.cpp \
    CompressionTcpSocket/lz4/lz4.c

HEADERS  += mainwindow.h \
    CompressionTcpSocket/NoCompressionTcpSocket.h \
    CompressionTcpSocket/HeaderUncompressedSizeCompressionTcpSocket.h \
    CompressionTcpSocket/HeaderCompressedSizeCompressionTcpSocket.h \
    CompressionTcpSocket/CompressionTcpSocketInterface.h \
    CompressionTcpSocket/zlib/ZlibCompressionTcpSocket.h \
    CompressionTcpSocket/zlib/ZlibAbstractCompressionTcpSocket.h \
    CompressionTcpSocket/zlib/qtiocompressor.h \
    CompressionTcpSocket/zlib/GzipCompressionTcpSocket.h \
    CompressionTcpSocket/lz4/Lz4HcCompressionTcpSocket.h \
    CompressionTcpSocket/lz4/lz4hc.h \
    CompressionTcpSocket/lz4/Lz4CompressionTcpSocket.h \
    CompressionTcpSocket/lz4/lz4.h

FORMS    += mainwindow.ui

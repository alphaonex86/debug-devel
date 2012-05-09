#include "ZlibCompressionTcpSocket.h"

ZlibCompressionTcpSocket::ZlibCompressionTcpSocket(int bufferSize,int compression)
	: ZlibAbstractCompressionTcpSocket(bufferSize,compression)
{
	compressor=new QtIOCompressor(buffer_compression,this->compression,this->bufferSize);
	compressor->setStreamFormat(QtIOCompressor::ZlibFormat);
	if(!compressor->open(QIODevice::WriteOnly))
		qDebug() << "Unable to open the compressor";

	decompressor=new QtIOCompressor(buffer_decompression);
	decompressor->setStreamFormat(QtIOCompressor::ZlibFormat);
	if(!decompressor->open(QIODevice::ReadOnly))
		qDebug() << "Unable to open the decompressor";
}

void ZlibCompressionTcpSocket::abstractBlockingMethod()
{
}

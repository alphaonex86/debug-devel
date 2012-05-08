#include "ZlibCompressionTcpSocket.h"

ZlibCompressionTcpSocket::ZlibCompressionTcpSocket(int bufferSize,int compression)
	: ZlibAbstractCompressionTcpSocket(bufferSize,compression)
{
	compressor=new QtIOCompressor(buffer_compression,this->compression,this->bufferSize);
	compressor->setStreamFormat(QtIOCompressor::ZlibFormat);
	compressor->open(QIODevice::WriteOnly);

	decompressor=new QtIOCompressor(buffer_decompression);
	decompressor->setStreamFormat(QtIOCompressor::ZlibFormat);
	decompressor->open(QIODevice::ReadOnly);
}

void ZlibCompressionTcpSocket::abstractBlockingMethod()
{
}

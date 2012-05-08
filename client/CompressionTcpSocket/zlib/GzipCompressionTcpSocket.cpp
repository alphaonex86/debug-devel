#include "GzipCompressionTcpSocket.h"

GzipCompressionTcpSocket::GzipCompressionTcpSocket(int bufferSize,int compression)
	: ZlibAbstractCompressionTcpSocket(bufferSize,compression)
{
	compressor=new QtIOCompressor(buffer_compression,this->compression,this->bufferSize);
	compressor->setStreamFormat(QtIOCompressor::GzipFormat);
	compressor->open(QIODevice::WriteOnly);

	decompressor=new QtIOCompressor(buffer_decompression);
	decompressor->setStreamFormat(QtIOCompressor::GzipFormat);
	decompressor->open(QIODevice::ReadOnly);
}

void GzipCompressionTcpSocket::abstractBlockingMethod()
{
}

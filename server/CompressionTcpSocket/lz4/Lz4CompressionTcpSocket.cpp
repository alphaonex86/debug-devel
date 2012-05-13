#include "Lz4CompressionTcpSocket.h"

Lz4CompressionTcpSocket::Lz4CompressionTcpSocket()
	: HeaderUncompressedSizeCompressionTcpSocket()
{
}

QByteArray Lz4CompressionTcpSocket::compressDataWithoutHeader(const QByteArray rawData)
{
	QByteArray compressedData;
	compressedData.resize(maxCompressedSize(rawData.size()));
	int returnSize=LZ4_compress(rawData.constData(),compressedData.data(),rawData.size());
	compressedData.resize(returnSize);
	return compressedData;
}

int Lz4CompressionTcpSocket::decompressDataWithoutHeader(const QByteArray &source,QByteArray *destination)
{
	return LZ4_uncompress_unknownOutputSize(source.constData(),destination->data(),source.size(),destination->size());
}

int Lz4CompressionTcpSocket::maxCompressedSize(const int &maxSize)
{
	return LZ4_compressBound(maxSize);
}


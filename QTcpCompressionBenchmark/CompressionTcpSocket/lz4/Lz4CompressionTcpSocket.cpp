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

bool Lz4CompressionTcpSocket::decompressDataWithoutHeader(const QByteArray &source,QByteArray *destination,int *isize,int *osize)
{
	int returnCode=LZ4_uncompress_unknownOutputSize(source.constData(),destination->data(),isize,*osize);
	if(returnCode<0)
		return false;
	else
	{
		*osize=returnCode;
		return true;
	}
}

int Lz4CompressionTcpSocket::maxCompressedSize(const int &maxSize)
{
	return LZ4_compressBound(maxSize);
}


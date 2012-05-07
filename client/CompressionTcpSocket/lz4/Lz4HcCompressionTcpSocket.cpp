#include "Lz4HcCompressionTcpSocket.h"

Lz4HcCompressionTcpSocket::Lz4HcCompressionTcpSocket(int chunkSize,int compression)
	: Lz4CompressionTcpSocket(chunkSize,compression)
{
}

QByteArray Lz4HcCompressionTcpSocket::real_compressData(const QByteArray &rawData)
{
	QByteArray compressedData;
	int returnSize;
	compressedData.resize(LZ4_compressBound(rawData.size()));
	returnSize=LZ4_compressHC(rawData.constData(),compressedData.data(),rawData.size());
	compressedData.resize(returnSize);
	return compressedData;
}


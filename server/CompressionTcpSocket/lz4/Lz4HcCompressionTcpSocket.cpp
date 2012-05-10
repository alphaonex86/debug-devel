#include "Lz4HcCompressionTcpSocket.h"

Lz4HcCompressionTcpSocket::Lz4HcCompressionTcpSocket()
	: Lz4CompressionTcpSocket()
{
}

QByteArray Lz4HcCompressionTcpSocket::compressDataWithoutHeader(const QByteArray rawData)
{
	QByteArray compressedData;
	compressedData.resize(maxCompressedSize(rawData.size()));
	int returnSize=LZ4_compressHC(rawData.constData(),compressedData.data(),rawData.size());
	compressedData.resize(returnSize);
	return compressedData;
}


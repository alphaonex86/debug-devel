#include "NoCompressionTcpSocket.h"

NoCompressionTcpSocket::NoCompressionTcpSocket(int chunkSize,int compression)
	: CompressionTcpSocketInterface(chunkSize,compression)
{
}

QByteArray NoCompressionTcpSocket::real_compressData(const QByteArray &rawData)
{
	return rawData;
}

QByteArray NoCompressionTcpSocket::real_decompressData(const QByteArray &compressedData)
{
	return compressedData;
}

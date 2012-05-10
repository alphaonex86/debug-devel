#include "NoCompressionTcpSocket.h"

NoCompressionTcpSocket::NoCompressionTcpSocket()
	: CompressionTcpSocketInterface()
{
}

QByteArray NoCompressionTcpSocket::compressData(const QByteArray &rawData)
{
	return rawData;
}

QByteArray NoCompressionTcpSocket::decompressData(const QByteArray &compressedData)
{
	return compressedData;
}

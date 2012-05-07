#ifndef NOCOMPRESSIONTCPSOCKET_H
#define NOCOMPRESSIONTCPSOCKET_H

#include "CompressionTcpSocketInterface.h"

class NoCompressionTcpSocket : public CompressionTcpSocketInterface
{
public:
	NoCompressionTcpSocket(int chunkSize=65536,int compression=6);
private:
	QByteArray real_compressData(const QByteArray &rawData);
	QByteArray real_decompressData(const QByteArray &compressedData);
};

#endif // NOCOMPRESSIONTCPSOCKET_H

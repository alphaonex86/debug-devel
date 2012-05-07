#ifndef LZ4HCCOMPRESSIONTCPSOCKET_H
#define LZ4HCCOMPRESSIONTCPSOCKET_H

#include "Lz4CompressionTcpSocket.h"
#include "lz4hc.h"

class Lz4HcCompressionTcpSocket : public Lz4CompressionTcpSocket
{
public:
	Lz4HcCompressionTcpSocket(int chunkSize=65536,int compression=6);
protected:
	virtual QByteArray real_compressData(const QByteArray &rawData);
};

#endif // LZ4HCCOMPRESSIONTCPSOCKET_H

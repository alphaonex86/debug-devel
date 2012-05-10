#ifndef LZ4HCCOMPRESSIONTCPSOCKET_H
#define LZ4HCCOMPRESSIONTCPSOCKET_H

#include "Lz4CompressionTcpSocket.h"
#include "lz4hc.h"

class Lz4HcCompressionTcpSocket : public Lz4CompressionTcpSocket
{
public:
	Lz4HcCompressionTcpSocket();
protected:
	virtual QByteArray compressDataWithoutHeader(const QByteArray rawData);
};

#endif // LZ4HCCOMPRESSIONTCPSOCKET_H

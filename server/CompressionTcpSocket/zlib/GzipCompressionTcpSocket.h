#ifndef GZIPCOMPRESSIONTCPSOCKET_H
#define GZIPCOMPRESSIONTCPSOCKET_H

#include "ZlibAbstractCompressionTcpSocket.h"

class GzipCompressionTcpSocket : public ZlibAbstractCompressionTcpSocket
{
public:
	GzipCompressionTcpSocket(int bufferSize=65536,int compression=6,bool byPacket=false);
private:
	void abstractBlockingMethod();
};

#endif // GZIPCOMPRESSIONTCPSOCKET_H

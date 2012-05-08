#ifndef ZLIBCOMPRESSIONTCPSOCKET_H
#define ZLIBCOMPRESSIONTCPSOCKET_H

#include "ZlibAbstractCompressionTcpSocket.h"

class ZlibCompressionTcpSocket : public ZlibAbstractCompressionTcpSocket
{
public:
	ZlibCompressionTcpSocket(int bufferSize=65536,int compression=6);
private:
	void abstractBlockingMethod();
};

#endif // ZLIBCOMPRESSIONTCPSOCKET_H

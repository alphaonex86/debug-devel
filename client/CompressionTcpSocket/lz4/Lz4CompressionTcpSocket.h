#ifndef LZ4COMPRESSIONTCPSOCKET_H
#define LZ4COMPRESSIONTCPSOCKET_H

#include "../HeaderCompressedSizeCompressionTcpSocket.h"
#include "lz4.h"

class Lz4CompressionTcpSocket : public HeaderCompressedSizeCompressionTcpSocket
{
public:
	Lz4CompressionTcpSocket();
protected:
	virtual QByteArray compressDataWithoutHeader(const QByteArray rawData);
	virtual int decompressDataWithoutHeader(const QByteArray &source,QByteArray *destination,const int &maxOutputSize);
	int maxCompressedSize(const int &maxSize);
private:
	int QLZ4_uncompress_unknownOutputSize(const QByteArray &source,QByteArray *destination,const int &maxOutputSize);
	QByteArray decompression_buffer;
};

#endif // LZ4COMPRESSIONTCPSOCKET_H

#ifndef LZ4COMPRESSIONTCPSOCKET_H
#define LZ4COMPRESSIONTCPSOCKET_H

#include "../CompressionTcpSocketInterface.h"
#include "lz4.h"

class Lz4CompressionTcpSocket : public CompressionTcpSocketInterface
{
public:
	Lz4CompressionTcpSocket(int chunkSize=65536,int compression=6);
protected:
	virtual QByteArray real_compressData(const QByteArray &rawData);
	virtual QByteArray real_decompressData(const QByteArray &compressedData);
private:
	int QLZ4_uncompress_unknownOutputSize(QByteArray *source,QByteArray *destination,int maxOutputSize);
	QByteArray buffer_decompression_out;
};

#endif // LZ4COMPRESSIONTCPSOCKET_H

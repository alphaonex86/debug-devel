#ifndef ZLIBABSTRACTCOMPRESSIONTCPSOCKET_H
#define ZLIBABSTRACTCOMPRESSIONTCPSOCKET_H

#include <QBuffer>

#include "../CompressionTcpSocketInterface.h"
#include "qtiocompressor.h"

class ZlibAbstractCompressionTcpSocket : public CompressionTcpSocketInterface
{
public:
	ZlibAbstractCompressionTcpSocket(int bufferSize,int compression);
	~ZlibAbstractCompressionTcpSocket();
protected:
	virtual QByteArray compressData(const QByteArray rawData);
	virtual QByteArray decompressData(const QByteArray compressedData,const int &maxSize);
	virtual void abstractBlockingMethod() = 0;
	QtIOCompressor* decompressor;
	QBuffer* buffer_decompression;
	QByteArray buffer_decompression_out;
	QtIOCompressor* compressor;
	QBuffer* buffer_compression;
	QByteArray buffer_compression_out;
	int bufferSize;
	int compression;
};

#endif // ZLIBABSTRACTCOMPRESSIONTCPSOCKET_H

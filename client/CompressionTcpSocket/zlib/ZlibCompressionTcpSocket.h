#ifndef ZLIBCOMPRESSIONTCPSOCKET_H
#define ZLIBCOMPRESSIONTCPSOCKET_H

#include <QBuffer>

#include "../CompressionTcpSocketInterface.h"
#include "qtiocompressor.h"

class ZlibCompressionTcpSocket : public CompressionTcpSocketInterface
{
public:
	ZlibCompressionTcpSocket(int chunkSize=65536,int compression=6);
	~ZlibCompressionTcpSocket();
protected:
	virtual QByteArray real_compressData(const QByteArray &rawData);
	virtual QByteArray real_decompressData(const QByteArray &compressedData);
private:
	QtIOCompressor* decompressor;
	QBuffer* buffer_decompression;
	QByteArray buffer_decompression_out;
	QtIOCompressor* compressor;
	QBuffer* buffer_compression;
	QByteArray buffer_compression_out;
};

#endif // ZLIBCOMPRESSIONTCPSOCKET_H

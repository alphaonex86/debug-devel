#ifndef GZIPCOMPRESSIONTCPSOCKET_H
#define GZIPCOMPRESSIONTCPSOCKET_H

#include <QBuffer>

#include "../CompressionTcpSocketInterface.h"
#include "qtiocompressor.h"

class GzipCompressionTcpSocket : public CompressionTcpSocketInterface
{
public:
	GzipCompressionTcpSocket(int chunkSize=65536,int compression=6);
	~GzipCompressionTcpSocket();
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

#endif // GZIPCOMPRESSIONTCPSOCKET_H

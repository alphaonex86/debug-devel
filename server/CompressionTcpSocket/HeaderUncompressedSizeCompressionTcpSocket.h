#ifndef HEADERUNCOMPRESSEDSIZECOMPRESSIONTCPSOCKET_H
#define HEADERUNCOMPRESSEDSIZECOMPRESSIONTCPSOCKET_H

#include "CompressionTcpSocketInterface.h"

class HeaderUncompressedSizeCompressionTcpSocket : public CompressionTcpSocketInterface
{
public:
	HeaderUncompressedSizeCompressionTcpSocket();
	virtual QByteArray compressData(const QByteArray &rawData);
	virtual QByteArray decompressData(const QByteArray &compressedData,const int &maxSize);
protected:
	virtual QByteArray compressDataWithoutHeader(const QByteArray rawData) = 0;
	virtual bool decompressDataWithoutHeader(const QByteArray &source,QByteArray *destination,int *isize,int *osize) = 0;
	virtual int maxCompressedSize(const int &maxSize) = 0;
private:
	QByteArray buffer_decompression_out;

};

#endif // HEADERUNCOMPRESSEDSIZECOMPRESSIONTCPSOCKET_H

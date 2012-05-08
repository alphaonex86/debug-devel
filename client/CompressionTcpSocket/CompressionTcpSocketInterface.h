#ifndef COMPRESSIONTCPSOCKETINTERFACE_H
#define COMPRESSIONTCPSOCKETINTERFACE_H

#include <QByteArray>
#include <QString>
#include <QDebug>

class CompressionTcpSocketInterface
{
public:
	CompressionTcpSocketInterface();
	virtual QByteArray compressData(const QByteArray rawData) = 0;
	virtual QByteArray decompressData(const QByteArray compressedData,const int &maxSize=16777216) = 0;
	bool isInError();
	QString errorString();
protected:
	QString m_errorString;
	int correctTheBufferSize(int bufferSize);
	int correctTheCompression(int bufferSize);
};

#endif // COMPRESSIONTCPSOCKETINTERFACE_H

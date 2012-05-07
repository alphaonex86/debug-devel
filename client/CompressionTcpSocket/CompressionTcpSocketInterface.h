#ifndef COMPRESSIONTCPSOCKETINTERFACE_H
#define COMPRESSIONTCPSOCKETINTERFACE_H

#include <QByteArray>
#include <QString>
#include <QDebug>

class CompressionTcpSocketInterface
{
public:
	CompressionTcpSocketInterface(int chunkSize,int compression=6);
	QByteArray compressData(QByteArray rawData);
	QByteArray decompressData(QByteArray compressedData);
	bool isInError();
	QString errorString();
protected:
	virtual QByteArray real_compressData(const QByteArray &rawData) = 0;
	virtual QByteArray real_decompressData(const QByteArray &compressedData) = 0;
	QString m_errorString;
	int chunkSize;
};

#endif // COMPRESSIONTCPSOCKETINTERFACE_H

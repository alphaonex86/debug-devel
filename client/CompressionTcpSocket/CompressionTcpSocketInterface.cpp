#include "CompressionTcpSocketInterface.h"

CompressionTcpSocketInterface::CompressionTcpSocketInterface(int chunkSize,int compression)
{
	m_errorString.clear();
	if(chunkSize<512)
	{
		qDebug() << "chunk size to low, fixed at 512";
		chunkSize=512;
	}
	if(chunkSize>1024*1024)
	{
		qDebug() << "chunk size to big, fixed at 1MB";
		chunkSize=1048576;
	}
	if(compression<1)
	{
		qDebug() << "compression to low, fixed at 1";
		compression=1;
	}
	if(compression>9)
	{
		qDebug() << "compression to big, fixed at 9";
		compression=9;
	}
	this->chunkSize=chunkSize;
}

QByteArray CompressionTcpSocketInterface::compressData(QByteArray rawData)
{
	m_errorString.clear();
	QByteArray chunk,returnedData;
	do
	{
		if(rawData.size()<=chunkSize)
			chunk=rawData;
		else
			chunk=rawData.mid(0,chunkSize);
		rawData.remove(0,chunk.size());
		chunk=real_compressData(chunk);
		if(isInError())
			return QByteArray();
		//load in hexa if needed and append or update the text
		returnedData.append(chunk);
	} while(rawData.size()>0);
	return returnedData;
}

QByteArray CompressionTcpSocketInterface::decompressData(QByteArray compressedData)
{
	m_errorString.clear();
	QByteArray chunk,returnedData;
	do
	{
		if(compressedData.size()<=chunkSize)
			chunk=compressedData;
		else
			chunk=compressedData.mid(0,chunkSize);
		compressedData.remove(0,chunk.size());
		chunk=real_decompressData(chunk);
		if(isInError())
			return QByteArray();
		//load in hexa if needed and append or update the text
		returnedData.append(chunk);
	} while(compressedData.size()>0);
	return returnedData;
}

bool CompressionTcpSocketInterface::isInError()
{
	return !m_errorString.isEmpty();
}

QString CompressionTcpSocketInterface::errorString()
{
	if(m_errorString.isEmpty())
		return "No error";
	return m_errorString;
}

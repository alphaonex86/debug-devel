#include "HeaderUncompressedSizeCompressionTcpSocket.h"

HeaderUncompressedSizeCompressionTcpSocket::HeaderUncompressedSizeCompressionTcpSocket()
	: CompressionTcpSocketInterface()
{
}

QByteArray HeaderUncompressedSizeCompressionTcpSocket::compressData(const QByteArray &rawData)
{
	m_errorString="";
	QByteArray compressedData;
	if(rawData.size()>12)
		compressedData=compressDataWithoutHeader(rawData);
	else
		compressedData=rawData;
	QByteArray outputData;
	QDataStream out(&outputData, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_4);
	out << (quint32)(rawData.size());
	return outputData+compressedData;
}

QByteArray HeaderUncompressedSizeCompressionTcpSocket::decompressData(const QByteArray &compressedData,const int &maxSize)
{
	m_errorString="";
	QByteArray rawData,tempReturnedData;
	int uncompressedDataSize;
	int successLoop;
	buffer_decompression_out.append(compressedData);
	if(buffer_decompression_out.size()>(maxSize+sizeof(quint32)))
	{
		buffer_decompression_out.clear();
		m_errorString="buffer is more than maxSize";
		return QByteArray();
	}
	int input_size;
	successLoop=0;
	do
	{
		if((quint32)buffer_decompression_out.size()<=sizeof(quint32))
			break;
		QDataStream in(buffer_decompression_out);
		in.setVersion(QDataStream::Qt_4_4);
		in >> uncompressedDataSize;
		if(tempReturnedData.size()!=uncompressedDataSize)
			tempReturnedData.resize(uncompressedDataSize);

		qDebug() << QString("buffer_decompression_out.size(): %1").arg(buffer_decompression_out.size());
		buffer_decompression_out.remove(0,sizeof(quint32));

		input_size=buffer_decompression_out.size();
		if(uncompressedDataSize>12)
		{
			if(!decompressDataWithoutHeader(buffer_decompression_out,&tempReturnedData,&input_size,&uncompressedDataSize))
			{
				m_errorString=QString("Error at decompressing: %1").arg(uncompressedDataSize);
				break;
			}
			qDebug() << QString("input_size: %1, uncompressedDataSize: %2").arg(input_size).arg(uncompressedDataSize);
		}
		else
			tempReturnedData=buffer_decompression_out;
		buffer_decompression_out.remove(0,input_size);
		qDebug() << QString("after: buffer_decompression_out.size(): %1").arg(buffer_decompression_out.size());
		rawData+=tempReturnedData;
		successLoop++;
	} while((quint32)buffer_decompression_out.size()>sizeof(quint32));
	return rawData;
}

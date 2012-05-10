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
	QByteArray rawData,chunk,tempReturnedData;
	int returnSize,uncompressedDataSize;
	int successLoop;
	buffer_decompression_out.append(compressedData);
	if(buffer_decompression_out.size()>(maxSize+sizeof(quint32)))
	{
		buffer_decompression_out.clear();
		m_errorString="buffer is more than maxSize";
		return QByteArray();
	}
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

		buffer_decompression_out.remove(0,sizeof(quint32));

		if(uncompressedDataSize>12)
		{
			returnSize=decompressDataWithoutHeader(buffer_decompression_out,&tempReturnedData);
			if(returnSize<=0)
			{
				m_errorString=QString("Error at decompressing: %1").arg(returnSize);
				break;
			}
		}
		else
			tempReturnedData=buffer_decompression_out;
		buffer_decompression_out.remove(0,tempReturnedData.size());
		rawData+=tempReturnedData;
		successLoop++;
	} while((quint32)buffer_decompression_out.size()>sizeof(quint32));
	return rawData;
}

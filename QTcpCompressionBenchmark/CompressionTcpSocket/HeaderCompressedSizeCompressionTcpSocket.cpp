#include "HeaderCompressedSizeCompressionTcpSocket.h"

HeaderCompressedSizeCompressionTcpSocket::HeaderCompressedSizeCompressionTcpSocket()
	: CompressionTcpSocketInterface()
{
}

QByteArray HeaderCompressedSizeCompressionTcpSocket::compressData(const QByteArray &rawData)
{
	m_errorString="";
	QByteArray compressedData=compressDataWithoutHeader(rawData);
	QByteArray outputData;
	QDataStream out(&outputData, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_4);
	out << (quint32)(compressedData.size()+sizeof(qint32));
	return outputData+compressedData;
}

QByteArray HeaderCompressedSizeCompressionTcpSocket::decompressData(const QByteArray &compressedData,const int &maxSize)
{
	m_errorString="";
	QByteArray rawData,chunk,tempReturnedData;
	int compressedDataSize;
	int successLoop;
	buffer_decompression_out.append(compressedData);
	if(buffer_decompression_out.size()>(maxCompressedSize(maxSize)+sizeof(quint32)))
	{
		buffer_decompression_out.clear();
		m_errorString="buffer is more than maxSize";
		return QByteArray();
	}
	int input_size,output_size;
	successLoop=0;
	do
	{
		if((quint32)buffer_decompression_out.size()<=sizeof(quint32))
			break;
		QDataStream in(buffer_decompression_out);
		in.setVersion(QDataStream::Qt_4_4);
		in >> compressedDataSize;

		if((quint32)buffer_decompression_out.size()<compressedDataSize)
		{
			qDebug() << QString("to short, compressedDataSize: %1, buffer_decompression_out.size(): %2").arg(compressedDataSize).arg(buffer_decompression_out.size());
			break;
		}
		buffer_decompression_out.remove(0,sizeof(quint32));
		compressedDataSize-=sizeof(quint32);
		if(compressedDataSize>(maxCompressedSize(maxSize)))
		{
			buffer_decompression_out.remove(0,compressedDataSize);
			m_errorString="chunk is more than maxSize";
			continue;
		}
		chunk=buffer_decompression_out.mid(0,compressedDataSize);
		buffer_decompression_out.remove(0,chunk.size());

		output_size=maxSize;
		input_size=chunk.size();
		if(!decompressDataWithoutHeader(chunk,&tempReturnedData,&input_size,&output_size) || input_size!=chunk.size())
		{
			m_errorString=QString("Error at decompressing: %1").arg(input_size);
			break;
		}
		/// \bug crash here, I don't see why
		tempReturnedData.resize(output_size);
		rawData+=tempReturnedData;
		successLoop++;
	} while((quint32)buffer_decompression_out.size()>sizeof(quint32));
	return rawData;
}

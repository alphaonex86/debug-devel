#include "ZlibAbstractCompressionTcpSocket.h"

ZlibAbstractCompressionTcpSocket::ZlibAbstractCompressionTcpSocket(int bufferSize,int compression,bool byPacket)
	: CompressionTcpSocketInterface()
{
	this->bufferSize=correctTheBufferSize(bufferSize);
	this->compression=correctTheCompression(compression);
	decompressor=NULL;
	buffer_decompression=new QBuffer(&buffer_decompression_out);
	compressor=NULL;
	buffer_compression=new QBuffer(&buffer_compression_out);
	this->byPacket=byPacket;
}

ZlibAbstractCompressionTcpSocket::~ZlibAbstractCompressionTcpSocket()
{
	delete decompressor;
	delete compressor;
}

QByteArray ZlibAbstractCompressionTcpSocket::compressData(const QByteArray &rawData)
{
	m_errorString="";
	QByteArray compressedData;
	compressor->write(rawData);
	if(!byPacket)
		compressor->flush(QtIOCompressor::StreamFlushPartial);
	else
	{
		compressor->close();
		compressor->open(QIODevice::WriteOnly);
	}
	compressedData=buffer_compression_out;
	buffer_compression->seek(0);
	buffer_compression_out.resize(0);
	return compressedData;
}

QByteArray ZlibAbstractCompressionTcpSocket::decompressData(const QByteArray &compressedData,const int &maxSize)
{
	m_errorString="";
	Q_UNUSED(maxSize)
	buffer_decompression_out=compressedData;
	buffer_decompression->seek(0);
	QByteArray rawData=decompressor->readAll();
	if(byPacket)
	{
		decompressor->close();
		decompressor->open(QIODevice::ReadOnly);
	}
	return rawData;
	if(rawData.size()>0)
	{
		buffer_decompression->seek(0);
		buffer_decompression_out.resize(0);
		return rawData;
	}
	else
	{
		m_errorString=decompressor->errorString();
		return QByteArray();
	}
}

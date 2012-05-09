#include "ZlibAbstractCompressionTcpSocket.h"

ZlibAbstractCompressionTcpSocket::ZlibAbstractCompressionTcpSocket(int bufferSize,int compression)
	: CompressionTcpSocketInterface()
{
	this->bufferSize=correctTheBufferSize(bufferSize);
	this->compression=correctTheCompression(compression);
	decompressor=NULL;
	buffer_decompression=new QBuffer(&buffer_decompression_out);
	compressor=NULL;
	buffer_compression=new QBuffer(&buffer_compression_out);
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
	//compressor->flush();//add big overhead, edited mode to Z_PARTIAL_FLUSH
	compressor->close();
	compressor->open(QIODevice::WriteOnly);
	compressedData=buffer_compression_out;
	buffer_compression->seek(0);
	buffer_compression_out.resize(0);
	return compressedData;
}

QByteArray ZlibAbstractCompressionTcpSocket::decompressData(const QByteArray &compressedData,const int &maxSize)
{
	m_errorString="";
	Q_UNUSED(maxSize)
	buffer_decompression_out.resize(buffer_decompression_out.size()+maxSize);
	//buffer_decompression->seek(0);
	buffer_decompression_out=compressedData;
	QByteArray rawData=decompressor->readAll();
	decompressor->close();
	decompressor->open(QIODevice::ReadOnly);
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

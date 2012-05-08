#include "ZlibAbstractCompressionTcpSocket.h"

ZlibAbstractCompressionTcpSocket::ZlibAbstractCompressionTcpSocket(int bufferSize,int compression)
	: CompressionTcpSocketInterface()
{
	this->bufferSize=correctTheBufferSize(bufferSize);
	this->compression=correctTheCompression(compression);
	decompressor=NULL;
	buffer_decompression=NULL;
	compressor=NULL;
	buffer_compression=NULL;
}

ZlibAbstractCompressionTcpSocket::~ZlibAbstractCompressionTcpSocket()
{
	delete decompressor;
	delete compressor;
}

QByteArray ZlibAbstractCompressionTcpSocket::compressData(const QByteArray rawData)
{
	QByteArray compressedData;
	compressor->write(rawData);
	compressor->flush();//add big overhead, edited mode to Z_PARTIAL_FLUSH
	compressedData=buffer_compression_out;
	buffer_compression->seek(0);
	buffer_compression_out.resize(0);
	return compressedData;
}

QByteArray ZlibAbstractCompressionTcpSocket::decompressData(const QByteArray compressedData,const int &maxSize)
{
	Q_UNUSED(maxSize)
	buffer_decompression->seek(0);
	buffer_decompression_out=compressedData;
	QByteArray rawData=decompressor->readAll();
	if(rawData.size()>0)
	{
		buffer_decompression->seek(0);
		buffer_decompression_out.resize(0);
		return rawData;
	}
	else
	{
		m_errorString=QString("Error at decompressing: %1").arg(decompressor->errorString());
		return QByteArray();
	}
}

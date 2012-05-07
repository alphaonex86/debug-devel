#include "ZlibCompressionTcpSocket.h"

ZlibCompressionTcpSocket::ZlibCompressionTcpSocket(int chunkSize,int compression)
	: CompressionTcpSocketInterface(chunkSize,compression)
{
	compressor=new QtIOCompressor(buffer_compression,9,chunkSize);
	compressor->setStreamFormat(QtIOCompressor::ZlibFormat);
	compressor->open(QIODevice::WriteOnly);

	decompressor=new QtIOCompressor(buffer_decompression);
	decompressor->setStreamFormat(QtIOCompressor::ZlibFormat);
	decompressor->open(QIODevice::ReadOnly);
}

ZlibCompressionTcpSocket::~ZlibCompressionTcpSocket()
{
	delete decompressor;
	delete compressor;
}

QByteArray ZlibCompressionTcpSocket::real_compressData(const QByteArray &rawData)
{
	QByteArray compressedData;
	compressor->write(rawData);
	compressor->flush();//add big overhead, edited mode to Z_PARTIAL_FLUSH
	compressedData=buffer_compression_out;
	buffer_compression->seek(0);
	buffer_compression_out.resize(0);
	return compressedData;
}

QByteArray ZlibCompressionTcpSocket::real_decompressData(const QByteArray &compressedData)
{
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

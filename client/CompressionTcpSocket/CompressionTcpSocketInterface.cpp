#include "CompressionTcpSocketInterface.h"

CompressionTcpSocketInterface::CompressionTcpSocketInterface()
{
	m_errorString.clear();
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

int CompressionTcpSocketInterface::correctTheBufferSize(int bufferSize)
{
	if(bufferSize<512)
	{
		qDebug() << "chunk size to low, fixed at 512";
		bufferSize=512;
	}
	if(bufferSize>16*1024*1024)
	{
		qDebug() << "chunk size to big, fixed at 16MB";
		bufferSize=16*1024*1024;
	}
	return bufferSize;
}

int CompressionTcpSocketInterface::correctTheCompression(int compression)
{
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
	return compression;
}

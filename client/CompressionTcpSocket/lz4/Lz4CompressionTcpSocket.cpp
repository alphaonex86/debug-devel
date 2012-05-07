#include "Lz4CompressionTcpSocket.h"

Lz4CompressionTcpSocket::Lz4CompressionTcpSocket(int chunkSize,int compression)
	: CompressionTcpSocketInterface(chunkSize,compression)
{
}

QByteArray Lz4CompressionTcpSocket::real_compressData(const QByteArray &rawData)
{
	QByteArray compressedData;
	int returnSize;
	compressedData.resize(LZ4_compressBound(rawData.size()));
	returnSize=LZ4_compress(rawData.constData(),compressedData.data(),rawData.size());
	compressedData.resize(returnSize);
	return compressedData;
}

QByteArray Lz4CompressionTcpSocket::real_decompressData(const QByteArray &compressedData)
{
	QByteArray rawData,chunk_data;
	int returnSize;
	int successLoop;
	chunk_data.resize(chunkSize);
	buffer_decompression_out.append(compressedData);
	successLoop=0;
	do
	{
		returnSize=QLZ4_uncompress_unknownOutputSize(&buffer_decompression_out,&chunk_data,chunkSize);
		if(buffer_decompression_out.size()>LZ4_compressBound(chunkSize))
		{
			m_errorString=QString("Error at decompressing, and max size matched: %1").arg(buffer_decompression_out.size());
			return QByteArray();
		}
		if(returnSize==0 || returnSize==-2 || returnSize==-4)
			break;
		if(returnSize<=0)
		{
			m_errorString=QString("Error at decompressing: %1").arg(returnSize);
			return QByteArray();
		}
		chunk_data.resize(returnSize);
		rawData+=chunk_data;
		successLoop++;
	} while(returnSize>0 && buffer_decompression_out.size()>0);
	return rawData;
}

//**************************************
// Constants
//**************************************
#define COPYLENGTH 8
#define ML_BITS 4
#define ML_MASK ((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)

//**************************************
// Basic Types
//**************************************
typedef struct _U16_S { quint16 v; } U16_S;
typedef struct _U32_S { quint32 v; } U32_S;
typedef struct _U64_S { quint64 v; } U64_S;

#define A64(x) (((U64_S *)(x))->v)
#define A32(x) (((U32_S *)(x))->v)
#define A16(x) (((U16_S *)(x))->v)

//**************************************
// Architecture-specific macros
//**************************************
#if LZ4_ARCH64	// 64-bit
#define STEPSIZE 8
#define UARCH U64
#define AARCH A64
#define LZ4_COPYSTEP(s,d)		A64(d) = A64(s); d+=8; s+=8;
#define LZ4_COPYPACKET(s,d)		LZ4_COPYSTEP(s,d)
#define LZ4_SECURECOPY(s,d,e)	if (d<e) LZ4_WILDCOPY(s,d,e)
#define HTYPE U32
#define INITBASE(base)			const quint8* const base = ip
#else		// 32-bit
#define STEPSIZE 4
#define UARCH U32
#define AARCH A32
#define LZ4_COPYSTEP(s,d)		A32(d) = A32(s); d+=4; s+=4;
#define LZ4_COPYPACKET(s,d)		LZ4_COPYSTEP(s,d); LZ4_COPYSTEP(s,d);
#define LZ4_SECURECOPY			LZ4_WILDCOPY
#define HTYPE const quint8*
#define INITBASE(base)			const int base = 0
#endif

//**************************************
// Macros
//**************************************
#define LZ4_WILDCOPY(s,d,e)		do { LZ4_COPYPACKET(s,d) } while (d<e);

#if (defined(LZ4_BIG_ENDIAN) && !defined(BIG_ENDIAN_NATIVE_BUT_INCOMPATIBLE))
#define LZ4_READ_LITTLEENDIAN_16(d,s,p) { U16 v = A16(p); v = bswap16(v); d = (s) - v; }
#define LZ4_WRITE_LITTLEENDIAN_16(p,i) { U16 v = (U16)(i); v = bswap16(v); A16(p) = v; p+=2; }
#else		// Little Endian
#define LZ4_READ_LITTLEENDIAN_16(d,s,p) { d = (s) - A16(p); }
#define LZ4_WRITE_LITTLEENDIAN_16(p,v) { A16(p) = v; p+=2; }
#endif

int Lz4CompressionTcpSocket::QLZ4_uncompress_unknownOutputSize(QByteArray *source,
				QByteArray *destination,
				int maxOutputSize)
{
	// Local Variables
	const quint8* ip = (const quint8*) source->data();
	const quint8* const iend = ip + source->size();
	const quint8* ref;

	quint8* op = (quint8*) destination->data();
	quint8* base_op = (quint8*) destination->data();
	quint8* const oend = op + maxOutputSize;
	quint8* cpy;

	size_t dec[] ={0, 3, 2, 3, 0, 0, 0, 0};

	quint8 token;
	int length;

	// Main Loop
	while (ip<iend)
	{
		// get runlength
		token = *ip++;
		if ((length=(token>>ML_BITS)) == RUN_MASK) { int s=255; while ((ip<iend) && (s==255)) { s=*ip++; length += s; } }

		// copy literals
		cpy = op+length;
		if ((cpy>oend-COPYLENGTH) || (ip+length>iend-COPYLENGTH))
		{
			if (cpy > oend)
				return -1;
			if (ip+length > iend)
				return -2;
			memcpy(op, ip, length);
			op += length;
			ip += length;
			//the remaining copy is of other packet
			/*if (ip<iend)
				return -3;*/
			break;    // Necessarily EOF, due to parsing restrictions
		}
		do { LZ4_COPYPACKET(ip,op) } while (op<cpy);
		ip -= (op-cpy); op = cpy;

		// get offset
		LZ4_READ_LITTLEENDIAN_16(ref,cpy,ip); ip+=2;
		if (ref<base_op)
			return -4;

		// get matchlength
		if ((length=(token&ML_MASK)) == ML_MASK) { while (ip<iend) { int s = *ip++; length +=s; if (s==255) continue; break; } }

		// copy repeated sequence
		if(__builtin_expect (((op-ref<STEPSIZE) != 0),0))
				//(expect(, 0))
		{
#if LZ4_ARCH64
			size_t dec2table[]={0, 0, 0, -1, 0, 1, 2, 3};
			size_t dec2 = dec2table[op-ref];
#else
			const int dec2 = 0;
#endif
			*op++ = *ref++;
			*op++ = *ref++;
			*op++ = *ref++;
			*op++ = *ref++;
			ref -= dec[op-ref];
			A32(op)=A32(ref); op += STEPSIZE-4;
			ref -= dec2;
		} else { LZ4_COPYSTEP(ref,op); }
		cpy = op + length - (STEPSIZE-4);
		if (cpy>oend-COPYLENGTH)
		{
			if (cpy > oend)
				return -5;
			LZ4_SECURECOPY(ref, op, (oend-COPYLENGTH));
			while(op<cpy) *op++=*ref++;
			op=cpy;
			if (op == oend) break;    // Check EOF (should never happen, since last 5 bytes are supposed to be literals)
			continue;
		}
		LZ4_SECURECOPY(ref, op, cpy);
		op=cpy;		// correction
	}

	// end of decoding
	destination->resize((int)(op-base_op));
	source->remove(0,(int)(ip-(const quint8*)source->data()));
	return destination->size();
}


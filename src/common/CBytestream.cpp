/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Byte stream class
// Created 13/10/01
// Jason Boettcher


#include "defs.h"




///////////////////
// Append another bytestream onto this one
void CBytestream::Append(CBytestream *bs)
{
	// Check to make sure we don't overflow the buffer
	if(CurByte + bs->GetLength() >= MAX_DATA) {
		d_printf("byte stream is too big to append any extra data\n");
		return;
	}

	memcpy(Data+CurByte,bs->GetData(),bs->GetLength());
	CurByte+=bs->GetLength();
	Length+=bs->GetLength();
}


///////////////////
// Dump the data out
void CBytestream::Dump(void)
{
    char buf[MAX_DATA+1];

    memcpy(buf, Data, Length);
    buf[Length] = '\0';

    fwrite(buf,1,Length,stdout);

}


///////////////////
// Append some data onto this bytestream
void CBytestream::AppendData(char *_data, int _length)
{
	// Check to make sure we don't overflow the buffer
	if(CurByte + _length >= MAX_DATA)
		return;

	memcpy(Data+CurByte,_data,_length);
	CurByte += _length;
	Length += _length;
}



// Writes


///////////////////
// Writes a single byte
int CBytestream::writeByte(uchar byte)
{
	if(CurByte >= MAX_DATA)
		return false;

	Data[CurByte] = byte;
	CurByte++;
	Length++;
	return true;
}


///////////////////
// Writes a boolean value to the stream
int CBytestream::writeBool(bool value)
{
	return writeByte((uchar)value);
}


///////////////////
// Writes a signed short
/*void CBytestream::writeSShort(short value)
{


}*/


///////////////////
// Writes an integer to the stream
int CBytestream::writeInt(int value, int numbytes)
{
	uchar bytes[4];
	int n;

	// Numbytes cannot be more then 4
	if(numbytes <= 0 || numbytes >= 5)
		return false;
	
	// Copy the interger into individual bytes
	// HINT: this is endian independent
	bytes[0] = (uint)value & 0xff;
	bytes[1] = ((uint)value & 0xff00) / 0x100;
	bytes[2] = ((uint)value & 0xff0000) / 0x10000;
	bytes[3] = ((uint)value & 0xff000000) / 0x1000000;

	for(n=0;n<numbytes;n++)
		if(!writeByte(bytes[n]))
			return false;

	return true;
}


///////////////////
// Write a short to the stream
int CBytestream::writeShort(short value)
{
	return writeInt(value,2);
}


///////////////////
// Writes a float to the stream
int CBytestream::writeFloat(float value)
{
	// TODO: how to do this only for the precompiler?
	assert( sizeof(float) == 4 );

	union {
		uchar bin[sizeof(float)];
		float val;
	} tmp;
	
	tmp.val = value;
	BEndianSwap(tmp.bin);
	for(short i = 0; i < sizeof(float); i++)
		if(!writeByte(tmp.bin[i]))
			return false;
			
	return true;
}


///////////////////
// Write a string to the stream
int CBytestream::writeString(char *fmt,...)
{
	char buf[1024];
	va_list	va;

	va_start(va,fmt);
	vsprintf(buf,fmt,va);
	va_end(va);

	int len = strlen(buf);

	if(len+CurByte >= MAX_DATA)
		return false;

	strcpy((char *)Data+CurByte,buf);
	CurByte+=len+1;
	Length+=len+1;

	return true;
}

// cast 2 int12 to 3 bytes
int	CBytestream::write2Int12(short x, short y) {
	if(!writeByte(x & 0xff)) return false;
	if(!writeByte(((x & 0xf00) / 0x100) + (y & 0xf) * 0x10)) return false;
	if(!writeByte((y & 0xff0) / 0x10)) return false;
	return true;
}

// cast 2 int4 to 1 byte
int CBytestream::write2Int4(short x, short y) {
	return writeByte((x & 0xf) + (y & 0xf) * 0x10);
}




// Reads




///////////////////
// Reads a single byte
uchar CBytestream::readByte(void)
{
	if(CurByte >= MAX_DATA)
		return 0;

	return Data[CurByte++];
}


///////////////////
// Reads a boolean value from the stream
bool CBytestream::readBool(void)
{
	return readByte() != 0;
}


///////////////////
// Reads an interger value from the stream
int CBytestream::readInt(int numbytes)
{
	uchar bytes[4];
	int n;
	int value;

	// Numbytes cannot be more than 4
	if(numbytes <= 0 || numbytes >= 5)
		return false;

	for(n=0;n<numbytes;n++)
		bytes[n] = readByte();

	uint ret = 0;
	
	// HINT: this is endian independent
	if(numbytes>0)
		ret = (uint)bytes[0];
	if(numbytes>1)
		ret += (uint)bytes[1] * 0x100;
	if(numbytes>2)
		ret += (uint)bytes[2] * 0x10000;
	if(numbytes>3)
		ret += (uint)bytes[3] * 0x1000000;
	value = (int)ret;
	
	return value;
}


///////////////////
// Read a short from the stream
short CBytestream::readShort(void)
{
	return (short)readInt(2);
}



///////////////////
// Read a float value from the stream
float CBytestream::readFloat(void)
{
	// TODO: how to do this only for the precompiler?
	assert( sizeof(float) == 4 );

	union {
		uchar bin[sizeof(float)];
		float val;
	} tmp;
	
	for(short i = 0; i < sizeof(float); i++)
		tmp.bin[i] = readByte();
	BEndianSwap(tmp.bin);
	return tmp.val;
}


///////////////////
// Read a string from the stream
char *CBytestream::readString(char *str)
{
	bool valid = false;
	for (int i=CurByte; i<GetLength(); i++)
		if(Data[i] == '\0')
			valid = true;

	if (!valid)
		return "";

	strcpy(str,(char *)Data+CurByte);
	CurByte += strlen(str)+1;

	return str;
}


// cast 3 bytes to 2 int12
void CBytestream::read2Int12(short& x, short& y) {
	short dat[3];
	dat[0] = readByte();
	dat[1] = readByte();
	dat[2] = readByte();

	x = dat[0] + ((dat[1] & 0xf) * 0x100);
	y = ((dat[1] & 0xf0) / 0x10) + dat[2] * 0x10;
}

// cast 1 byte to 2 int4
void CBytestream::read2Int4(short& x, short& y) {
	uchar tmp = readByte();
	x = tmp & 0xf;
	y = (tmp & 0xf0) / 0x10;
}

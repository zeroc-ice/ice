// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/StreamI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::StreamI::StreamI(const InstancePtr& instance) :
    _stream(instance)
{
}

void
Ice::StreamI::resize(Int len)
{
    _stream.resize(len);
}

void
Ice::StreamI::reserve(Int len)
{
    _stream.reserve(len);
}

void
Ice::StreamI::startWriteEncaps()
{
    _stream.startWriteEncaps();
}

void
Ice::StreamI::endWriteEncaps()
{
    _stream.endWriteEncaps();
}

void
Ice::StreamI::startReadEncaps()
{
    _stream.startReadEncaps();
}

void
Ice::StreamI::endReadEncaps()
{
    _stream.endReadEncaps();
}

void
Ice::StreamI::skipEncaps()
{
    _stream.skipEncaps();
}

void
Ice::StreamI::writeByte(Byte value)
{
    _stream.write(value);
}

void
Ice::StreamI::writeByteSeq(const ByteSeq& value)
{
    _stream.write(value);
}

void
Ice::StreamI::readByte(Byte& value)
{
    _stream.read(value);
}

void
Ice::StreamI::readByteSeq(ByteSeq& value)
{
    _stream.read(value);
}

void
Ice::StreamI::writeBool(bool value)
{
    _stream.write(value);
}

void
Ice::StreamI::writeBoolSeq(const BoolSeq& value)
{
    _stream.write(value);
}

void
Ice::StreamI::readBool(bool& value)
{
    _stream.read(value);
}

void
Ice::StreamI::readBoolSeq(BoolSeq& value)
{
    _stream.read(value);
}

void
Ice::StreamI::writeShort(Short value)
{
    _stream.write(value);
}

void
Ice::StreamI::writeShortSeq(const ShortSeq& value)
{
    _stream.write(value);
}

void
Ice::StreamI::readShort(Short& value)
{
    _stream.read(value);
}

void
Ice::StreamI::readShortSeq(ShortSeq& value)
{
    _stream.read(value);
}

void
Ice::StreamI::writeInt(Int value)
{
    _stream.write(value);
}

void
Ice::StreamI::writeIntSeq(const IntSeq& value)
{
    _stream.write(value);
}

void
Ice::StreamI::readInt(Int& value)
{
    _stream.read(value);
}

void
Ice::StreamI::readIntSeq(IntSeq& value)
{
    _stream.read(value);
}

void
Ice::StreamI::writeLong(Long value)
{
    _stream.write(value);
}

void
Ice::StreamI::writeLongSeq(const LongSeq& value)
{
    _stream.write(value);
}

void
Ice::StreamI::readLong(Long& value)
{
    _stream.read(value);
}

void
Ice::StreamI::readLongSeq(LongSeq& value)
{
    _stream.read(value);
}

void
Ice::StreamI::writeFloat(Float value)
{
    _stream.write(value);
}

void
Ice::StreamI::writeFloatSeq(const FloatSeq& value)
{
    _stream.write(value);
}

void
Ice::StreamI::readFloat(Float& value)
{
    _stream.read(value);
}

void
Ice::StreamI::readFloatSeq(FloatSeq& value)
{
    _stream.read(value);
}

void
Ice::StreamI::writeDouble(Double value)
{
    _stream.write(value);
}

void
Ice::StreamI::writeDoubleSeq(const DoubleSeq& value)
{
    _stream.write(value);
}

void
Ice::StreamI::readDouble(Double& value)
{
    _stream.read(value);
}

void
Ice::StreamI::readDoubleSeq(DoubleSeq& value)
{
    _stream.read(value);
}

void
Ice::StreamI::writeString(const string& value)
{
    _stream.write(value);
}

void
Ice::StreamI::writeStringSeq(const StringSeq& value)
{
    _stream.write(value);
}

void
Ice::StreamI::readString(string& value)
{
    _stream.read(value);
}

void
Ice::StreamI::readStringSeq(StringSeq& value)
{
    _stream.read(value);
}

void
Ice::StreamI::writeWString(const wstring& value)
{
    _stream.write(value);
}

void
Ice::StreamI::writeWStringSeq(const WStringSeq& value)
{
    _stream.write(value);
}

void
Ice::StreamI::readWString(wstring& value)
{
    _stream.read(value);
}

void
Ice::StreamI::readWStringSeq(WStringSeq& value)
{
    _stream.read(value);
}

void
Ice::StreamI::writeProxy(const ObjectPrx& value)
{
    _stream.write(value);
}

void
Ice::StreamI::readProxy(ObjectPrx& value)
{
    _stream.read(value);
}

void
Ice::StreamI::writeObject(const ObjectPtr& value)
{
    _stream.write(value);
}

void
Ice::StreamI::readObject(const string& signature, ObjectPtr& value)
{
    _stream.read(value, signature.c_str());
}

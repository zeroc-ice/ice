// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STREAM_I_H
#define ICE_STREAM_I_H

#include <Ice/Stream.h>
#include <Ice/BasicStream.h>

namespace Ice
{

class StreamI : public Stream
{
public:

    StreamI(const ::IceInternal::InstancePtr&);

    virtual void resize(Int);
    virtual void reserve(Int);
    virtual void startWriteEncaps();
    virtual void endWriteEncaps();
    virtual void startReadEncaps();
    virtual void endReadEncaps();
    virtual void skipEncaps();
    virtual void writeByte(Byte);
    virtual void writeByteSeq(const ByteSeq&);
    virtual void readByte(Byte&);
    virtual void readByteSeq(ByteSeq&);
    virtual void writeBool(bool);
    virtual void writeBoolSeq(const BoolSeq&);
    virtual void readBool(bool&);
    virtual void readBoolSeq(BoolSeq&);
    virtual void writeShort(Short);
    virtual void writeShortSeq(const ShortSeq&);
    virtual void readShort(Short&);
    virtual void readShortSeq(ShortSeq&);
    virtual void writeInt(Int);
    virtual void writeIntSeq(const IntSeq&);
    virtual void readInt(Int&);
    virtual void readIntSeq(IntSeq&);
    virtual void writeLong(Long);
    virtual void writeLongSeq(const LongSeq&);
    virtual void readLong(Long&);
    virtual void readLongSeq(LongSeq&);
    virtual void writeFloat(Float);
    virtual void writeFloatSeq(const FloatSeq&);
    virtual void readFloat(Float&);
    virtual void readFloatSeq(FloatSeq&);
    virtual void writeDouble(Double);
    virtual void writeDoubleSeq(const DoubleSeq&);
    virtual void readDouble(Double&);
    virtual void readDoubleSeq(DoubleSeq&);
    virtual void writeString(const ::std::string&);
    virtual void writeStringSeq(const StringSeq&);
    virtual void readString(::std::string&);
    virtual void readStringSeq(StringSeq&);
    virtual void writeWString(const ::std::wstring&);
    virtual void writeWStringSeq(const WStringSeq&);
    virtual void readWString(::std::wstring&);
    virtual void readWStringSeq(WStringSeq&);
    virtual void writeProxy(const ObjectPrx&);
    virtual void readProxy(ObjectPrx&);
    virtual void writeObject(const ObjectPtr&);
    virtual void readObject(const ::std::string&, ObjectPtr&);

private:

    ::IceInternal::BasicStream _stream;
};

}

#endif

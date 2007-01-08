// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STREAM_H
#define ICE_STREAM_H

#include <Ice/StreamF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Object.h>
#include <Ice/Exception.h>
#include <IceUtil/Shared.h>

namespace Ice
{

class ICE_API ReadObjectCallback : public ::IceUtil::Shared
{
public:

    virtual void invoke(const ::Ice::ObjectPtr&) = 0;
};
typedef IceUtil::Handle< ReadObjectCallback > ReadObjectCallbackPtr;

class ICE_API InputStream : public ::IceUtil::Shared
{
public:

    virtual Ice::CommunicatorPtr communicator() const = 0;

    virtual void sliceObjects(bool) = 0;

    virtual bool readBool() = 0;
    virtual ::std::vector< bool > readBoolSeq() = 0;
    virtual bool* readBoolSeq(::std::pair<const bool*, const bool*>&) = 0;

    virtual ::Ice::Byte readByte() = 0;
    virtual ::std::vector< ::Ice::Byte > readByteSeq() = 0;
    virtual void readByteSeq(::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&) = 0;

    virtual ::Ice::Short readShort() = 0;
    virtual ::std::vector< ::Ice::Short > readShortSeq() = 0;
    virtual ::Ice::Short* readShortSeq(::std::pair<const ::Ice::Short*, const ::Ice::Short*>&) = 0;

    virtual ::Ice::Int readInt() = 0;
    virtual ::std::vector< ::Ice::Int > readIntSeq() = 0;
    virtual ::Ice::Int* readIntSeq(::std::pair<const ::Ice::Int*, const ::Ice::Int*>&) = 0;

    virtual ::Ice::Long readLong() = 0;
    virtual ::std::vector< ::Ice::Long > readLongSeq() = 0;
    virtual ::Ice::Long* readLongSeq(::std::pair<const ::Ice::Long*, const ::Ice::Long*>&) = 0;

    virtual ::Ice::Float readFloat() = 0;
    virtual ::std::vector< ::Ice::Float > readFloatSeq() = 0;
    virtual ::Ice::Float* readFloatSeq(::std::pair<const ::Ice::Float*, const ::Ice::Float*>&) = 0;

    virtual ::Ice::Double readDouble() = 0;
    virtual ::std::vector< ::Ice::Double > readDoubleSeq() = 0;
    virtual ::Ice::Double* readDoubleSeq(::std::pair<const ::Ice::Double*, const ::Ice::Double*>&) = 0;

    virtual ::std::string readString() = 0;
    virtual ::std::vector< ::std::string > readStringSeq() = 0;

    virtual ::std::wstring readWstring() = 0;
    virtual ::std::vector< ::std::wstring > readWstringSeq() = 0;

    virtual ::Ice::Int readSize() = 0;

    virtual ::Ice::ObjectPrx readProxy() = 0;

    virtual void readObject(const ::Ice::ReadObjectCallbackPtr&) = 0;

    virtual ::std::string readTypeId() = 0;

    virtual void throwException() = 0;

    virtual void startSlice() = 0;
    virtual void endSlice() = 0;
    virtual void skipSlice() = 0;

    virtual void startEncapsulation() = 0;
    virtual void endEncapsulation() = 0;

    virtual void readPendingObjects() = 0;
};

class ICE_API OutputStream : public ::IceUtil::Shared
{
public:

    virtual Ice::CommunicatorPtr communicator() const = 0;

    virtual void writeBool(bool) = 0;
    virtual void writeBoolSeq(const ::std::vector< bool >&) = 0;
    virtual void writeBoolSeq(const bool*, const bool*) = 0;

    virtual void writeByte(::Ice::Byte) = 0;
    virtual void writeByteSeq(const ::std::vector< ::Ice::Byte >&) = 0;
    virtual void writeByteSeq(const Ice::Byte*, const Ice::Byte*) = 0;

    virtual void writeShort(::Ice::Short) = 0;
    virtual void writeShortSeq(const ::std::vector< ::Ice::Short >&) = 0;
    virtual void writeShortSeq(const Ice::Short*, const Ice::Short*) = 0;

    virtual void writeInt(::Ice::Int) = 0;
    virtual void writeIntSeq(const ::std::vector< ::Ice::Int >&) = 0;
    virtual void writeIntSeq(const Ice::Int*, const Ice::Int*) = 0;

    virtual void writeLong(::Ice::Long) = 0;
    virtual void writeLongSeq(const ::std::vector< ::Ice::Long >&) = 0;
    virtual void writeLongSeq(const Ice::Long*, const Ice::Long*) = 0;

    virtual void writeFloat(::Ice::Float) = 0;
    virtual void writeFloatSeq(const ::std::vector< ::Ice::Float >&) = 0;
    virtual void writeFloatSeq(const Ice::Float*, const Ice::Float*) = 0;

    virtual void writeDouble(::Ice::Double) = 0;
    virtual void writeDoubleSeq(const ::std::vector< ::Ice::Double >&) = 0;
    virtual void writeDoubleSeq(const Ice::Double*, const Ice::Double*) = 0;

    virtual void writeString(const ::std::string&) = 0;
    virtual void writeStringSeq(const ::std::vector< ::std::string >&) = 0;

    virtual void writeWstring(const ::std::wstring&) = 0;
    virtual void writeWstringSeq(const ::std::vector< ::std::wstring >&) = 0;

    virtual void writeSize(::Ice::Int) = 0;

    virtual void writeProxy(const ::Ice::ObjectPrx&) = 0;

    virtual void writeObject(const ::Ice::ObjectPtr&) = 0;

    virtual void writeTypeId(const ::std::string&) = 0;

    virtual void writeException(const ::Ice::UserException&) = 0;

    virtual void startSlice() = 0;
    virtual void endSlice() = 0;

    virtual void startEncapsulation() = 0;
    virtual void endEncapsulation() = 0;

    virtual void writePendingObjects() = 0;

    virtual void finished(::std::vector< ::Ice::Byte >&) = 0;
};

class ICE_API ObjectReader : public ::Ice::Object
{
public:

    virtual void read(const InputStreamPtr&, bool) = 0;

private:

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool = true);

    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};
typedef ::IceInternal::Handle< ObjectReader > ObjectReaderPtr;

class ICE_API ObjectWriter : public ::Ice::Object
{
public:

    virtual void write(const OutputStreamPtr&) const = 0;

private:

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool = true);

    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};
typedef ::IceInternal::Handle< ObjectWriter > ObjectWriterPtr;

class ICE_API ReadObjectCallbackI : public ReadObjectCallback
{
public:

    typedef void (*PatchFunc)(void*, Ice::ObjectPtr&);

    ReadObjectCallbackI(PatchFunc, void*);

    virtual void invoke(const ::Ice::ObjectPtr&);

private:

    PatchFunc _func;
    void* _arg;
};

}

#endif

// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STREAM_I_H
#define ICE_STREAM_I_H

#include <Ice/Stream.h>
#include <Ice/BasicStream.h>

namespace IceInternal
{

//
// Subclass BasicStream in order to associate an InputStream with it.
//
class BasicInputStream : public BasicStream
{
public:

    BasicInputStream(Instance*, Ice::InputStream*);

    Ice::InputStream* _in;
};

//
// Subclass BasicStream in order to associate an OutputStream with it.
//
class BasicOutputStream : public BasicStream
{
public:

    BasicOutputStream(Instance*, Ice::OutputStream*);

    Ice::OutputStream* _out;
};

}

namespace Ice
{

//
// Implement InputStream as a wrapper around BasicInputStream.
//
class InputStreamI : public InputStream
{
public:

    //InputStreamI(const IceInternal::InstancePtr&, const ::std::vector< ::Ice::Byte >&);
    InputStreamI(const Ice::CommunicatorPtr&, const ::std::vector< ::Ice::Byte >&);
    virtual ~InputStreamI();

    virtual Ice::CommunicatorPtr communicator() const;

    virtual void sliceObjects(bool);

    virtual bool readBool();
    virtual ::std::vector< bool > readBoolSeq();
    virtual bool* readBoolSeq(::std::pair<const bool*, const bool*>&);

    virtual ::Ice::Byte readByte();
    virtual ::std::vector< ::Ice::Byte > readByteSeq();
    virtual void readByteSeq(::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&);

    virtual ::Ice::Short readShort();
    virtual ::std::vector< ::Ice::Short > readShortSeq();
    virtual ::Ice::Short* readShortSeq(::std::pair<const ::Ice::Short*, const ::Ice::Short*>&);

    virtual ::Ice::Int readInt();
    virtual ::std::vector< ::Ice::Int > readIntSeq();
    virtual ::Ice::Int* readIntSeq(::std::pair<const ::Ice::Int*, const ::Ice::Int*>&);

    virtual ::Ice::Long readLong();
    virtual ::std::vector< ::Ice::Long > readLongSeq();
    virtual ::Ice::Long* readLongSeq(::std::pair<const ::Ice::Long*, const ::Ice::Long*>&);

    virtual ::Ice::Float readFloat();
    virtual ::std::vector< ::Ice::Float > readFloatSeq();
    virtual ::Ice::Float* readFloatSeq(::std::pair<const ::Ice::Float*, const ::Ice::Float*>&);

    virtual ::Ice::Double readDouble();
    virtual ::std::vector< ::Ice::Double > readDoubleSeq();
    virtual ::Ice::Double* readDoubleSeq(::std::pair<const ::Ice::Double*, const ::Ice::Double*>&);

    virtual ::std::string readString();
    virtual ::std::vector< ::std::string > readStringSeq();

    virtual ::std::wstring readWstring();
    virtual ::std::vector< ::std::wstring > readWstringSeq();

    virtual ::Ice::Int readSize();

    virtual ::Ice::ObjectPrx readProxy();

    virtual void readObject(const ::Ice::ReadObjectCallbackPtr&);

    virtual ::std::string readTypeId();

    virtual void throwException();

    virtual void startSlice();
    virtual void endSlice();
    virtual void skipSlice();

    virtual void startEncapsulation();
    virtual void endEncapsulation();

    virtual void readPendingObjects();

private:

    Ice::CommunicatorPtr _communicator;
    IceInternal::BasicInputStream _is;
    ::std::vector< ReadObjectCallbackPtr > _callbacks;
};

//
// Implement OutputStream as a wrapper around BasicOutputStream.
//
class OutputStreamI : public OutputStream
{
public:

    //OutputStreamI(const IceInternal::InstancePtr&);
    OutputStreamI(const Ice::CommunicatorPtr&);
    virtual ~OutputStreamI();

    virtual Ice::CommunicatorPtr communicator() const;

    virtual void writeBool(bool);
    virtual void writeBoolSeq(const ::std::vector< bool >&);
    virtual void writeBoolSeq(const bool*, const bool*);

    virtual void writeByte(::Ice::Byte);
    virtual void writeByteSeq(const ::std::vector< ::Ice::Byte >&);
    virtual void writeByteSeq(const Ice::Byte*, const Ice::Byte*);

    virtual void writeShort(::Ice::Short);
    virtual void writeShortSeq(const ::std::vector< ::Ice::Short >&);
    virtual void writeShortSeq(const Ice::Short*, const Ice::Short*);

    virtual void writeInt(::Ice::Int);
    virtual void writeIntSeq(const ::std::vector< ::Ice::Int >&);
    virtual void writeIntSeq(const Ice::Int*, const Ice::Int*);

    virtual void writeLong(::Ice::Long);
    virtual void writeLongSeq(const ::std::vector< ::Ice::Long >&);
    virtual void writeLongSeq(const Ice::Long*, const Ice::Long*);

    virtual void writeFloat(::Ice::Float);
    virtual void writeFloatSeq(const ::std::vector< ::Ice::Float >&);
    virtual void writeFloatSeq(const Ice::Float*, const Ice::Float*);

    virtual void writeDouble(::Ice::Double);
    virtual void writeDoubleSeq(const ::std::vector< ::Ice::Double >&);
    virtual void writeDoubleSeq(const Ice::Double*, const Ice::Double*);

    virtual void writeString(const ::std::string&);
    virtual void writeStringSeq(const ::std::vector< ::std::string >&);

    virtual void writeWstring(const ::std::wstring&);
    virtual void writeWstringSeq(const ::std::vector< ::std::wstring >&);

    virtual void writeSize(::Ice::Int);

    virtual void writeProxy(const ::Ice::ObjectPrx&);

    virtual void writeObject(const ::Ice::ObjectPtr&);

    virtual void writeTypeId(const ::std::string&);

    virtual void writeException(const ::Ice::UserException&);

    virtual void startSlice();
    virtual void endSlice();

    virtual void startEncapsulation();
    virtual void endEncapsulation();

    virtual void writePendingObjects();

    virtual void finished(::std::vector< ::Ice::Byte >&);

private:

    Ice::CommunicatorPtr _communicator;
    IceInternal::BasicOutputStream _os;
};

}

#endif

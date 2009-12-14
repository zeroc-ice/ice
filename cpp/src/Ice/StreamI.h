// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STREAM_I_H
#define ICE_STREAM_I_H

#include <Ice/Stream.h>
#include <Ice/BasicStream.h>

namespace Ice
{

//
// Implement InputStream as a wrapper around BasicStream.
//
class InputStreamI : public InputStream
{
public:

    InputStreamI(const Ice::CommunicatorPtr&, const std::vector< Ice::Byte >&);
    InputStreamI(const Ice::CommunicatorPtr&, const std::pair< const Ice::Byte*, const Ice::Byte* >&);
    virtual ~InputStreamI();

    virtual Ice::CommunicatorPtr communicator() const;

    virtual void sliceObjects(bool);

    virtual Ice::Int readSize();
    virtual Ice::Int readAndCheckSeqSize(int);

    virtual void readObject(const Ice::ReadObjectCallbackPtr&);

    virtual std::string readTypeId();

    virtual void throwException();

    virtual void startSlice();
    virtual void endSlice();
    virtual void skipSlice();

    virtual void startEncapsulation();
    virtual void skipEncapsulation();
    virtual void endEncapsulation();

    virtual void readPendingObjects();

    virtual void rewind();

private:
    
    virtual bool internalReadBool();
    virtual ::Ice::Byte internalReadByte();
    virtual ::Ice::Short internalReadShort();
    virtual ::Ice::Int internalReadInt();
    virtual ::Ice::Long internalReadLong();
    virtual ::Ice::Float internalReadFloat();
    virtual ::Ice::Double internalReadDouble();
    virtual ::std::string internalReadString(bool = true);
    virtual ::std::wstring internalReadWstring();
    virtual ::Ice::ObjectPrx internalReadProxy();
    
    //
    // Remove these methods when the old Stream api, is removed.
    //
    virtual std::vector< bool > internalReadBoolSeq();
    virtual bool* internalReadBoolSeq(std::pair<const bool*, const bool*>&);

    virtual std::vector< Ice::Byte > internalReadByteSeq();
    virtual void internalReadByteSeq(std::pair<const Ice::Byte*, const Ice::Byte*>&);

    virtual std::vector< Ice::Short > internalReadShortSeq();
    virtual Ice::Short* internalReadShortSeq(std::pair<const Ice::Short*, const Ice::Short*>&);

    virtual std::vector< Ice::Int > internalReadIntSeq();
    virtual Ice::Int* internalReadIntSeq(std::pair<const Ice::Int*, const Ice::Int*>&);

    virtual std::vector< Ice::Long > internalReadLongSeq();
    virtual Ice::Long* internalReadLongSeq(std::pair<const Ice::Long*, const Ice::Long*>&);

    virtual std::vector< Ice::Float > internalReadFloatSeq();
    virtual Ice::Float* internalReadFloatSeq(std::pair<const Ice::Float*, const Ice::Float*>&);

    virtual std::vector< Ice::Double > internalReadDoubleSeq();
    virtual Ice::Double* internalReadDoubleSeq(std::pair<const Ice::Double*, const Ice::Double*>&);

    virtual std::vector< std::string > internalReadStringSeq(bool = true);

    virtual std::vector< std::wstring > internalReadWstringSeq();

    Ice::CommunicatorPtr _communicator;
    IceInternal::BasicStream* _is;
    std::vector< ReadObjectCallbackPtr > _callbacks;
};

//
// Implement OutputStream as a wrapper around BasicStream.
//
class OutputStreamI : public OutputStream
{
public:

    OutputStreamI(const Ice::CommunicatorPtr&, IceInternal::BasicStream* = 0);
    virtual ~OutputStreamI();

    virtual Ice::CommunicatorPtr communicator() const;

    virtual void writeObject(const ::Ice::ObjectPtr&);
    virtual void writeException(const Ice::UserException&);
    
    virtual void writeSize(Ice::Int);

    virtual void writeTypeId(const std::string&);


    virtual void startSlice();
    virtual void endSlice();

    virtual void startEncapsulation();
    virtual void endEncapsulation();

    virtual void writePendingObjects();

    virtual void finished(std::vector< Ice::Byte >&);

    virtual void reset(bool);

private:
    
    virtual void internalWriteBool(bool);
    virtual void internalWriteByte(::Ice::Byte);
    virtual void internalWriteShort(::Ice::Short);
    virtual void internalWriteInt(::Ice::Int);
    virtual void internalWriteLong(::Ice::Long);
    virtual void internalWriteFloat(::Ice::Float);
    virtual void internalWriteDouble(::Ice::Double);
    virtual void internalWriteString(const ::std::string&, bool = true);
    virtual void internalWriteWstring(const ::std::wstring&);
    virtual void internalWriteProxy(const ::Ice::ObjectPrx&);
    
    //
    // Remove these methods when the old Stream api, is removed.
    //
    virtual void internalWriteBoolSeq(const std::vector< bool >&);
    virtual void internalWriteBoolSeq(const bool*, const bool*);

    virtual void internalWriteByteSeq(const std::vector< Ice::Byte >&);
    virtual void internalWriteByteSeq(const Ice::Byte*, const Ice::Byte*);

    virtual void internalWriteShortSeq(const std::vector< Ice::Short >&);
    virtual void internalWriteShortSeq(const Ice::Short*, const Ice::Short*);

    virtual void internalWriteIntSeq(const std::vector< Ice::Int >&);
    virtual void internalWriteIntSeq(const Ice::Int*, const Ice::Int*);

    virtual void internalWriteLongSeq(const std::vector< Ice::Long >&);
    virtual void internalWriteLongSeq(const Ice::Long*, const Ice::Long*);

    virtual void internalWriteFloatSeq(const std::vector< Ice::Float >&);
    virtual void internalWriteFloatSeq(const Ice::Float*, const Ice::Float*);

    virtual void internalWriteDoubleSeq(const std::vector< Ice::Double >&);
    virtual void internalWriteDoubleSeq(const Ice::Double*, const Ice::Double*);

    virtual void internalWriteStringSeq(const std::vector< std::string >&, bool = true);

    virtual void internalWriteWstringSeq(const std::vector< std::wstring >&);

    Ice::CommunicatorPtr _communicator;
    IceInternal::BasicStream* _os;
    bool _own;
};

}

#endif

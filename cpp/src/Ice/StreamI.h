// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STREAM_I_H
#define ICE_STREAM_I_H

#include <Ice/Stream.h>

namespace IceInternal
{

// Forward declaration.
class BasicStream;

};

namespace Ice
{

//
// Implement InputStream as a wrapper around BasicStream.
//
class InputStreamI : public InputStream
{
public:

    InputStreamI(const CommunicatorPtr&, const std::vector< Byte >&);
    InputStreamI(const CommunicatorPtr&, const std::pair< const Byte*, const Byte* >&);
    virtual ~InputStreamI();

    virtual CommunicatorPtr communicator() const;

    virtual void sliceObjects(bool);

    //
    // These methods should be removed when the old stream API is removed.
    //
    virtual bool readBool();
    virtual Byte readByte();
    virtual Short readShort();
    virtual Int readInt();
    virtual Long readLong();
    virtual Float readFloat();
    virtual Double readDouble();
    virtual ::std::string readString(bool = true);
    virtual ::std::wstring readWstring();
    
    //
    // These methods should be removed when the old stream API is removed.
    //
    virtual std::vector< bool > readBoolSeq();
    virtual std::vector< Byte > readByteSeq();
    virtual std::vector< Short > readShortSeq();
    virtual std::vector< Int > readIntSeq();
    virtual std::vector< Long > readLongSeq();
    virtual std::vector< Float > readFloatSeq();
    virtual std::vector< Double > readDoubleSeq();
    virtual std::vector< std::string > readStringSeq(bool = true);
    virtual std::vector< std::wstring > readWstringSeq();
    
    //
    // These methods should be removed when the old stream API is removed.
    //
    virtual bool* readBoolSeq(std::pair<const bool*, const bool*>&);
    virtual void readByteSeq(std::pair<const Byte*, const Byte*>&);
    virtual Short* readShortSeq(std::pair<const Short*, const Short*>&);
    virtual Int* readIntSeq(std::pair<const Int*, const Int*>&);
    virtual Long* readLongSeq(std::pair<const Long*, const Long*>&);
    virtual Float* readFloatSeq(std::pair<const Float*, const Float*>&);
    virtual Double* readDoubleSeq(std::pair<const Double*, const Double*>&);

    virtual Int readSize();
    virtual Int readAndCheckSeqSize(int);

    virtual ObjectPrx readProxy();
    virtual void readObject(const ReadObjectCallbackPtr&);
    virtual std::string readTypeId();

    virtual void throwException();

    virtual void startSlice();
    virtual void endSlice();
    virtual void skipSlice();

    virtual Ice::EncodingVersion startEncapsulation();
    virtual Ice::EncodingVersion skipEncapsulation();
    virtual void endEncapsulation();

    virtual void readPendingObjects();

    virtual void rewind();
    
    virtual void read(bool& v);
    virtual void read(Byte& v);
    virtual void read(Short& v);
    virtual void read(Int& v);
    virtual void read(Long& v);
    virtual void read(Float& v);
    virtual void read(Double& v);
    virtual void read(std::string& v, bool convert = true);
    virtual void read(std::vector<std::string>&, bool);
    virtual void read(std::wstring& v);
    virtual void read(std::pair<const bool*, const bool*>&, ::IceUtil::ScopedArray<bool>&);
    virtual void read(std::pair<const Byte*, const Byte*>&);
    virtual void read(std::pair<const Short*, const Short*>&, ::IceUtil::ScopedArray<Short>&);
    virtual void read(std::pair<const Int*, const Int*>&, ::IceUtil::ScopedArray<Int>&);
    virtual void read(std::pair<const Long*, const Long*>&, ::IceUtil::ScopedArray<Long>&);
    virtual void read(std::pair<const Float*, const Float*>&, ::IceUtil::ScopedArray<Float>&);
    virtual void read(std::pair<const Double*, const Double*>&, ::IceUtil::ScopedArray<Double>&);

private:

    const CommunicatorPtr _communicator;
    IceInternal::BasicStream* _is;
    std::vector< ReadObjectCallbackPtr > _callbacks;
};

//
// Implement OutputStream as a wrapper around BasicStream.
//
class OutputStreamI : public OutputStream
{
public:

    OutputStreamI(const CommunicatorPtr&, IceInternal::BasicStream* = 0);
    virtual ~OutputStreamI();

    virtual CommunicatorPtr communicator() const;

    //
    // These methods should be removed when the old stream API is removed.
    //
    virtual void writeBool(bool);
    virtual void writeByte(Byte);
    virtual void writeShort(Short);
    virtual void writeInt(Int);
    virtual void writeLong(Long);
    virtual void writeFloat(Float);
    virtual void writeDouble(Double);
    virtual void writeString(const ::std::string&, bool = true);
    virtual void writeWstring(const ::std::wstring&);
    
    //
    // These methods should be removed when the old stream API is removed.
    //
    virtual void writeBoolSeq(const std::vector< bool >&);
    virtual void writeByteSeq(const std::vector< Byte >&);    
    virtual void writeShortSeq(const std::vector< Short >&);
    virtual void writeIntSeq(const std::vector< Int >&);
    virtual void writeLongSeq(const std::vector< Long >&);
    virtual void writeFloatSeq(const std::vector< Float >&);    
    virtual void writeDoubleSeq(const std::vector< Double >&);
    virtual void writeStringSeq(const std::vector< std::string >&, bool = true);
    virtual void writeWstringSeq(const std::vector< std::wstring >&);

    //
    // These methods should be removed when the old stream API is removed.
    //
    virtual void writeBoolSeq(const bool*, const bool*);
    virtual void writeByteSeq(const Byte*, const Byte*);
    virtual void writeShortSeq(const Short*, const Short*);
    virtual void writeIntSeq(const Int*, const Int*);
    virtual void writeLongSeq(const Long*, const Long*);
    virtual void writeFloatSeq(const Float*, const Float*);
    virtual void writeDoubleSeq(const Double*, const Double*);

    virtual void writeObject(const ObjectPtr&);
    virtual void writeException(const UserException&);
    virtual void writeProxy(const ObjectPrx&);
        
    virtual void writeSize(Int);

    virtual void writeTypeId(const std::string&);
    
    virtual void write(bool);
    virtual void write(Byte);
    virtual void write(Short);
    virtual void write(Int);
    virtual void write(Long);
    virtual void write(Float);
    virtual void write(Double);
    virtual void write(const std::string&, bool = true);
    virtual void write(const std::vector<std::string>&, bool);
    virtual void write(const char* v, bool = true);
    virtual void write(const std::wstring& v);

    virtual void write(const bool*, const bool*);
    virtual void write(const Byte*, const Byte*);
    virtual void write(const Short*, const Short*);
    virtual void write(const Int*, const Int*);
    virtual void write(const Long*, const Long*);
    virtual void write(const Float*, const Float*);
    virtual void write(const Double*, const Double*);

    virtual void startSlice();
    virtual void endSlice();

    virtual void startEncapsulation(const Ice::EncodingVersion&);
    virtual void startEncapsulation();
    virtual void endEncapsulation();

    virtual void writePendingObjects();

    virtual void finished(std::vector< Byte >&);

    virtual void reset(bool);

private:

    const CommunicatorPtr _communicator;
    IceInternal::BasicStream* _os;
    const bool _own;
};

}

#endif

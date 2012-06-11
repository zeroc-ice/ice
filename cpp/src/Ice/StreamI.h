// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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

    virtual Int readSize();
    virtual Int readAndCheckSeqSize(int);

    virtual ObjectPrx readProxy();
    virtual void readObject(const ReadObjectCallbackPtr&);

    virtual void throwException();
    virtual void throwException(const UserExceptionReaderFactoryPtr&);

    virtual void startObject();
    virtual SlicedDataPtr endObject(bool);

    virtual void startException();
    virtual SlicedDataPtr endException(bool);

    virtual std::string startSlice();
    virtual void endSlice();
    virtual void skipSlice();

    virtual Ice::EncodingVersion startEncapsulation();
    virtual void endEncapsulation();
    virtual Ice::EncodingVersion skipEncapsulation();

    virtual Ice::EncodingVersion getEncoding();

    virtual void readPendingObjects();

    virtual void rewind();

    virtual void skip(Ice::Int);
    virtual void skipSize();
    
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

    virtual bool readOptional(Int, OptionalType);

    virtual void closure(void*);
    virtual void* closure() const;

private:

    const CommunicatorPtr _communicator;
    IceInternal::BasicStream* _is;
    std::vector< ReadObjectCallbackPtr > _callbacks;
    void* _closure;
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

    virtual void writeObject(const ObjectPtr&);
    virtual void writeException(const UserException&);
    virtual void writeProxy(const ObjectPrx&);
        
    virtual void writeSize(Int);

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

    virtual void writeOptional(Int, OptionalType);

    virtual void format(FormatType);

    virtual void startObject(const SlicedDataPtr&);
    virtual void endObject();

    virtual void startException(const SlicedDataPtr&);
    virtual void endException();

    virtual void startSlice(const std::string&, bool);
    virtual void endSlice();

    virtual void startEncapsulation(const Ice::EncodingVersion&);
    virtual void startEncapsulation();
    virtual void endEncapsulation();

    virtual Ice::EncodingVersion getEncoding();

    virtual void writePendingObjects();

    virtual void finished(std::vector< Byte >&);

    virtual void reset(bool);

    virtual size_type pos();
    virtual void rewrite(size_type, Int);

private:

    const CommunicatorPtr _communicator;
    IceInternal::BasicStream* _os;
    const bool _own;
};

}

#endif

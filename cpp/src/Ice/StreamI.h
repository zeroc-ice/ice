// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    InputStreamI(const CommunicatorPtr&, const std::pair<const Byte*, const Byte*>&, bool);
    InputStreamI(const CommunicatorPtr&, const std::pair<const Byte*, const Byte*>&, const EncodingVersion&, bool);
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

    virtual EncodingVersion startEncapsulation();
    virtual void endEncapsulation();
    virtual EncodingVersion skipEncapsulation();

    virtual EncodingVersion getEncoding() const;

    virtual void readPendingObjects();

    virtual size_type pos();
    virtual void rewind();

    virtual void skip(Int);
    virtual void skipSize();

    virtual void read(bool&);
    virtual void read(Byte&);
    virtual void read(Short&);
    virtual void read(Int&);
    virtual void read(Long&);
    virtual void read(Float&);
    virtual void read(Double&);
    virtual void read(std::string&, bool = true);
    virtual void read(const char*&, size_t&);
    virtual void read(const char*&, size_t&, std::string&);
    virtual void read(std::vector<std::string>&, bool);
    virtual void read(std::wstring&);
    virtual void read(std::vector<bool>&);
    virtual void read(std::pair<const bool*, const bool*>&, ::IceUtil::ScopedArray<bool>&);
    virtual void read(std::pair<const Byte*, const Byte*>&);
    virtual void read(std::pair<const Short*, const Short*>&, ::IceUtil::ScopedArray<Short>&);
    virtual void read(std::pair<const Int*, const Int*>&, ::IceUtil::ScopedArray<Int>&);
    virtual void read(std::pair<const Long*, const Long*>&, ::IceUtil::ScopedArray<Long>&);
    virtual void read(std::pair<const Float*, const Float*>&, ::IceUtil::ScopedArray<Float>&);
    virtual void read(std::pair<const Double*, const Double*>&, ::IceUtil::ScopedArray<Double>&);

#ifdef __SUNPRO_CC
    using InputStream::read;
#endif

    virtual bool readOptional(Int, OptionalFormat);

    virtual void closure(void*);
    virtual void* closure() const;

private:

    void initialize(IceInternal::Instance*, const std::pair<const Byte*, const Byte*>&, const EncodingVersion&, bool);

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

    OutputStreamI(const CommunicatorPtr&);
    OutputStreamI(const CommunicatorPtr&, const EncodingVersion&);
    OutputStreamI(const CommunicatorPtr&, IceInternal::BasicStream*);
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
    virtual void write(const char*, size_t, bool = true);
    virtual void write(const std::vector<std::string>&, bool);
    virtual void write(const char*, bool = true);
    virtual void write(const std::wstring&);
    virtual void write(const std::vector<bool>&);
    virtual void write(const bool*, const bool*);
    virtual void write(const Byte*, const Byte*);
    virtual void write(const Short*, const Short*);
    virtual void write(const Int*, const Int*);
    virtual void write(const Long*, const Long*);
    virtual void write(const Float*, const Float*);
    virtual void write(const Double*, const Double*);

#ifdef __SUNPRO_CC
    using OutputStream::write;
#endif

    virtual bool writeOptional(Int, OptionalFormat);

    virtual void startObject(const SlicedDataPtr&);
    virtual void endObject();

    virtual void startException(const SlicedDataPtr&);
    virtual void endException();

    virtual void startSlice(const std::string&, int, bool);
    virtual void endSlice();

    virtual void startEncapsulation(const EncodingVersion&, FormatType);
    virtual void startEncapsulation();
    virtual void endEncapsulation();

    virtual EncodingVersion getEncoding() const;

    virtual void writePendingObjects();

    virtual void finished(std::vector< Byte >&);
    virtual std::pair<const Byte*, const Byte*> finished();

    virtual void reset(bool);

    virtual size_type pos();
    virtual void rewrite(Int, size_type);

    virtual size_type startSize();
    virtual void endSize(size_type pos);

private:

    const CommunicatorPtr _communicator;
    IceInternal::BasicStream* _os;
    const bool _own;
};

}

#endif

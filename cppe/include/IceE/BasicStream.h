// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_BASIC_STREAM_H
#define ICEE_BASIC_STREAM_H

#include <IceE/InstanceF.h>
#include <IceE/ProxyF.h>

#include <IceE/Buffer.h>

namespace IceE
{

class UserException;

}

namespace IceEInternal
{

class ICEE_API BasicStream : public Buffer
{
public:

    BasicStream(Instance*);
    ~BasicStream();

    //
    // Must return Instance*, because we don't hold an InstancePtr for
    // optimization reasons (see comments below).
    //
    Instance* instance() const;

    void swap(BasicStream&);

    void resize(Container::size_type sz)
    {
	if(sz > _messageSizeMax)
	{
	    throwMemoryLimitException(__FILE__, __LINE__);
	}
	
	b.resize(sz);
    }

    void startSeq(int, int);
    void checkSeq();
    void checkSeq(int);
    void checkFixedSeq(int, int); // For sequences of fixed-size types.
    void endElement()
    {
	assert(_seqDataStack);
	--_seqDataStack->numElements;
    }
    void endSeq(int);

    void startWriteEncaps();
    void endWriteEncaps();

    void startReadEncaps();
    void endReadEncaps();
    void checkReadEncaps();
    IceE::Int getReadEncapsSize();
    void skipEncaps();

    void startWriteSlice();
    void endWriteSlice();

    void startReadSlice();
    void endReadSlice();
    void skipSlice();

    void writeSize(IceE::Int);
    void readSize(IceE::Int&);

    void writeBlob(const std::vector<IceE::Byte>&);
    void readBlob(std::vector<IceE::Byte>&, IceE::Int);

    void writeBlob(const IceE::Byte*, Container::size_type);
    void readBlob(IceE::Byte*, Container::size_type);

    void write(IceE::Byte v)
    {
	b.push_back(v);
    }
    void read(IceE::Byte& v)
    {
	if(i >= b.end())
	{
	    throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	v = *i++;
    }

    void write(const std::vector<IceE::Byte>&);
    void read(std::vector<IceE::Byte>&);

    void write(bool v)
    {
	b.push_back(static_cast<IceE::Byte>(v));
    }
    void write(const std::vector<bool>&);
    void read(bool& v)
    {
	if(i >= b.end())
	{
	    throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	v = *i++;
    }
    void read(std::vector<bool>&);

    void write(IceE::Short);
    void read(IceE::Short&);
    void write(const std::vector<IceE::Short>&);
    void read(std::vector<IceE::Short>&);

    void write(IceE::Int);
    void read(IceE::Int&);
    void write(const std::vector<IceE::Int>&);
    void read(std::vector<IceE::Int>&);

    void write(IceE::Long);
    void read(IceE::Long&);
    void write(const std::vector<IceE::Long>&);
    void read(std::vector<IceE::Long>&);

    void write(IceE::Float);
    void read(IceE::Float&);
    void write(const std::vector<IceE::Float>&);
    void read(std::vector<IceE::Float>&);

    void write(IceE::Double);
    void read(IceE::Double&);
    void write(const std::vector<IceE::Double>&);
    void read(std::vector<IceE::Double>&);

    //
    // NOTE: This function is not implemented. It is declared here to
    // catch programming errors that assume a call such as write("")
    // will invoke write(const std::string&), when in fact the compiler
    // will silently select a different overloading. A link error is the
    // intended result.
    //
    void write(const char*);

    void write(const std::string&);
    void write(const std::vector<std::string>&);
    void read(std::string&);
    void read(std::vector<std::string>&);

    void write(const IceE::ObjectPrx&);
    void read(IceE::ObjectPrx&);

    void write(const IceE::UserException&);
    void throwException();

private:

    //
    // I can't throw these exception from inline functions from within
    // this file, because I cannot include the header with the
    // exceptions. Doing so would screw up the whole include file
    // ordering.
    //
    void throwUnmarshalOutOfBoundsException(const char*, int);
    void throwMemoryLimitException(const char*, int);

    //
    // Optimization. The instance may not be deleted while a
    // stack-allocated BasicStream still holds it.
    //
    Instance* _instance;

    class ICEE_API ReadEncaps : private ::IceE::noncopyable
    {
    public:

	ReadEncaps();
	~ReadEncaps();
	void reset();
	void swap(ReadEncaps&);

	Container::size_type start;
	IceE::Int sz;

	IceE::Byte encodingMajor;
	IceE::Byte encodingMinor;

	ReadEncaps* previous;
    };

    class ICEE_API WriteEncaps : private ::IceE::noncopyable
    {
    public:

	WriteEncaps();
	~WriteEncaps();
	void reset();
	void swap(WriteEncaps&);

	Container::size_type start;

	IceE::Int writeIndex;

	WriteEncaps* previous;
    };

    ReadEncaps* _currentReadEncaps;
    WriteEncaps* _currentWriteEncaps;

    ReadEncaps _preAllocatedReadEncaps;
    WriteEncaps _preAllocatedWriteEncaps;

    Container::size_type _readSlice;
    Container::size_type _writeSlice;

    const Container::size_type _messageSizeMax;

    struct SeqData
    {
	SeqData(int, int);
	int numElements;
	int minSize;
	SeqData* previous;
    };
    SeqData* _seqDataStack;
};

}

#endif

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

namespace Ice
{

class UserException;

}

namespace IceInternal
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
    Ice::Int getReadEncapsSize();
    void skipEncaps();

    void startWriteSlice();
    void endWriteSlice();

    void startReadSlice();
    void endReadSlice();
    void skipSlice();

    void writeSize(Ice::Int);
    void readSize(Ice::Int&);

    void writeBlob(const std::vector<Ice::Byte>&);
    void readBlob(std::vector<Ice::Byte>&, Ice::Int);

    void writeBlob(const Ice::Byte*, Container::size_type);
    void readBlob(Ice::Byte*, Container::size_type);

    void write(Ice::Byte v)
    {
	b.push_back(v);
    }
    void read(Ice::Byte& v)
    {
	if(i >= b.end())
	{
	    throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	v = *i++;
    }

    void write(const std::vector<Ice::Byte>&);
    void read(std::vector<Ice::Byte>&);

    void write(bool v)
    {
	b.push_back(static_cast<Ice::Byte>(v));
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

    void write(Ice::Short);
    void read(Ice::Short&);
    void write(const std::vector<Ice::Short>&);
    void read(std::vector<Ice::Short>&);

    void write(Ice::Int);
    void read(Ice::Int&);
    void write(const std::vector<Ice::Int>&);
    void read(std::vector<Ice::Int>&);

    void write(Ice::Long);
    void read(Ice::Long&);
    void write(const std::vector<Ice::Long>&);
    void read(std::vector<Ice::Long>&);

    void write(Ice::Float);
    void read(Ice::Float&);
    void write(const std::vector<Ice::Float>&);
    void read(std::vector<Ice::Float>&);

    void write(Ice::Double);
    void read(Ice::Double&);
    void write(const std::vector<Ice::Double>&);
    void read(std::vector<Ice::Double>&);

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

    void write(const Ice::ObjectPrx&);
    void read(Ice::ObjectPrx&);

    void write(const Ice::UserException&);
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

    class ICEE_API ReadEncaps : private ::Ice::noncopyable
    {
    public:

	ReadEncaps();
	~ReadEncaps();
	void reset();
	void swap(ReadEncaps&);

	Container::size_type start;
	Ice::Int sz;

	Ice::Byte encodingMajor;
	Ice::Byte encodingMinor;

	ReadEncaps* previous;
    };

    class ICEE_API WriteEncaps : private ::Ice::noncopyable
    {
    public:

	WriteEncaps();
	~WriteEncaps();
	void reset();
	void swap(WriteEncaps&);

	Container::size_type start;

	Ice::Int writeIndex;

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

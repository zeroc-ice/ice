// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_STREAM_H
#define ICE_STREAM_H

#include <Ice/InstanceF.h>
#include <Ice/ObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectFactoryF.h>
#include <Ice/Buffer.h>

namespace Ice
{

class UserException;

}

namespace IceInternal
{

class ICE_API BasicStream : public Buffer
{
public:

    typedef void (*PatchFunc)(void*, Ice::ObjectPtr&);

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
	
	Container::size_type capacity = b.capacity();
	if(capacity < sz)
	{
	    //
	    // COMPILERBUG: Stupid Visual C++ defines max as a
	    // macro. But I can't undefine it in a header file,
	    // because this might cause side effects with other code
	    // that depends on this macro being defined.
	    //
	    //b.reserve(std::max(sz, 2 * capacity));
	    b.reserve(sz > 2 * capacity ? sz : 2 * capacity);
	}

	b.resize(sz);
    }
    void reserve(Container::size_type);

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

    void writeTypeId(const std::string&);
    void readTypeId(std::string&);

    void writeBlob(const std::vector<Ice::Byte>&);
    void readBlob(std::vector<Ice::Byte>&, Ice::Int);

    void writeBlob(const Ice::Byte*, Container::size_type);
    void readBlob(Ice::Byte*, Container::size_type);

    void write(Ice::Byte v)
    {
	b.push_back(v);
    }
    void write(const std::vector<Ice::Byte>&);
    void read(Ice::Byte& v)
    {
	if(i >= b.end())
	{
	    throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	v = *i++;
    }
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
    void write(const std::vector<Ice::Short>&);
    void read(Ice::Short&);
    void read(std::vector<Ice::Short>&);

    void write(Ice::Int);
    void write(const std::vector<Ice::Int>&);
    void read(Ice::Int&);
    void read(std::vector<Ice::Int>&);

    void write(Ice::Long);
    void write(const std::vector<Ice::Long>&);
    void read(Ice::Long&);
    void read(std::vector<Ice::Long>&);

    void write(Ice::Float);
    void write(const std::vector<Ice::Float>&);
    void read(Ice::Float&);
    void read(std::vector<Ice::Float>&);

    void write(Ice::Double);
    void write(const std::vector<Ice::Double>&);
    void read(Ice::Double&);
    void read(std::vector<Ice::Double>&);

    void write(const std::string&);
    void write(const std::vector<std::string>&);
    void read(std::string&);
    void read(std::vector<std::string>&);

    void write(const Ice::ObjectPrx&);
    void read(Ice::ObjectPrx&);

    void write(const Ice::ObjectPtr&);
    void read(PatchFunc, void*);

    void write(const Ice::UserException&);
    void throwException();

    void writePendingObjects();
    void readPendingObjects();

    void marshalFacets(bool);

    struct PatchEntry 
    {
	PatchFunc patchFunc;
	void* patchAddr;
    };

    typedef std::vector<PatchEntry> PatchList;
    typedef std::map<Ice::Int, PatchList> PatchMap;
    typedef std::map<Ice::Int, Ice::ObjectPtr> IndexToPtrMap;
    typedef std::map<Ice::Int, std::string> TypeIdReadMap;

    typedef std::map<Ice::ObjectPtr, Ice::Int> PtrToIndexMap;
    typedef std::map<std::string, Ice::Int> TypeIdWriteMap;

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

    class ICE_API ReadEncaps : public ::IceUtil::noncopyable
    {
    public:

	ReadEncaps();
	~ReadEncaps();

	Container::size_type start;
	Ice::Int sz;

	Ice::Byte encodingMajor;
	Ice::Byte encodingMinor;

	PatchMap* patchMap;
	IndexToPtrMap* unmarshaledMap;
	TypeIdReadMap* typeIdMap;
	Ice::Int typeIdIndex;

	ReadEncaps* previous;
    };

    class ICE_API WriteEncaps : public ::IceUtil::noncopyable
    {
    public:

	WriteEncaps();
	~WriteEncaps();

	Container::size_type start;

	Ice::Int writeIndex;
	PtrToIndexMap* toBeMarshaledMap;
	PtrToIndexMap* marshaledMap;
	TypeIdWriteMap* typeIdMap;
	Ice::Int typeIdIndex;

	WriteEncaps* previous;
    };

    ReadEncaps* _currentReadEncaps;
    WriteEncaps* _currentWriteEncaps;
    Container::size_type _readSlice;
    Container::size_type _writeSlice;

    void writeInstance(const Ice::ObjectPtr&, Ice::Int);
    void patchPointers(Ice::Int, IndexToPtrMap::const_iterator, PatchMap::iterator);

    int _traceSlicing;
    const char* _slicingCat;

    bool _marshalFacets;

    const Container::size_type _messageSizeMax;
};

}

#endif

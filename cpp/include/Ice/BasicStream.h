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
#include <Ice/Logger.h>
#include <list>

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

    BasicStream(Instance *);

    //
    // Must return Instance*, because we don't hold an InstancePtr for
    // optimization reasons (see comments below).
    //
    Instance* instance() const;

    void swap(BasicStream&);

    void resize(int);
    void reserve(int);

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

    void writeBlob(const Ice::Byte*, size_t);
    void readBlob(Ice::Byte*, size_t);

    // Performance critical function inlined, as writing single bytes
    // is used in many places in Ice code.
    void write(Ice::Byte v)
    {
	b.push_back(v);
    }
    void write(const std::vector<Ice::Byte>&);
    void read(Ice::Byte&);
    void read(std::vector<Ice::Byte>&);

    // Performance critical function inlined, as writing single bools
    // is used in many places in Ice code.
    void write(bool v)
    {
	b.push_back(static_cast<Ice::Byte>(v));
    }
    void write(const std::vector<bool>&);
    void read(bool&);
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
    // Optimization. The instance may not be deleted while a
    // stack-allocated BasicStream still holds it.
    //
    Instance* _instance;

    class ICE_API ReadEncaps
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
	Ice::Int typeIdIndex;
	TypeIdReadMap* typeIdMap;
    };

    class ICE_API WriteEncaps
    {
    public:

	WriteEncaps();
	~WriteEncaps();

	Container::size_type start;

	Ice::Int writeIndex;
	PtrToIndexMap* toBeMarshaledMap;
	PtrToIndexMap* marshaledMap;
	Ice::Int typeIdIndex;
	TypeIdWriteMap* typeIdMap;
    };

    std::list<ReadEncaps> _readEncapsStack;
    std::list<WriteEncaps> _writeEncapsStack;
    ReadEncaps* _currentReadEncaps;
    WriteEncaps* _currentWriteEncaps;
    Container::size_type _readSlice;
    Container::size_type _writeSlice;

    void writeInstance(const Ice::ObjectPtr&, Ice::Int);
    void patchPointers(Ice::Int, IndexToPtrMap::const_iterator, PatchMap::iterator);

    int _traceSlicing;
    const char* _slicingCat;
};

}

#endif

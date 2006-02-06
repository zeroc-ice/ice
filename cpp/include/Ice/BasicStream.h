// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BASIC_STREAM_H
#define ICE_BASIC_STREAM_H

#include <Ice/InstanceF.h>
#include <Ice/ObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectFactoryF.h>
#include <Ice/Buffer.h>
#include <IceUtil/AutoArray.h>

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
    void write(const Ice::Byte*, const Ice::Byte*);
    void read(Ice::Byte& v)
    {
	if(i >= b.end())
	{
	    throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	v = *i++;
    }
    void read(std::pair<const Ice::Byte*, const Ice::Byte*>&);

    void write(bool v)
    {
	b.push_back(static_cast<Ice::Byte>(v));
    }
    void write(const std::vector<bool>&);
    void write(const bool*, const bool*);
    void read(bool& v)
    {
	if(i >= b.end())
	{
	    throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
	}
	v = *i++;
    }
    void read(std::vector<bool>&);
    void read(std::pair<const bool*, const bool*>&, IceUtil::auto_array<bool>&);

    void write(Ice::Short);
    void write(const Ice::Short*, const Ice::Short*);
    void read(Ice::Short&);
    void read(std::vector<Ice::Short>&);

    void write(Ice::Int);
    void write(const Ice::Int*, const Ice::Int*);
    void read(Ice::Int&);
    void read(std::vector<Ice::Int>&);

    void write(Ice::Long);
    void write(const Ice::Long*, const Ice::Long*);
    void read(Ice::Long&);
    void read(std::vector<Ice::Long>&);

    void write(Ice::Float);
    void write(const Ice::Float*, const Ice::Float*);
    void read(Ice::Float&);
    void read(std::vector<Ice::Float>&);

    void write(Ice::Double);
    void write(const Ice::Double*, const Ice::Double*);
    void read(Ice::Double&);
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
    void write(const std::string*, const std::string*);
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

    void sliceObjects(bool);

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

    typedef std::vector<Ice::ObjectPtr> ObjectList;

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

    class ICE_API ReadEncaps : private ::IceUtil::noncopyable
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

	PatchMap* patchMap;
	IndexToPtrMap* unmarshaledMap;
	TypeIdReadMap* typeIdMap;
	Ice::Int typeIdIndex;

	ReadEncaps* previous;
    };

    class ICE_API WriteEncaps : private ::IceUtil::noncopyable
    {
    public:

	WriteEncaps();
	~WriteEncaps();
	void reset();
	void swap(WriteEncaps&);

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

    ReadEncaps _preAllocatedReadEncaps;
    WriteEncaps _preAllocatedWriteEncaps;

    Container::size_type _readSlice;
    Container::size_type _writeSlice;

    void writeInstance(const Ice::ObjectPtr&, Ice::Int);
    void patchPointers(Ice::Int, IndexToPtrMap::const_iterator, PatchMap::iterator);

    int _traceSlicing;
    const char* _slicingCat;

    bool _sliceObjects;

    const Container::size_type _messageSizeMax;

    struct SeqData
    {
	SeqData(int, int);
	int numElements;
	int minSize;
	SeqData* previous;
    };
    SeqData* _seqDataStack;

    ObjectList* _objectList;
};

//
// Template functions for marshalling alternative sequence mappings
//

// Sequences of Builtin Types
// Sequences of Bool Sequences
template<typename T> void
writeSequence1(::IceInternal::BasicStream* __os, const T& seq)
{
    ::Ice::Int size = static_cast< ::Ice::Int>(seq.size());
    __os->writeSize(size);
    typedef typename T::const_iterator I;
    for(I p = seq.begin(); p != seq.end(); ++p)
    {
        __os->write(*p);
    }
}

// Sequences of Structures
template<typename T> void
writeSequence2(::IceInternal::BasicStream* __os, const T& seq)
{
    ::Ice::Int size = static_cast< ::Ice::Int>(seq.size());
    __os->writeSize(size);
    typedef typename T::const_iterator I;
    for(I p = seq.begin(); p != seq.end(); ++p)
    {
        (*p).__write(__os);
    }
}

// Sequences of regular Builtin Type Sequences (except Bool)
template<typename T> void
writeSequence3(::IceInternal::BasicStream* __os, const T& seq)
{
    ::Ice::Int size = static_cast< ::Ice::Int>(seq.size());
    __os->writeSize(size);
    typedef typename T::const_iterator I;
    for(I p = seq.begin(); p != seq.end(); ++p)
    {
        __os->write(&(*p)[0], &(*p)[0] + (*p).size());
    }
}

// Sequences of regular non-Builtin Type Sequences
template<typename T, typename S, typename C> void
writeSequence4(::IceInternal::BasicStream* __os, const T& seq, 
		       void (*func)(::IceInternal::BasicStream*, const S*, const S*, C))
{
    ::Ice::Int size = static_cast< ::Ice::Int>(seq.size());
    __os->writeSize(size);
    typedef typename T::const_iterator I;
    for(I p = seq.begin(); p != seq.end(); ++p)
    {
        (*func)(__os, &(*p)[0], &(*p)[0] + (*p).size(), C());
    }
}

// Sequences of alternative Sequences
// Sequences of Dictionaries
template<typename T, typename S, typename C> void
writeSequence5(::IceInternal::BasicStream* __os, const T& seq, 
		       void (*func)(::IceInternal::BasicStream*, const S&, C))
{
    ::Ice::Int size = static_cast< ::Ice::Int>(seq.size());
    __os->writeSize(size);
    typedef typename T::const_iterator I;
    for(I p = seq.begin(); p != seq.end(); ++p)
    {
        (*func)(__os, *p, C());
    }
}

// Sequences of Enums
template<typename T, typename E> void
writeSequence6(::IceInternal::BasicStream* __os, const T& seq, 
		  void (*func)(::IceInternal::BasicStream*, E))
{
    ::Ice::Int size = static_cast< ::Ice::Int>(seq.size());
    __os->writeSize(size);
    typedef typename T::const_iterator I;
    for(I p = seq.begin(); p != seq.end(); ++p)
    {
        (*func)(__os, *p);
    }
}

// Sequences of Proxies
template<typename T, typename P> void
writeSequence7(::IceInternal::BasicStream* __os, const T& seq, 
		  void (*func)(::IceInternal::BasicStream*, const P&))
{
    ::Ice::Int size = static_cast< ::Ice::Int>(seq.size());
    __os->writeSize(size);
    typedef typename T::const_iterator I;
    for(I p = seq.begin(); p != seq.end(); ++p)
    {
        (*func)(__os, *p);
    }
}

// Sequences of Builtin Types
// Sequences of Builtin Type Sequences (except Byte)
template<typename T> void
readSequence1(::IceInternal::BasicStream* __is, T& seq, bool isVariable)
{
    ::Ice::Int size;
    __is->readSize(size);
    T(size).swap(seq);
    if(size > 0)
    {
        if(isVariable)
	{
	    __is->startSeq(size, 1);
	}
        typedef typename T::iterator I;
        for(I p = seq.begin(); p != seq.end(); ++p)
        {
            __is->read(*p);
	    if(isVariable)
	    {
	        __is->checkSeq();
		__is->endElement();
	    }
        }
	if(isVariable)
	{
	    __is->endSeq(size);
	}
    }
}

// Sequences of Structures
template<typename T> void
readSequence2(::IceInternal::BasicStream* __is, T& seq, int elemSize, bool isVariable)
{
    ::Ice::Int size;
    __is->readSize(size);
    T(size).swap(seq);
    if(size > 0)
    {
        if(isVariable)
	{
            __is->startSeq(size, elemSize);
	}
	else
	{
	    __is->checkFixedSeq(size, elemSize);
	}
        typedef typename T::iterator I;
        for(I p = seq.begin(); p != seq.end(); ++p)
        {
            (*p).__read(__is);
	    if(isVariable)
	    {
	        __is->checkSeq();
	        __is->endElement();
	    }
        }
	if(isVariable)
	{
            __is->endSeq(size);
	}
    }
}

// Sequences of Byte Sequences
template<typename T> void
readSequence3(::IceInternal::BasicStream* __is, T& seq)
{
    ::Ice::Int size;
    __is->readSize(size);
    T(size).swap(seq);
    if(size > 0)
    {
	__is->startSeq(size, 1);
        typedef typename T::iterator I;
        for(I p = seq.begin(); p != seq.end(); ++p)
        {
	    std::pair<const Ice::Byte*, const Ice::Byte*> tmp;
	    __is->read(tmp);
	    std::vector<Ice::Byte>(tmp.first, tmp.second).swap(*p);
	    __is->checkSeq();
	    __is->endElement();
        }
	__is->endSeq(size);
    }
}

// Sequences of regular Non-Builtin Type Sequences
// Sequences of Alternative Sequences
// Sequences of Dictionaries
template<typename T, typename S, typename C> void
readSequence4(::IceInternal::BasicStream* __is, T& seq, void (*func)(::IceInternal::BasicStream*, S&, C))
{
    ::Ice::Int size;
    __is->readSize(size);
    T(size).swap(seq);
    if(size > 0)
    {
	__is->startSeq(size, 1);
        typedef typename T::iterator I;
        for(I p = seq.begin(); p != seq.end(); ++p)
        {
	    (*func)(__is, *p, C());
	    __is->checkSeq();
	    __is->endElement();
        }
	__is->endSeq(size);
    }
}

// Sequences of Enums
// Sequences of Proxies
template<typename T, typename S> void
readSequence5(::IceInternal::BasicStream* __is, T& seq, void (*func)(::IceInternal::BasicStream*, S&))
{
    ::Ice::Int size;
    __is->readSize(size);
    T(size).swap(seq);
    if(size > 0)
    {
        typedef typename T::iterator I;
        for(I p = seq.begin(); p != seq.end(); ++p)
        {
	    (*func)(__is, *p);
        }
    }
}

} // End namespace IceInternal

#endif

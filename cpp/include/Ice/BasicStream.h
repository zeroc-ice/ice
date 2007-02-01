// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <Ice/Protocol.h>
#include <Ice/StringConverter.h>
#include <IceUtil/Unicode.h>

namespace Ice
{

class UserException;

}

namespace IceInternal
{

class ICE_API BasicStream : public Buffer
{
public:

    class StreamUTF8BufferI : public Ice::UTF8Buffer
    {
    public:

        StreamUTF8BufferI(BasicStream& stream) : 
            _stream(stream)
        {
        }

        Ice::Byte*
        getMoreBytes(size_t howMany, Ice::Byte* firstUnused)
        {
            assert(howMany > 0);

            if(firstUnused != 0)
            {
                //
                // Return unused bytes
                //
                _stream.b.resize(firstUnused - _stream.b.begin());
            }

            //
            // Index of first unused byte
            //
            Container::size_type pos = _stream.b.size();

            //
            // Since resize may reallocate the buffer, when firstUnused != 0, the
            // return value can be != firstUnused
            //
            _stream.resize(pos + howMany);

            return &_stream.b[pos];
        }

    private:

        BasicStream& _stream;
    };

    typedef void (*PatchFunc)(void*, Ice::ObjectPtr&);

    BasicStream(Instance*, bool = false);
    ~BasicStream()
    {
        // Inlined for performance reasons.

        if(_currentReadEncaps != &_preAllocatedReadEncaps ||
           _currentWriteEncaps != &_preAllocatedWriteEncaps ||
           _seqDataStack || _objectList)
        {
            clear(); // Not inlined.
        }
    }

    void clear();

    //
    // Must return Instance*, because we don't hold an InstancePtr for
    // optimization reasons (see comments below).
    //
    Instance* instance() const { return _instance; } // Inlined for performance reasons.

    void swap(BasicStream&);

    void resize(Container::size_type sz)
    {
        //
        // Check memory limit if stream is not unlimited.
        //
        if(!_unlimited && sz > _messageSizeMax)
        {
            throwMemoryLimitException(__FILE__, __LINE__);
        }
        
        b.resize(sz);
    }

    void startSeq(int, int);
    void checkSeq()
    {
        checkSeq(static_cast<int>(b.end() - i));
    }
    void checkSeq(int bytesLeft)
    {
        //
        // Check, given the number of elements requested for this sequence,
        // that this sequence, plus the sum of the sizes of the remaining
        // number of elements of all enclosing sequences, would still fit
        // within the message.
        //
        int size = 0;
        SeqData* sd = _seqDataStack;
        do
        {
            size += (sd->numElements - 1) * sd->minSize;
            sd = sd->previous;
        }
        while(sd);

        if(size > bytesLeft)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
    }
    void checkFixedSeq(int, int); // For sequences of fixed-size types.
    void endElement()
    {
        assert(_seqDataStack);
        --_seqDataStack->numElements;
    }
    void endSeq(int);

    void startWriteEncaps()
    {
        WriteEncaps* oldEncaps = _currentWriteEncaps;
        if(!oldEncaps) // First allocated encaps?
        {
            _currentWriteEncaps = &_preAllocatedWriteEncaps;
        }
        else
        {
            _currentWriteEncaps = new WriteEncaps();
            _currentWriteEncaps->previous = oldEncaps;
        }
        _currentWriteEncaps->start = b.size();

        write(Ice::Int(0)); // Placeholder for the encapsulation length.
        write(encodingMajor);
        write(encodingMinor);
    }
    void endWriteEncaps()
    {
        assert(_currentWriteEncaps);
        Container::size_type start = _currentWriteEncaps->start;
        Ice::Int sz = static_cast<Ice::Int>(b.size() - start); // Size includes size and version.
        Ice::Byte* dest = &(*(b.begin() + start));

#ifdef ICE_BIG_ENDIAN
        const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&sz) + sizeof(Ice::Int) - 1;
        *dest++ = *src--;
        *dest++ = *src--;
        *dest++ = *src--;
        *dest = *src;
#else
        const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&sz);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;
#endif

        WriteEncaps* oldEncaps = _currentWriteEncaps;
        _currentWriteEncaps = _currentWriteEncaps->previous;
        if(oldEncaps == &_preAllocatedWriteEncaps)
        {
            oldEncaps->reset();
        }
        else
        {
            delete oldEncaps;
        }
    }

    void startReadEncaps()
    {
        ReadEncaps* oldEncaps = _currentReadEncaps;
        if(!oldEncaps) // First allocated encaps?
        {
            _currentReadEncaps = &_preAllocatedReadEncaps;
        }
        else
        {
            _currentReadEncaps = new ReadEncaps();
            _currentReadEncaps->previous = oldEncaps;
        }
        _currentReadEncaps->start = i - b.begin();

        //
        // I don't use readSize() and writeSize() for encapsulations,
        // because when creating an encapsulation, I must know in advance
        // how many bytes the size information will require in the data
        // stream. If I use an Int, it is always 4 bytes. For
        // readSize()/writeSize(), it could be 1 or 5 bytes.
        //
        Ice::Int sz;
        read(sz);
        if(sz < 0)
        {
            throwNegativeSizeException(__FILE__, __LINE__);
        }
        if(i - sizeof(Ice::Int) + sz > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        _currentReadEncaps->sz = sz;

        Ice::Byte eMajor;
        Ice::Byte eMinor;
        read(eMajor);
        read(eMinor);
        if(eMajor != encodingMajor
           || static_cast<unsigned char>(eMinor) > static_cast<unsigned char>(encodingMinor))
        {
            throwUnsupportedEncodingException(__FILE__, __LINE__, eMajor, eMinor);
        }
        _currentReadEncaps->encodingMajor = eMajor;
        _currentReadEncaps->encodingMinor = eMinor;
    }
    void endReadEncaps()
    {
        assert(_currentReadEncaps);
        Container::size_type start = _currentReadEncaps->start;
        Ice::Int sz = _currentReadEncaps->sz;
        i = b.begin() + start + sz;

        ReadEncaps* oldEncaps = _currentReadEncaps;
        _currentReadEncaps = _currentReadEncaps->previous;
        if(oldEncaps == &_preAllocatedReadEncaps)
        {
            oldEncaps->reset();
        }
        else
        {
            delete oldEncaps;
        }
    }
    void checkReadEncaps();
    Ice::Int getReadEncapsSize();
    void skipEncaps();

    void startWriteSlice();
    void endWriteSlice();

    void startReadSlice();
    void endReadSlice();
    void skipSlice();

    void writeSize(Ice::Int v) // Inlined for performance reasons.
    {
        assert(v >= 0);
        if(v > 254)
        {
            write(Ice::Byte(255));
            write(v);
        }
        else
        {
            write(static_cast<Ice::Byte>(v));
        }
    }
    void rewriteSize(Ice::Int v, Container::iterator dest)
    {
        assert(v >= 0);
        if(v > 254)
        {
            *dest++ = Ice::Byte(255);
#ifdef ICE_BIG_ENDIAN
            const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&v) + sizeof(Ice::Int) - 1;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest = *src;
#else
            const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&v);
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest = *src;
#endif
        }
        else
        {
            *dest = static_cast<Ice::Byte>(v);
        }
    }
    void readSize(Ice::Int& v) // Inlined for performance reasons.
    {
        Ice::Byte byte;
        read(byte);
        unsigned val = static_cast<unsigned char>(byte);
        if(val == 255)
        {
            read(v);
            if(v < 0)
            {
                throwNegativeSizeException(__FILE__, __LINE__);
            }
        }
        else
        {
            v = static_cast<Ice::Int>(static_cast<unsigned char>(byte));
        }
    }

    void writeTypeId(const std::string&);
    void readTypeId(std::string&);

    void writeBlob(const std::vector<Ice::Byte>&);
    void readBlob(std::vector<Ice::Byte>&, Ice::Int);

    void writeBlob(const Ice::Byte* v, Container::size_type sz)
    {
        if(sz > 0)
        {
            Container::size_type pos = b.size();
            resize(pos + sz);
            memcpy(&b[pos], &v[0], sz);
        }
    }

    void readBlob(const Ice::Byte*& v, Container::size_type sz)
    {
        if(sz > 0)
        {
            v = i;
            if(static_cast<Container::size_type>(b.end() - i) < sz)
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            i += sz;
        }
        else
        {
            v = i;
        }
    }

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
    bool* read(std::pair<const bool*, const bool*>&);

    void write(Ice::Short);
    void write(const Ice::Short*, const Ice::Short*);
    void read(Ice::Short&);
    void read(std::vector<Ice::Short>&);
    Ice::Short* read(std::pair<const Ice::Short*, const Ice::Short*>&);

    void write(Ice::Int v) // Inlined for performance reasons.
    {
        Container::size_type pos = b.size();
        resize(pos + sizeof(Ice::Int));
        Ice::Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
        const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&v) + sizeof(Ice::Int) - 1;
        *dest++ = *src--;
        *dest++ = *src--;
        *dest++ = *src--;
        *dest = *src;
#else
        const Ice::Byte* src = reinterpret_cast<const Ice::Byte*>(&v);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;
#endif
    }

    void read(Ice::Int& v) // Inlined for performance reasons.
    {
        if(b.end() - i < static_cast<int>(sizeof(Ice::Int)))
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        const Ice::Byte* src = &(*i);
        i += sizeof(Ice::Int);
#ifdef ICE_BIG_ENDIAN
        Ice::Byte* dest = reinterpret_cast<Ice::Byte*>(&v) + sizeof(Ice::Int) - 1;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest = *src;
#else
        Ice::Byte* dest = reinterpret_cast<Ice::Byte*>(&v);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;
#endif
    }

    void write(const Ice::Int*, const Ice::Int*);
    void read(std::vector<Ice::Int>&);
    Ice::Int* read(std::pair<const Ice::Int*, const Ice::Int*>&);

    void write(Ice::Long);
    void write(const Ice::Long*, const Ice::Long*);
    void read(Ice::Long&);
    void read(std::vector<Ice::Long>&);
    Ice::Long* read(std::pair<const Ice::Long*, const Ice::Long*>&);

    void write(Ice::Float);
    void write(const Ice::Float*, const Ice::Float*);
    void read(Ice::Float&);
    void read(std::vector<Ice::Float>&);
    Ice::Float* read(std::pair<const Ice::Float*, const Ice::Float*>&);

    void write(Ice::Double);
    void write(const Ice::Double*, const Ice::Double*);
    void read(Ice::Double&);
    void read(std::vector<Ice::Double>&);
    Ice::Double* read(std::pair<const Ice::Double*, const Ice::Double*>&);

    //
    // NOTE: This function is not implemented. It is declared here to
    // catch programming errors that assume a call such as write("")
    // will invoke write(const std::string&), when in fact the compiler
    // will silently select a different overloading. A link error is the
    // intended result.
    //
    void write(const char*);

    void writeConverted(const std::string& v);
    void write(const std::string& v, bool convert = true)
    {
        Ice::Int sz = static_cast<Ice::Int>(v.size());
        if(convert && sz > 0 && _stringConverter != 0)
        {
            writeConverted(v);
        }
        else
        {
            writeSize(sz);
            if(sz > 0)
            {
                Container::size_type pos = b.size();
                resize(pos + sz);
                memcpy(&b[pos], v.data(), sz);
            }
        }
    }
    void write(const std::string*, const std::string*, bool = true);
    void read(std::string& v, bool convert = true)

    {
        Ice::Int sz;
        readSize(sz);
        if(sz > 0)
        {
            if(b.end() - i < sz)
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            if(convert && _stringConverter != 0)
            {
                _stringConverter->fromUTF8(i, i + sz, v);
            }
            else
            {
                std::string(reinterpret_cast<const char*>(&*i), reinterpret_cast<const char*>(&*i) + sz).swap(v);
            }
            i += sz;
        }
        else
        {
            v.clear();
        }
    }
    void read(std::vector<std::string>&, bool = true);

    void write(const std::wstring& v);
    void write(const std::wstring*, const std::wstring*);
    void read(std::wstring& v)
    {
        Ice::Int sz;
        readSize(sz);
        if(sz > 0)
        {
            if(b.end() - i < sz)
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }

            _wstringConverter->fromUTF8(i, i + sz, v);
            i += sz;
        }
        else
        {
            v.clear();
        }
    }
    void read(std::vector<std::wstring>&);

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
    void throwNegativeSizeException(const char*, int);
    void throwUnsupportedEncodingException(const char*, int, Ice::Byte, Ice::Byte);

    //
    // Optimization. The instance may not be deleted while a
    // stack-allocated BasicStream still holds it.
    //
    Instance* _instance;

    class ICE_API ReadEncaps : private ::IceUtil::noncopyable
    {
    public:

        ReadEncaps() : patchMap(0), unmarshaledMap(0), typeIdMap(0), typeIdIndex(0), previous(0)
        {
            // Inlined for performance reasons.
        }
        ~ReadEncaps()
        {
            // Inlined for performance reasons.
            delete patchMap;
            delete unmarshaledMap;
            delete typeIdMap;
        }
        void reset()
        {
            // Inlined for performance reasons.
            delete patchMap;
            delete unmarshaledMap;
            delete typeIdMap;

            patchMap = 0;
            unmarshaledMap = 0;
            typeIdMap = 0;
            typeIdIndex = 0;
            previous = 0;
        }
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

        WriteEncaps() : writeIndex(0), toBeMarshaledMap(0), marshaledMap(0), typeIdMap(0), typeIdIndex(0), previous(0)
        {
            // Inlined for performance reasons.
        }
        ~WriteEncaps()
        {
            // Inlined for performance reasons.
            delete toBeMarshaledMap;
            delete marshaledMap;
            delete typeIdMap;
        }
        void reset()
        {
            // Inlined for performance reasons.
            delete toBeMarshaledMap;
            delete marshaledMap;
            delete typeIdMap;

            writeIndex = 0;
            toBeMarshaledMap = 0;
            marshaledMap = 0;
            typeIdMap = 0;
            typeIdIndex = 0;
            previous = 0;
        }
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
    bool _unlimited;

    const Ice::StringConverterPtr& _stringConverter;
    const Ice::WstringConverterPtr& _wstringConverter;

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

} // End namespace IceInternal

#endif

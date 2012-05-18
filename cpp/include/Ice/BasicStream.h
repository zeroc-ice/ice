// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
#include <Ice/SlicedDataF.h>
#include <Ice/UserExceptionFactory.h>

namespace Ice
{

class UserException;

template<typename charT> class BasicStringConverter;

typedef BasicStringConverter<char> StringConverter;
typedef IceUtil::Handle<StringConverter> StringConverterPtr;

typedef BasicStringConverter<wchar_t> WstringConverter;
typedef IceUtil::Handle<WstringConverter> WstringConverterPtr;

}

namespace IceInternal
{

class ICE_API BasicStream : public Buffer
{
public:

    typedef void (*PatchFunc)(void*, Ice::ObjectPtr&);

    BasicStream(Instance*, const Ice::EncodingVersion&, bool = false);
    ~BasicStream()
    {
        // Inlined for performance reasons.

        if(_currentReadEncaps != &_preAllocatedReadEncaps || _currentWriteEncaps != &_preAllocatedWriteEncaps)
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

    void* closure() const;
    void* closure(void*);

    void swap(BasicStream&);

    void resize(Container::size_type sz)
    {
        //
        // Check memory limit if stream is not unlimited.
        //
        if(!_unlimited && sz > _messageSizeMax)
        {
            IceInternal::Ex::throwMemoryLimitException(__FILE__, __LINE__, sz, _messageSizeMax);
        }

        b.resize(sz);
    }

    void format(Ice::FormatType);

    void startWriteObject(const Ice::SlicedDataPtr& data)
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->startObject(data);
    }
    void endWriteObject()
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->endObject();
    }

    void startReadObject()
    {
        assert(_currentReadEncaps && _currentReadEncaps->decoder);
        _currentReadEncaps->decoder->startObject();
    }
    Ice::SlicedDataPtr endReadObject(bool preserve)
    {
        assert(_currentReadEncaps && _currentReadEncaps->decoder);
        return _currentReadEncaps->decoder->endObject(preserve);
    }

    void startWriteException(const Ice::SlicedDataPtr& data)
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->startException(data);
    }
    void endWriteException()
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->endException();
    }

    void startReadException()
    {
        assert(_currentReadEncaps && _currentReadEncaps->decoder);
        _currentReadEncaps->decoder->startException();
    }
    Ice::SlicedDataPtr endReadException(bool preserve)
    {
        assert(_currentReadEncaps && _currentReadEncaps->decoder);
        return _currentReadEncaps->decoder->endException(preserve);
    }

    void startWriteEncaps();

    void startWriteEncaps(const Ice::EncodingVersion& encoding)
    {
        checkSupportedEncoding(encoding);

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
        _currentWriteEncaps->encoding = encoding;
        _currentWriteEncaps->start = b.size();

        write(Ice::Int(0)); // Placeholder for the encapsulation length.
        _currentWriteEncaps->encoding.__write(this);
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
    void endWriteEncapsChecked(); // Used by public stream API.
    void writeEmptyEncaps(const Ice::EncodingVersion& encoding)
    {
        checkSupportedEncoding(encoding);
        write(Ice::Int(6)); // Size
        encoding.__write(this);
    }
    void writeEncaps(const Ice::Byte* v, Ice::Int sz)
    {
        if(sz < 6)
        {
            throwEncapsulationException(__FILE__, __LINE__);
        }

        Container::size_type pos = b.size();
        resize(pos + sz);
        memcpy(&b[pos], &v[0], sz);
    }

    const Ice::EncodingVersion& getWriteEncoding() const
    {
        return _currentWriteEncaps ? _currentWriteEncaps->encoding : _encoding;
    }

    const Ice::EncodingVersion& startReadEncaps()
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
        if(sz < 6)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        if(i - sizeof(Ice::Int) + sz > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        _currentReadEncaps->sz = sz;

        _currentReadEncaps->encoding.__read(this);
        checkSupportedEncoding(_currentReadEncaps->encoding); // Make sure the encoding is supported

        return _currentReadEncaps->encoding;
    }
    void endReadEncaps()
    {
        assert(_currentReadEncaps);
        Container::size_type start = _currentReadEncaps->start;
        Ice::Int sz = _currentReadEncaps->sz;
        if(i != b.begin() + start + sz)
        {
            if(i + 1 != b.begin() + start + sz)
            {
                throwEncapsulationException(__FILE__, __LINE__);
            }

            //
            // Ice version < 3.3 had a bug where user exceptions with
            // class members could be encoded with a trailing byte
            // when dispatched with AMD. So we tolerate an extra byte
            // in the encapsulation.
            //
            ++i;
        }

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
    Ice::EncodingVersion skipEmptyEncaps()
    {
        Ice::Int sz;
        read(sz);
        if(sz != static_cast<Ice::Int>(sizeof(Ice::Int)) + 2)
        {
            throwEncapsulationException(__FILE__, __LINE__);
        }

        if(i + 2 > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        Ice::EncodingVersion encoding;
        encoding.__read(this);
        return encoding;
    }
    void endReadEncapsChecked(); // Used by public stream API.
    Ice::EncodingVersion readEncaps(const Ice::Byte*& v, Ice::Int& sz)
    {
        Ice::EncodingVersion encoding;
        v = i;
        read(sz);
        if(sz < 6)
        {
            throwEncapsulationException(__FILE__, __LINE__);
        }

        if(i + 2 > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        encoding.__read(this);
        i += sz - sizeof(Ice::Int) - 2;
        return encoding;
    }

    const Ice::EncodingVersion& getReadEncoding() const
    {
        return _currentReadEncaps ? _currentReadEncaps->encoding : _encoding;
    }

    Ice::Int getReadEncapsSize();
    Ice::EncodingVersion skipEncaps();

    void startWriteSlice(const std::string& typeId, bool last)
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->startSlice(typeId, last);
    }
    void endWriteSlice()
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->endSlice();
    }

    std::string startReadSlice() // Returns type ID of next slice
    {
        assert(_currentReadEncaps && _currentReadEncaps->decoder);
        return _currentReadEncaps->decoder->startSlice();
    }
    void endReadSlice()
    {
        assert(_currentReadEncaps && _currentReadEncaps->decoder);
        _currentReadEncaps->decoder->endSlice();
    }
    void skipSlice()
    {
        assert(_currentReadEncaps && _currentReadEncaps->decoder);
        _currentReadEncaps->decoder->skipSlice();
    }

    void readPendingObjects()
    {
        if(_currentReadEncaps && _currentReadEncaps->decoder)
        {
            _currentReadEncaps->decoder->readPendingObjects();
        }
    }
    void writePendingObjects()
    {
        if(_currentWriteEncaps && _currentWriteEncaps->encoder)
        {
            _currentWriteEncaps->encoder->writePendingObjects();
        }
    }

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
    void writeSizeSeq(const std::vector<Ice::Int>& v)
    {
        writeSize(v.size());
        for(std::vector<Ice::Int>::const_iterator p = v.begin(); p != v.end(); ++p)
        {
            writeSize(*p);
        }
    }
    void rewriteSize(Ice::Int v, Container::iterator dest)
    {
        assert(v >= 0);
        if(v > 254)
        {
            *dest++ = Ice::Byte(255);
            write(v, dest);
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
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
        }
        else
        {
            v = static_cast<Ice::Int>(static_cast<unsigned char>(byte));
        }
    }
    void readSizeSeq(std::vector<Ice::Int>& v)
    {
        Ice::Int sz;
        readSize(sz);
        if(sz > 0)
        {
            v.resize(sz);
            for(Ice::Int n = 0; n < sz; ++n)
            {
                readSize(v[n]);
            }
        }
        else
        {
            v.clear();
        }
    }

    void readAndCheckSeqSize(int, Ice::Int&);

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
    void write(Ice::Byte v, int limit);
    void write(const Ice::Byte*, const Ice::Byte*);
    void read(Ice::Byte& v)
    {
        if(i >= b.end())
        {
            assert(false);
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        v = *i++;
    }
    void read(Ice::Byte& v, int limit);
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
    void write(Ice::Short, int limit);
    void write(const Ice::Short*, const Ice::Short*);
    void read(Ice::Short&);
    void read(Ice::Short&, int limit);
    void read(std::vector<Ice::Short>&);
    Ice::Short* read(std::pair<const Ice::Short*, const Ice::Short*>&);

    void write(Ice::Int v) // Inlined for performance reasons.
    {
        Container::size_type pos = b.size();
        resize(pos + sizeof(Ice::Int));
        write(v, &b[pos]);
    }
    void write(Ice::Int v, Container::iterator dest)
    {
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

    void write(Ice::Int, int limit);

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
    void read(Ice::Int& v, int limit);

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

    void readConverted(std::string&, Ice::Int);
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
                readConverted(v, sz);
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
    void read(std::wstring&);
    void read(std::vector<std::wstring>&);

    void write(const Ice::ObjectPrx&);
    void read(Ice::ObjectPrx&);

    void write(const Ice::ObjectPtr& v)
    {
        initWriteEncaps();
        _currentWriteEncaps->encoder->write(v);
    }
    void read(PatchFunc patchFunc, void* patchAddr)
    {
        assert(patchFunc && patchAddr);
        initReadEncaps();
        _currentReadEncaps->decoder->read(patchFunc, patchAddr);
    }
    void write(const Ice::UserException& e)
    {
        initWriteEncaps();
        _currentWriteEncaps->encoder->write(e);
    }
    void throwException(const UserExceptionFactoryPtr& factory = 0)
    {
        initReadEncaps();
        _currentReadEncaps->decoder->throwException(factory);
    }

    void sliceObjects(bool);

    struct IndirectPatchEntry
    {
        Ice::Int index;
        PatchFunc patchFunc;
        void* patchAddr;
    };

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

    typedef std::vector<IndirectPatchEntry> IndirectPatchList;
    typedef std::vector<Ice::Int> IndexList;
    typedef std::map<Ice::Int, Ice::Int> IndirectionMap;

    typedef std::vector<IndexList> IndexListList;

private:

    //
    // I can't throw these exception from inline functions from within
    // this file, because I cannot include the header with the
    // exceptions. Doing so would screw up the whole include file
    // ordering.
    //
    void throwUnmarshalOutOfBoundsException(const char*, int);
    void throwEncapsulationException(const char*, int);

    //
    // Optimization. The instance may not be deleted while a
    // stack-allocated BasicStream still holds it.
    //
    Instance* _instance;

    //
    // The public stream API needs to attach data to a stream.
    //
    void* _closure;

    class ReadEncaps;
    class WriteEncaps;
    enum SliceType { NoSlice, ObjectSlice, ExceptionSlice };

    class EncapsDecoder : private ::IceUtil::noncopyable
    {
    public:
        EncapsDecoder(BasicStream* stream, ReadEncaps* encaps, bool sliceObjects) :
            _stream(stream), _encaps(encaps), _sliceObjects(sliceObjects), _traceSlicing(-1), _sliceType(NoSlice),
            _typeIdIndex(0)
        {
        } 

        void read(PatchFunc, void*);
        void throwException(const UserExceptionFactoryPtr&);

        void startObject();
        Ice::SlicedDataPtr endObject(bool);

        void startException();
        Ice::SlicedDataPtr endException(bool);

        const std::string& startSlice();
        void endSlice();
        void skipSlice();

        void readPendingObjects();
        
    private:

        const std::string& readTypeId() const;
        Ice::ObjectPtr readInstance();
        void addPatchEntry(Ice::Int, PatchFunc, void*);
        Ice::SlicedDataPtr readSlicedData();

        BasicStream* _stream;
        ReadEncaps* _encaps;
        const bool _sliceObjects;

        int _traceSlicing;
        const char* _slicingCat;

        // Object/exception attributes
        SliceType _sliceType;
        bool _skipFirstSlice;
        Ice::SliceInfoSeq _slices;          // Preserved slices.
        IndexListList _indirectionTables;   // Indirection tables for the preserved slices.

        // Slice attributes
        Ice::Byte _sliceFlags;
        Ice::Int _sliceSize;
        std::string _typeId;
        IndirectPatchList _indirectPatchList;
        
        // Encapsulation attributes for object un-marshalling
        PatchMap _patchMap;
        IndexToPtrMap _unmarshaledMap;
        TypeIdReadMap _typeIdMap;
        Ice::Int _typeIdIndex;
    };

    class EncapsEncoder : private ::IceUtil::noncopyable
    {
    public:
        EncapsEncoder(BasicStream* stream, WriteEncaps* encaps, Ice::FormatType format) : 
            _stream(stream), _encaps(encaps), _format(format), _sliceType(NoSlice), _objectIdIndex(0), _typeIdIndex(0)
        {
        }

        void write(const Ice::ObjectPtr&);
        void write(const Ice::UserException&);

        void startObject(const Ice::SlicedDataPtr&);
        void endObject();

        void startException(const Ice::SlicedDataPtr&);
        void endException();

        void startSlice(const std::string&, bool);
        void endSlice();

        void writePendingObjects();

    private:

        void writeTypeId(const std::string&);
        void writeInstance(const Ice::ObjectPtr&, Ice::Int);
        void writeSlicedData(const Ice::SlicedDataPtr&);
        Ice::Int registerObject(const Ice::ObjectPtr&);

        BasicStream* _stream;
        WriteEncaps* _encaps;
        const Ice::FormatType _format;

        // Object/exception attributes
        SliceType _sliceType;
        bool _firstSlice;

        // Slice attributes
        Ice::Byte _sliceFlags;
        Container::size_type _writeSlice;    // Position of the slice data members
        Container::size_type _sliceFlagsPos; // Position of the slice flags
        IndexList _indirectionTable;
        IndirectionMap _indirectionMap;

        // Encapsulation attributes for object marshalling.
        Ice::Int _objectIdIndex;
        PtrToIndexMap _toBeMarshaledMap;
        PtrToIndexMap _marshaledMap;
        TypeIdWriteMap _typeIdMap;
        Ice::Int _typeIdIndex;
    };

    class ReadEncaps : private ::IceUtil::noncopyable
    {
    public:

        ReadEncaps() : decoder(0), previous(0)
        {
            // Inlined for performance reasons.
        }
        ~ReadEncaps()
        {
            // Inlined for performance reasons.
            delete decoder;
        }
        void reset()
        {
            // Inlined for performance reasons.
            delete decoder;
            decoder = 0;

            previous = 0;
        }

        Container::size_type start;
        Ice::Int sz;
        Ice::EncodingVersion encoding;

        EncapsDecoder* decoder;

        ReadEncaps* previous;
    };

    class WriteEncaps : private ::IceUtil::noncopyable
    {

    public:

        WriteEncaps() : encoder(0), previous(0)
        {
            // Inlined for performance reasons.
        }
        ~WriteEncaps()
        {
            // Inlined for performance reasons.
            delete encoder;
        }
        void reset()
        {
            // Inlined for performance reasons.
            delete encoder;
            encoder = 0;

            previous = 0;
        }

        Container::size_type start;
        Ice::EncodingVersion encoding;

        EncapsEncoder* encoder;

        WriteEncaps* previous;
    };

    //
    // The encoding version to use when there's no encapsulation to
    // read from or write to. This is for example used to read message
    // headers or when the user is using the streaming API with no
    // encapsulation.
    //
    Ice::EncodingVersion _encoding;

    ReadEncaps* _currentReadEncaps;
    WriteEncaps* _currentWriteEncaps;

    void initReadEncaps();
    void initWriteEncaps();

    ReadEncaps _preAllocatedReadEncaps;
    WriteEncaps _preAllocatedWriteEncaps;

    bool _sliceObjects;

    const Container::size_type _messageSizeMax;
    bool _unlimited;

    const Ice::StringConverterPtr& _stringConverter;
    const Ice::WstringConverterPtr& _wstringConverter;

    int _startSeq;
    int _minSeqSize;

    Ice::FormatType _format;

    static const Ice::Byte FLAG_HAS_TYPE_ID_STRING;
    static const Ice::Byte FLAG_HAS_TYPE_ID_INDEX;
    static const Ice::Byte FLAG_HAS_OPTIONAL_MEMBERS;
    static const Ice::Byte FLAG_HAS_INDIRECTION_TABLE;
    static const Ice::Byte FLAG_HAS_SLICE_SIZE;
    static const Ice::Byte FLAG_IS_LAST_SLICE;
};

} // End namespace IceInternal

#endif

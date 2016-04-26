// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BASIC_STREAM_H
#define ICE_BASIC_STREAM_H

#include <IceUtil/StringConverter.h>
#include <Ice/InstanceF.h>
#include <Ice/Object.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectFactoryF.h>
#include <Ice/ObjectFactoryManagerF.h>
#include <Ice/Buffer.h>
#include <Ice/Protocol.h>
#include <Ice/SlicedDataF.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/StreamHelpers.h>
#include <Ice/FactoryTable.h>

namespace Ice
{

class UserException;

}

namespace IceInternal
{

template<typename T> inline void
patchHandle(void* addr, const Ice::ObjectPtr& v)
{
    IceInternal::Handle<T>* p = static_cast<IceInternal::Handle<T>*>(addr);
    __patch(*p, v); // Generated __patch method, necessary for forward declarations.
}

class ICE_API BasicStream : public Buffer
{
public:

    typedef size_t size_type;
    typedef void (*PatchFunc)(void*, const Ice::ObjectPtr&);

    BasicStream(Instance*, const Ice::EncodingVersion&);
    BasicStream(Instance*, const Ice::EncodingVersion&, const Ice::Byte*, const Ice::Byte*);
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
    void resetEncaps();

    void resize(Container::size_type sz)
    {
        b.resize(sz);
        i = b.end();
    }

    void startWriteObject(const Ice::SlicedDataPtr& data)
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->startInstance(ObjectSlice, data);
    }
    void endWriteObject()
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->endInstance();
    }

    void startReadObject()
    {
        assert(_currentReadEncaps && _currentReadEncaps->decoder);
        _currentReadEncaps->decoder->startInstance(ObjectSlice);
    }
    Ice::SlicedDataPtr endReadObject(bool preserve)
    {
        assert(_currentReadEncaps && _currentReadEncaps->decoder);
        return _currentReadEncaps->decoder->endInstance(preserve);
    }

    void startWriteException(const Ice::SlicedDataPtr& data)
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->startInstance(ExceptionSlice, data);
    }
    void endWriteException()
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->endInstance();
    }

    void startReadException()
    {
        assert(_currentReadEncaps && _currentReadEncaps->decoder);
        _currentReadEncaps->decoder->startInstance(ExceptionSlice);
    }
    Ice::SlicedDataPtr endReadException(bool preserve)
    {
        assert(_currentReadEncaps && _currentReadEncaps->decoder);
        return _currentReadEncaps->decoder->endInstance(preserve);
    }

    void startWriteEncaps();

    void startWriteEncaps(const Ice::EncodingVersion& encoding, Ice::FormatType format)
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
        _currentWriteEncaps->format = format;
        _currentWriteEncaps->encoding = encoding;
        _currentWriteEncaps->start = b.size();

        write(Ice::Int(0)); // Placeholder for the encapsulation length.
        write(_currentWriteEncaps->encoding);
    }
    void endWriteEncaps()
    {
        assert(_currentWriteEncaps);

        // Size includes size and version.
        const Ice::Int sz = static_cast<Ice::Int>(b.size() - _currentWriteEncaps->start);
        write(sz, &(*(b.begin() + _currentWriteEncaps->start)));

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
        write(encoding);
    }
    void writeEncaps(const Ice::Byte* v, Ice::Int sz)
    {
        if(sz < 6)
        {
            throwEncapsulationException(__FILE__, __LINE__);
        }

        Container::size_type position = b.size();
        resize(position + sz);
        memcpy(&b[position], &v[0], sz);
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

        read(_currentReadEncaps->encoding);
        checkSupportedEncoding(_currentReadEncaps->encoding); // Make sure the encoding is supported

        return _currentReadEncaps->encoding;
    }

    void endReadEncaps()
    {
        assert(_currentReadEncaps);

        if(_currentReadEncaps->encoding != Ice::Encoding_1_0)
        {
            skipOpts();
            if(i != b.begin() + _currentReadEncaps->start + _currentReadEncaps->sz)
            {
                throwEncapsulationException(__FILE__, __LINE__);
            }
        }
        else if(i != b.begin() + _currentReadEncaps->start + _currentReadEncaps->sz)
        {
            if(i + 1 != b.begin() + _currentReadEncaps->start + _currentReadEncaps->sz)
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
        if(sz < 6)
        {
            throwEncapsulationException(__FILE__, __LINE__);
        }
        if(i - sizeof(Ice::Int) + sz > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        Ice::EncodingVersion encoding;
        read(encoding);
        if(encoding == Ice::Encoding_1_0)
        {
            if(sz != static_cast<Ice::Int>(sizeof(Ice::Int)) + 2)
            {
                throwEncapsulationException(__FILE__, __LINE__);
            }
        }
        else
        {
            // Skip the optional content of the encapsulation if we are expecting an
            // empty encapsulation.
            i += sz - sizeof(Ice::Int) - 2;
        }
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
        if(i - sizeof(Ice::Int) + sz > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        read(encoding);
        i += sz - sizeof(Ice::Int) - 2;
        return encoding;
    }

    const Ice::EncodingVersion& getReadEncoding() const
    {
        return _currentReadEncaps ? _currentReadEncaps->encoding : _encoding;
    }

    Ice::Int getReadEncapsSize();
    Ice::EncodingVersion skipEncaps();

    void startWriteSlice(const std::string& typeId, int compactId, bool last)
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->startSlice(typeId, compactId, last);
    }
    void endWriteSlice()
    {
        assert(_currentWriteEncaps && _currentWriteEncaps->encoder);
        _currentWriteEncaps->encoder->endSlice();
    }

    std::string startReadSlice()
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

    void readPendingObjects();
    void writePendingObjects();

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
            write(v, dest);
        }
        else
        {
            *dest = static_cast<Ice::Byte>(v);
        }
    }
    Ice::Int readSize() // Inlined for performance reasons.
    {
        Ice::Byte byte;
        read(byte);
        unsigned char val = static_cast<unsigned char>(byte);
        if(val == 255)
        {
            Ice::Int v;
            read(v);
            if(v < 0)
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            return v;
        }
        else
        {
            return static_cast<Ice::Int>(static_cast<unsigned char>(byte));
        }
    }

    Ice::Int readAndCheckSeqSize(int);

    size_type startSize()
    {
        size_type position = b.size();
        write(Ice::Int(0));
        return position;
    }

    void endSize(size_type position)
    {
        rewrite(static_cast<Ice::Int>(b.size() - position) - 4, position);
    }

    void writeBlob(const std::vector<Ice::Byte>&);
    void readBlob(std::vector<Ice::Byte>&, Ice::Int);

    void writeBlob(const Ice::Byte* v, Container::size_type sz)
    {
        if(sz > 0)
        {
            Container::size_type position = b.size();
            resize(position + sz);
            memcpy(&b[position], &v[0], sz);
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

    template<typename T> void write(const T& v)
    {
        Ice::StreamHelper<T, Ice::StreamableTraits<T>::helper>::write(this, v);
    }
    template<typename T> void read(T& v)
    {
        Ice::StreamHelper<T, Ice::StreamableTraits<T>::helper>::read(this, v);
    }

    template<typename T> void write(Ice::Int tag, const IceUtil::Optional<T>& v)
    {
        if(!v)
        {
            return; // Optional not set
        }

        if(writeOpt(tag, Ice::StreamOptionalHelper<T,
                                                   Ice::StreamableTraits<T>::helper,
                                                   Ice::StreamableTraits<T>::fixedLength>::optionalFormat))
        {
            Ice::StreamOptionalHelper<T,
                                      Ice::StreamableTraits<T>::helper,
                                      Ice::StreamableTraits<T>::fixedLength>::write(this, *v);
        }
    }
    template<typename T> void read(Ice::Int tag, IceUtil::Optional<T>& v)
    {
        if(readOpt(tag, Ice::StreamOptionalHelper<T,
                                                  Ice::StreamableTraits<T>::helper,
                                                  Ice::StreamableTraits<T>::fixedLength>::optionalFormat))
        {
            v.__setIsSet();
            Ice::StreamOptionalHelper<T,
                                      Ice::StreamableTraits<T>::helper,
                                      Ice::StreamableTraits<T>::fixedLength>::read(this, *v);
        }
        else
        {
            v = IceUtil::None;
        }
    }

    //
    // Template functions for sequences and custom sequences
    //
    template<typename T> void write(const std::vector<T>& v)
    {
        if(v.empty())
        {
            writeSize(0);
        }
        else
        {
            write(&v[0], &v[0] + v.size());
        }
    }
    template<typename T> void write(const T* begin, const T* end)
    {
        writeSize(static_cast<Ice::Int>(end - begin));
        for(const T* p = begin; p != end; ++p)
        {
            write(*p);
        }
    }

    // Read/write type and tag for optionals
    bool writeOpt(Ice::Int tag, Ice::OptionalFormat format)
    {
        assert(_currentWriteEncaps);
        if(_currentWriteEncaps->encoder)
        {
            return _currentWriteEncaps->encoder->writeOpt(tag, format);
        }
        else
        {
            return writeOptImpl(tag, format);
        }
    }
    bool readOpt(Ice::Int tag, Ice::OptionalFormat expectedFormat)
    {
        assert(_currentReadEncaps);
        if(_currentReadEncaps->decoder)
        {
            return _currentReadEncaps->decoder->readOpt(tag, expectedFormat);
        }
        else
        {
            return readOptImpl(tag, expectedFormat);
        }
    }

    // Byte
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
    void read(std::vector<Ice::Byte>&);
    void read(std::pair<const Ice::Byte*, const Ice::Byte*>&);

    // This method is useful for generic stream helpers
    void read(std::pair<const Ice::Byte*, const Ice::Byte*>& p, ::IceUtil::ScopedArray<Ice::Byte>& result)
    {
        result.reset();
        read(p);
    }

    // Bool
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
        v = (0 != *i++);
    }
    void read(std::vector<bool>&);
    void read(std::pair<const bool*, const bool*>&, ::IceUtil::ScopedArray<bool>&);

    // Short
    void write(Ice::Short);
    void write(const Ice::Short*, const Ice::Short*);
    void read(Ice::Short&);
    void read(std::vector<Ice::Short>&);
    void read(std::pair<const Ice::Short*, const Ice::Short*>&, ::IceUtil::ScopedArray<Ice::Short>&);

    // Int
    void write(Ice::Int v) // Inlined for performance reasons.
    {
        Container::size_type position = b.size();
        resize(position + sizeof(Ice::Int));
        write(v, &b[position]);
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
    void read(std::pair<const Ice::Int*, const Ice::Int*>&, ::IceUtil::ScopedArray<Ice::Int>&);

    // Long
    void write(Ice::Long);
    void write(const Ice::Long*, const Ice::Long*);
    void read(Ice::Long&);
    void read(std::vector<Ice::Long>&);
    void read(std::pair<const Ice::Long*, const Ice::Long*>&, ::IceUtil::ScopedArray<Ice::Long>&);

    // Float
    void write(Ice::Float);
    void write(const Ice::Float*, const Ice::Float*);
    void read(Ice::Float&);
    void read(std::vector<Ice::Float>&);
    void read(std::pair<const Ice::Float*, const Ice::Float*>&, ::IceUtil::ScopedArray<Ice::Float>&);

    // Double
    void write(Ice::Double);
    void write(const Ice::Double*, const Ice::Double*);
    void read(Ice::Double&);
    void read(std::vector<Ice::Double>&);
    void read(std::pair<const Ice::Double*, const Ice::Double*>&, ::IceUtil::ScopedArray<Ice::Double>&);

    // String
    void write(const std::string& v, bool convert = true)
    {
        Ice::Int sz = static_cast<Ice::Int>(v.size());
        if(convert && sz > 0 && _stringConverter != 0)
        {
            writeConverted(v.data(), static_cast<size_t>(sz));
        }
        else
        {
            writeSize(sz);
            if(sz > 0)
            {
                Container::size_type position = b.size();
                resize(position + sz);
                memcpy(&b[position], v.data(), sz);
            }
        }
    }

    // for custom strings
    void write(const char* vdata, size_t vsize, bool convert = true)
    {
        Ice::Int sz = static_cast<Ice::Int>(vsize);
        if(convert && sz > 0 && _stringConverter != 0)
        {
            writeConverted(vdata, vsize);
        }
        else
        {
            writeSize(sz);
            if(sz > 0)
            {
                Container::size_type position = b.size();
                resize(position + sz);
                memcpy(&b[position], vdata, vsize);
            }
        }
    }

    // Null-terminated C string
    void write(const char* vdata, bool convert = true)
    {
        write(vdata, strlen(vdata), convert);
    }

    void write(const std::string*, const std::string*, bool = true);

    void read(std::string& v, bool convert = true)
    {
        Ice::Int sz = readSize();
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

    // For custom strings, convert = false
    void read(const char*& vdata, size_t& vsize)
    {
        Ice::Int sz = readSize();
        if(sz > 0)
        {
            if(b.end() - i < sz)
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }

            vdata = reinterpret_cast<const char*>(&*i);
            vsize = static_cast<size_t>(sz);
            i += sz;
        }
        else
        {
            vdata = 0;
            vsize = 0;
        }
    }

    // For custom strings, convert = true
    void read(const char*& vdata, size_t& vsize, std::string& holder)
    {
        if(_stringConverter == 0)
        {
            holder.clear();
            read(vdata, vsize);
        }
        else
        {
            Ice::Int sz = readSize();
            if(sz > 0)
            {
                if(b.end() - i < sz)
                {
                    throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
                }

                readConverted(holder, sz);
                vdata = holder.data();
                vsize = holder.size();
            }
            else
            {
                holder.clear();
                vdata = 0;
                vsize = 0;
            }
        }
    }

    void read(std::vector<std::string>&, bool = true);

    void write(const std::wstring& v);
    void write(const std::wstring*, const std::wstring*);
    void read(std::wstring&);
    void read(std::vector<std::wstring>&);

    // Proxy
    void write(const Ice::ObjectPrx&);
    template<typename T> void write(const IceInternal::ProxyHandle<T>& v)
    {
        write(Ice::ObjectPrx(upCast(v.get())));
    }
    void read(Ice::ObjectPrx&);
    template<typename T> void read(IceInternal::ProxyHandle<T>& v)
    {
        __read(this, v); // Generated __read method, necessary for forward declarations.
    }

    // Class
    void write(const Ice::ObjectPtr& v)
    {
        initWriteEncaps();
        _currentWriteEncaps->encoder->write(v);
    }
    template<typename T> void write(const IceInternal::Handle<T>& v)
    {
        write(Ice::ObjectPtr(upCast(v.get())));
    }
    void read(PatchFunc patchFunc, void* patchAddr)
    {
        initReadEncaps();
        _currentReadEncaps->decoder->read(patchFunc, patchAddr);
    }
    template<typename T> void read(IceInternal::Handle<T>& v)
    {
        read(&patchHandle<T>, &v);
    }

    // Enum
    Ice::Int readEnum(Ice::Int);
    void writeEnum(Ice::Int, Ice::Int);

    // Exception
    void writeException(const Ice::UserException&);
    void throwException(const UserExceptionFactoryPtr& = 0);

    void sliceObjects(bool);

    // Read/write/skip optionals
    bool readOptImpl(Ice::Int, Ice::OptionalFormat);
    bool writeOptImpl(Ice::Int, Ice::OptionalFormat);
    void skipOpt(Ice::OptionalFormat);
    void skipOpts();

    // Skip bytes from the stream
    void skip(size_type size)
    {
        if(i + size > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        i += size;
    }
    void skipSize()
    {
        Ice::Byte bt;
        read(bt);
        if(static_cast<unsigned char>(bt) == 255)
        {
            skip(4);
        }
    }

    size_type pos()
    {
        return i - b.begin();
    }

    void rewrite(Ice::Int value, size_type p)
    {
        write(value, b.begin() + p);
    }

private:

    //
    // String
    //
    void writeConverted(const char*, size_t);
    void readConverted(std::string&, Ice::Int);

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

    typedef std::vector<Ice::ObjectPtr> ObjectList;

    class ICE_API EncapsDecoder : private ::IceUtil::noncopyable
    {
    public:

        virtual ~EncapsDecoder() { }

        virtual void read(PatchFunc, void*) = 0;
        virtual void throwException(const UserExceptionFactoryPtr&) = 0;

        virtual void startInstance(SliceType) = 0;
        virtual Ice::SlicedDataPtr endInstance(bool) = 0;
        virtual const std::string& startSlice() = 0;
        virtual void endSlice() = 0;
        virtual void skipSlice() = 0;

        virtual bool readOpt(Ice::Int, Ice::OptionalFormat)
        {
            return false;
        }

        virtual void readPendingObjects()
        {
        }

    protected:

        EncapsDecoder(BasicStream* stream, ReadEncaps* encaps, bool sliceObjects, const ObjectFactoryManagerPtr& f) :
            _stream(stream), _encaps(encaps), _sliceObjects(sliceObjects), _servantFactoryManager(f), _typeIdIndex(0)
        {
        }

        std::string readTypeId(bool);
        Ice::ObjectPtr newInstance(const std::string&);

        void addPatchEntry(Ice::Int, PatchFunc, void*);
        void unmarshal(Ice::Int, const Ice::ObjectPtr&);

        typedef std::map<Ice::Int, Ice::ObjectPtr> IndexToPtrMap;
        typedef std::map<Ice::Int, std::string> TypeIdReadMap;

        struct PatchEntry
        {
            PatchFunc patchFunc;
            void* patchAddr;
        };
        typedef std::vector<PatchEntry> PatchList;
        typedef std::map<Ice::Int, PatchList> PatchMap;

        BasicStream* _stream;
        ReadEncaps* _encaps;
        const bool _sliceObjects;
        ObjectFactoryManagerPtr _servantFactoryManager;

        // Encapsulation attributes for object un-marshalling
        PatchMap _patchMap;

    private:

        // Encapsulation attributes for object un-marshalling
        IndexToPtrMap _unmarshaledMap;
        TypeIdReadMap _typeIdMap;
        Ice::Int _typeIdIndex;
        ObjectList _objectList;
    };

    class ICE_API EncapsDecoder10 : public EncapsDecoder
    {
    public:

        EncapsDecoder10(BasicStream* stream, ReadEncaps* encaps, bool sliceObjects, const ObjectFactoryManagerPtr& f) :
            EncapsDecoder(stream, encaps, sliceObjects, f), _sliceType(NoSlice)
        {
        }

        virtual void read(PatchFunc, void*);
        virtual void throwException(const UserExceptionFactoryPtr&);

        virtual void startInstance(SliceType);
        virtual Ice::SlicedDataPtr endInstance(bool);
        virtual const std::string& startSlice();
        virtual void endSlice();
        virtual void skipSlice();

        virtual void readPendingObjects();

    private:

        void readInstance();

        // Instance attributes
        SliceType _sliceType;
        bool _skipFirstSlice;

        // Slice attributes
        Ice::Int _sliceSize;
        std::string _typeId;
    };

    class ICE_API EncapsDecoder11 : public EncapsDecoder
    {
    public:

        EncapsDecoder11(BasicStream* stream, ReadEncaps* encaps, bool sliceObjects, const ObjectFactoryManagerPtr& f) :
            EncapsDecoder(stream, encaps, sliceObjects, f), _preAllocatedInstanceData(0), _current(0), _objectIdIndex(1)
        {
        }

        virtual void read(PatchFunc, void*);
        virtual void throwException(const UserExceptionFactoryPtr&);

        virtual void startInstance(SliceType);
        virtual Ice::SlicedDataPtr endInstance(bool);
        virtual const std::string& startSlice();
        virtual void endSlice();
        virtual void skipSlice();

        virtual bool readOpt(Ice::Int, Ice::OptionalFormat);

    private:

        Ice::Int readInstance(Ice::Int, PatchFunc, void*);
        Ice::SlicedDataPtr readSlicedData();

        struct IndirectPatchEntry
        {
            Ice::Int index;
            PatchFunc patchFunc;
            void* patchAddr;
        };
        typedef std::vector<IndirectPatchEntry> IndirectPatchList;

        typedef std::vector<Ice::Int> IndexList;
        typedef std::vector<IndexList> IndexListList;

        struct InstanceData
        {
            InstanceData(InstanceData* p) : previous(p), next(0)
            {
                if(previous)
                {
                    previous->next = this;
                }
            }

            ~InstanceData()
            {
                if(next)
                {
                    delete next;
                }
            }

            // Instance attributes
            SliceType sliceType;
            bool skipFirstSlice;
            Ice::SliceInfoSeq slices; // Preserved slices.
            IndexListList indirectionTables;

            // Slice attributes
            Ice::Byte sliceFlags;
            Ice::Int sliceSize;
            std::string typeId;
            int compactId;
            IndirectPatchList indirectPatchList;

            InstanceData* previous;
            InstanceData* next;
        };
        InstanceData _preAllocatedInstanceData;
        InstanceData* _current;

        void push(SliceType sliceType)
        {
            if(!_current)
            {
                _current = &_preAllocatedInstanceData;
            }
            else
            {
                _current = _current->next ? _current->next : new InstanceData(_current);
            }
            _current->sliceType = sliceType;
            _current->skipFirstSlice = false;
        }

        Ice::Int _objectIdIndex; // The ID of the next object to un-marshal.
    };

    class ICE_API EncapsEncoder : private ::IceUtil::noncopyable
    {
    public:

        virtual ~EncapsEncoder() { }

        virtual void write(const Ice::ObjectPtr&) = 0;
        virtual void write(const Ice::UserException&) = 0;

        virtual void startInstance(SliceType, const Ice::SlicedDataPtr&) = 0;
        virtual void endInstance() = 0;
        virtual void startSlice(const std::string&, int, bool) = 0;
        virtual void endSlice() = 0;

        virtual bool writeOpt(Ice::Int, Ice::OptionalFormat)
        {
            return false;
        }

        virtual void writePendingObjects()
        {
        }

    protected:

        EncapsEncoder(BasicStream* stream, WriteEncaps* encaps) : _stream(stream), _encaps(encaps), _typeIdIndex(0)
        {
        }

        Ice::Int registerTypeId(const std::string&);

        BasicStream* _stream;
        WriteEncaps* _encaps;

        typedef std::map<Ice::ObjectPtr, Ice::Int> PtrToIndexMap;
        typedef std::map<std::string, Ice::Int> TypeIdWriteMap;

        // Encapsulation attributes for object marshalling.
        PtrToIndexMap _marshaledMap;

    private:

        // Encapsulation attributes for object marshalling.
        TypeIdWriteMap _typeIdMap;
        Ice::Int _typeIdIndex;
    };

    class ICE_API EncapsEncoder10 : public EncapsEncoder
    {
    public:

        EncapsEncoder10(BasicStream* stream, WriteEncaps* encaps) :
            EncapsEncoder(stream, encaps), _sliceType(NoSlice), _objectIdIndex(0)
        {
        }

        virtual void write(const Ice::ObjectPtr&);
        virtual void write(const Ice::UserException&);

        virtual void startInstance(SliceType, const Ice::SlicedDataPtr&);
        virtual void endInstance();
        virtual void startSlice(const std::string&, int, bool);
        virtual void endSlice();

        virtual void writePendingObjects();

    private:

        Ice::Int registerObject(const Ice::ObjectPtr&);

        // Instance attributes
        SliceType _sliceType;

        // Slice attributes
        Container::size_type _writeSlice; // Position of the slice data members

        // Encapsulation attributes for object marshalling.
        Ice::Int _objectIdIndex;
        PtrToIndexMap _toBeMarshaledMap;
    };

    class ICE_API EncapsEncoder11 : public EncapsEncoder
    {
    public:

        EncapsEncoder11(BasicStream* stream, WriteEncaps* encaps) :
            EncapsEncoder(stream, encaps), _preAllocatedInstanceData(0), _current(0), _objectIdIndex(1)
        {
        }

        virtual void write(const Ice::ObjectPtr&);
        virtual void write(const Ice::UserException&);

        virtual void startInstance(SliceType, const Ice::SlicedDataPtr&);
        virtual void endInstance();
        virtual void startSlice(const std::string&, int, bool);
        virtual void endSlice();

        virtual bool writeOpt(Ice::Int, Ice::OptionalFormat);

    private:

        void writeSlicedData(const Ice::SlicedDataPtr&);
        void writeInstance(const Ice::ObjectPtr&);

        struct InstanceData
        {
            InstanceData(InstanceData* p) : previous(p), next(0)
            {
                if(previous)
                {
                    previous->next = this;
                }
            }

            ~InstanceData()
            {
                if(next)
                {
                    delete next;
                }
            }

            // Instance attributes
            SliceType sliceType;
            bool firstSlice;

            // Slice attributes
            Ice::Byte sliceFlags;
            Container::size_type writeSlice;    // Position of the slice data members
            Container::size_type sliceFlagsPos; // Position of the slice flags
            PtrToIndexMap indirectionMap;
            ObjectList indirectionTable;

            InstanceData* previous;
            InstanceData* next;
        };
        InstanceData _preAllocatedInstanceData;
        InstanceData* _current;

        Ice::Int _objectIdIndex; // The ID of the next object to marhsal
    };

    class ReadEncaps : private ::IceUtil::noncopyable
    {
    public:

        ReadEncaps() : start(0), decoder(0), previous(0)
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

        WriteEncaps() : format(Ice::DefaultFormat), encoder(0), previous(0)
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
        Ice::FormatType format;

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

    const IceUtil::StringConverterPtr _stringConverter;
    const IceUtil::WstringConverterPtr _wstringConverter;

    int _startSeq;
    int _minSeqSize;
};

} // End namespace IceInternal

#endif

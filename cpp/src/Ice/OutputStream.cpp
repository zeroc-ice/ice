// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/OutputStream.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Instance.h>
#include <Ice/Object.h>
#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ValueFactory.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/TraceUtil.h>
#include <Ice/LoggerUtil.h>
#include <Ice/SlicedData.h>
#include <Ice/StringConverter.h>
#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

class StreamUTF8BufferI : public IceUtil::UTF8Buffer
{
public:

    StreamUTF8BufferI(OutputStream& stream) :
        _stream(stream)
    {
    }

    Ice::Byte* getMoreBytes(size_t howMany, Ice::Byte* firstUnused)
    {
        assert(howMany > 0);

        if(firstUnused != 0)
        {
            //
            // Return unused bytes
            //
            _stream.resize(firstUnused - _stream.b.begin());
        }

        //
        // Index of first unused byte
        //
        Buffer::Container::size_type pos = _stream.b.size();

        //
        // Since resize may reallocate the buffer, when firstUnused != 0, the
        // return value can be != firstUnused
        //
        _stream.resize(pos + howMany);

        return &_stream.b[pos];
    }

private:

    OutputStream& _stream;
};

}

Ice::OutputStream::OutputStream() :
    _instance(0),
    _closure(0),
    _encoding(currentEncoding),
    _format(ICE_ENUM(FormatType, CompactFormat)),
    _currentEncaps(0)
{
}

Ice::OutputStream::OutputStream(const CommunicatorPtr& communicator) :
    _closure(0),
    _currentEncaps(0)
{
    initialize(communicator);
}

Ice::OutputStream::OutputStream(const CommunicatorPtr& communicator, const EncodingVersion& encoding) :
    _closure(0),
    _currentEncaps(0)
{
    initialize(communicator, encoding);
}

Ice::OutputStream::OutputStream(const CommunicatorPtr& communicator, const EncodingVersion& encoding,
                                const pair<const Byte*, const Byte*>& buf) :
    Buffer(buf.first, buf.second),
    _closure(0),
    _currentEncaps(0)
{
    initialize(communicator, encoding);
    b.reset();
}

Ice::OutputStream::OutputStream(Instance* instance, const EncodingVersion& encoding) :
    _closure(0),
    _currentEncaps(0)
{
    initialize(instance, encoding);
}

void
Ice::OutputStream::initialize(const CommunicatorPtr& communicator)
{
    assert(communicator);
    Instance* instance = getInstance(communicator).get();
    initialize(instance, instance->defaultsAndOverrides()->defaultEncoding);
}

void
Ice::OutputStream::initialize(const CommunicatorPtr& communicator, const EncodingVersion& encoding)
{
    assert(communicator);
    initialize(getInstance(communicator).get(), encoding);
}

void
Ice::OutputStream::initialize(Instance* instance, const EncodingVersion& encoding)
{
    assert(instance);

    _instance = instance;
    _encoding = encoding;

    _format = _instance->defaultsAndOverrides()->defaultFormat;
}

void
Ice::OutputStream::clear()
{
    while(_currentEncaps && _currentEncaps != &_preAllocatedEncaps)
    {
        Encaps* oldEncaps = _currentEncaps;
        _currentEncaps = _currentEncaps->previous;
        delete oldEncaps;
    }
}

void
Ice::OutputStream::setFormat(FormatType fmt)
{
    _format = fmt;
}

void*
Ice::OutputStream::getClosure() const
{
    return _closure;
}

void*
Ice::OutputStream::setClosure(void* p)
{
    void* prev = _closure;
    _closure = p;
    return prev;
}

void
Ice::OutputStream::swap(OutputStream& other)
{
    swapBuffer(other);

    std::swap(_instance, other._instance);
    std::swap(_closure, other._closure);
    std::swap(_encoding, other._encoding);
    std::swap(_format, other._format);

    //
    // Swap is never called for streams that have encapsulations being written. However,
    // encapsulations might still be set in case marshalling failed. We just
    // reset the encapsulations if there are still some set.
    //
    resetEncapsulation();
    other.resetEncapsulation();
}

void
Ice::OutputStream::resetEncapsulation()
{
    while(_currentEncaps && _currentEncaps != &_preAllocatedEncaps)
    {
        Encaps* oldEncaps = _currentEncaps;
        _currentEncaps = _currentEncaps->previous;
        delete oldEncaps;
    }

    _preAllocatedEncaps.reset();
}

void
Ice::OutputStream::startEncapsulation()
{
    //
    // If no encoding version is specified, use the current write
    // encapsulation encoding version if there's a current write
    // encapsulation, otherwise, use the stream encoding version.
    //

    if(_currentEncaps)
    {
        startEncapsulation(_currentEncaps->encoding, _currentEncaps->format);
    }
    else
    {
        startEncapsulation(_encoding, Ice::ICE_ENUM(FormatType, DefaultFormat));
    }
}

void
Ice::OutputStream::writePendingValues()
{
    if(_currentEncaps && _currentEncaps->encoder)
    {
        _currentEncaps->encoder->writePendingValues();
    }
    else if(getEncoding() == Ice::Encoding_1_0)
    {
        //
        // If using the 1.0 encoding and no instances were written, we
        // still write an empty sequence for pending instances if
        // requested (i.e.: if this is called).
        //
        // This is required by the 1.0 encoding, even if no instances
        // are written we do marshal an empty sequence if marshaled
        // data types use classes.
        //
        writeSize(0);
    }
}

void
Ice::OutputStream::writeBlob(const vector<Byte>& v)
{
    if(!v.empty())
    {
        Container::size_type pos = b.size();
        resize(pos + v.size());
        memcpy(&b[pos], &v[0], v.size());
    }
}

void
Ice::OutputStream::write(const Byte* begin, const Byte* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz);
        memcpy(&b[pos], begin, sz);
    }
}

void
Ice::OutputStream::write(const vector<bool>& v)
{
    Int sz = static_cast<Int>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz);
        copy(v.begin(), v.end(), b.begin() + pos);
    }
}

namespace
{

template<size_t boolSize>
struct WriteBoolHelper
{
    static void write(const bool* begin, OutputStream::Container::size_type pos, OutputStream::Container& b, Int sz)
    {
        for(int idx = 0; idx < sz; ++idx)
        {
           b[pos + idx] = static_cast<Byte>(*(begin + idx));
        }
    }
};

template<>
struct WriteBoolHelper<1>
{
    static void write(const bool* begin, OutputStream::Container::size_type pos, OutputStream::Container& b, Int sz)
    {
        memcpy(&b[pos], begin, sz);
    }
};

}

void
Ice::OutputStream::write(const bool* begin, const bool* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz);
        WriteBoolHelper<sizeof(bool)>::write(begin, pos, b, sz);
    }
}

void
Ice::OutputStream::write(Short v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Short));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(Short) - 1;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
Ice::OutputStream::write(const Short* begin, const Short* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz * sizeof(Short));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(Short) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(Short);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), sz * sizeof(Short));
#endif
    }
}

void
Ice::OutputStream::write(const Int* begin, const Int* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz * sizeof(Int));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(Int) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(Int);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), sz * sizeof(Int));
#endif
    }
}

void
Ice::OutputStream::write(Long v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Long));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(Long) - 1;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
Ice::OutputStream::write(const Long* begin, const Long* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz * sizeof(Long));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(Long) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(Long);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), sz * sizeof(Long));
#endif
    }
}

void
Ice::OutputStream::write(Float v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Float));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(Float) - 1;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
Ice::OutputStream::write(const Float* begin, const Float* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz * sizeof(Float));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(Float) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(Float);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), sz * sizeof(Float));
#endif
    }
}

void
Ice::OutputStream::write(Double v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(Double));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(Double) - 1;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
#  if defined(ICE_LITTLEBYTE_BIGWORD)
    dest[4] = *src++;
    dest[5] = *src++;
    dest[6] = *src++;
    dest[7] = *src++;
    dest[0] = *src++;
    dest[1] = *src++;
    dest[2] = *src++;
    dest[3] = *src;
#  else
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#  endif
#endif
}

void
Ice::OutputStream::write(const Double* begin, const Double* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + sz * sizeof(Double));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(Double) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(Double);
        }
#elif defined(ICE_LITTLEBYTE_BIGWORD)
        const Byte* src = reinterpret_cast<const Byte*>(begin);
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            dest[4] = *src++;
            dest[5] = *src++;
            dest[6] = *src++;
            dest[7] = *src++;
            dest[0] = *src++;
            dest[1] = *src++;
            dest[2] = *src++;
            dest[3] = *src++;
            dest += sizeof(Double);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), sz * sizeof(Double));
#endif
    }
}

//
// NOTE: This member function is intentionally omitted in order to
// cause a link error if it is used. This is for efficiency reasons:
// writing a const char * requires a traversal of the string to get
// the string length first, which takes O(n) time, whereas getting the
// string length from a std::string takes constant time.
//
/*
void
Ice::OutputStream::write(const char*)
{
}
*/

void
Ice::OutputStream::writeConverted(const char* vdata, size_t vsize)
{
    //
    // What is the size of the resulting UTF-8 encoded string?
    // Impossible to tell, so we guess. If we don't guess correctly,
    // we'll have to fix the mistake afterwards
    //
    try
    {
        Int guessedSize = static_cast<Int>(vsize);
        writeSize(guessedSize); // writeSize() only writes the size; it does not reserve any buffer space.

        size_t firstIndex = b.size();
        StreamUTF8BufferI buffer(*this);

        Byte* lastByte = ICE_NULLPTR;
        bool converted = false;
        if(_instance)
        {
            const StringConverterPtr& stringConverter = _instance->getStringConverter();
            if(stringConverter)
            {
                lastByte = stringConverter->toUTF8(vdata, vdata + vsize, buffer);
                converted = true;
            }
        }
        else
        {
            StringConverterPtr stringConverter = getProcessStringConverter();
            if(stringConverter)
            {
                lastByte = stringConverter->toUTF8(vdata, vdata + vsize, buffer);
                converted = true;
            }
        }

        if(!converted)
        {
            Container::size_type position = b.size();
            resize(position + vsize);
            memcpy(&b[position], vdata, vsize);
            return;
        }

        if(lastByte != b.end())
        {
            resize(lastByte - b.begin());
        }
        size_t lastIndex = b.size();

        Int actualSize = static_cast<Int>(lastIndex - firstIndex);

        //
        // Check against the guess
        //
        if(guessedSize != actualSize)
        {
            if(guessedSize <= 254 && actualSize > 254)
            {
                //
                // Move the UTF-8 sequence 4 bytes further
                // Use memmove instead of memcpy since the source and destination typically overlap.
                //
                resize(b.size() + 4);
                memmove(b.begin() + firstIndex + 4, b.begin() + firstIndex, actualSize);
            }
            else if(guessedSize > 254 && actualSize <= 254)
            {
                //
                // Move the UTF-8 sequence 4 bytes back
                //
                memmove(b.begin() + firstIndex - 4, b.begin() + firstIndex, actualSize);
                resize(b.size() - 4);
            }

            if(guessedSize <= 254)
            {
                rewriteSize(actualSize, b.begin() + firstIndex - 1);
            }
            else
            {
                rewriteSize(actualSize, b.begin() + firstIndex - 1 - 4);
            }
        }
    }
    catch(const IceUtil::IllegalConversionException& ex)
    {
        throw StringConversionException(__FILE__, __LINE__, ex.reason());
    }
}

void
Ice::OutputStream::write(const string* begin, const string* end, bool convert)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        for(int i = 0; i < sz; ++i)
        {
            write(begin[i], convert);
        }
    }
}

void
Ice::OutputStream::write(const wstring& v)
{
    if(v.empty())
    {
        writeSize(0);
        return;
    }

    //
    // What is the size of the resulting UTF-8 encoded string?
    // Impossible to tell, so we guess. If we don't guess correctly,
    // we'll have to fix the mistake afterwards
    //
    try
    {
        Int guessedSize = static_cast<Int>(v.size());
        writeSize(guessedSize); // writeSize() only writes the size; it does not reserve any buffer space.

        size_t firstIndex = b.size();
        StreamUTF8BufferI buffer(*this);

        Byte* lastByte = ICE_NULLPTR;

        // Note: wstringConverter is never null; when set to null, get returns the default unicode wstring converter
        if(_instance)
        {
            lastByte = _instance->getWstringConverter()->toUTF8(v.data(), v.data() + v.size(), buffer);
        }
        else
        {
            lastByte = getProcessWstringConverter()->toUTF8(v.data(), v.data() + v.size(), buffer);
        }

        if(lastByte != b.end())
        {
            resize(lastByte - b.begin());
        }
        size_t lastIndex = b.size();

        Int actualSize = static_cast<Int>(lastIndex - firstIndex);

        //
        // Check against the guess
        //
        if(guessedSize != actualSize)
        {
            if(guessedSize <= 254 && actualSize > 254)
            {
                //
                // Move the UTF-8 sequence 4 bytes further
                // Use memmove instead of memcpy since the source and destination typically overlap.
                //
                resize(b.size() + 4);
                memmove(b.begin() + firstIndex + 4, b.begin() + firstIndex, actualSize);
            }
            else if(guessedSize > 254 && actualSize <= 254)
            {
                //
                // Move the UTF-8 sequence 4 bytes back
                //
                memmove(b.begin() + firstIndex - 4, b.begin() + firstIndex, actualSize);
                resize(b.size() - 4);
            }

            if(guessedSize <= 254)
            {
                rewriteSize(actualSize, b.begin() + firstIndex - 1);
            }
            else
            {
                rewriteSize(actualSize, b.begin() + firstIndex - 1 - 4);
            }
        }
    }
    catch(const IceUtil::IllegalConversionException& ex)
    {
        throw StringConversionException(__FILE__, __LINE__, ex.reason());
    }
}

void
Ice::OutputStream::write(const wstring* begin, const wstring* end)
{
    Int sz = static_cast<Int>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        for(int i = 0; i < sz; ++i)
        {
            write(begin[i]);
        }
    }
}

void
#ifdef ICE_CPP11_MAPPING
Ice::OutputStream::writeProxy(const shared_ptr<ObjectPrx>& v)
#else
Ice::OutputStream::write(const ObjectPrx& v)
#endif
{
    if(v)
    {
        v->_write(*this);
    }
    else
    {
        Identity ident;
        write(ident);
    }
}

void
Ice::OutputStream::writeEnum(Int v, Int maxValue)
{
    if(getEncoding() == Encoding_1_0)
    {
        if(maxValue < 127)
        {
            write(static_cast<Byte>(v));
        }
        else if(maxValue < 32767)
        {
            write(static_cast<Short>(v));
        }
        else
        {
            write(v);
        }
    }
    else
    {
        writeSize(v);
    }
}

void
Ice::OutputStream::writeException(const UserException& e)
{
    initEncaps();
    _currentEncaps->encoder->write(e);
}

bool
Ice::OutputStream::writeOptImpl(Int tag, OptionalFormat type)
{
    if(getEncoding() == Encoding_1_0)
    {
        return false; // Optional members aren't supported with the 1.0 encoding.
    }

    Byte v = static_cast<Byte>(type);
    if(tag < 30)
    {
        v |= static_cast<Byte>(tag << 3);
        write(v);
    }
    else
    {
        v |= 0xF0; // tag = 30
        write(v);
        writeSize(tag);
    }
    return true;
}

void
Ice::OutputStream::finished(vector<Byte>& bytes)
{
    vector<Byte>(b.begin(), b.end()).swap(bytes);
}

pair<const Byte*, const Byte*>
Ice::OutputStream::finished()
{
    if(b.empty())
    {
        return pair<const Byte*, const Byte*>(reinterpret_cast<Ice::Byte*>(0), reinterpret_cast<Ice::Byte*>(0));
    }
    else
    {
        return pair<const Byte*, const Byte*>(&b[0], &b[0] + b.size());
    }
}

void
Ice::OutputStream::throwEncapsulationException(const char* file, int line)
{
    throw EncapsulationException(file, line);
}

void
Ice::OutputStream::initEncaps()
{
    if(!_currentEncaps) // Lazy initialization.
    {
        _currentEncaps = &_preAllocatedEncaps;
        _currentEncaps->start = b.size();
        _currentEncaps->encoding = _encoding;
    }

    if(_currentEncaps->format == Ice::ICE_ENUM(FormatType, DefaultFormat))
    {
        _currentEncaps->format = _format;
    }

    if(!_currentEncaps->encoder) // Lazy initialization.
    {
        if(_currentEncaps->encoding == Encoding_1_0)
        {
            _currentEncaps->encoder = new EncapsEncoder10(this, _currentEncaps);
        }
        else
        {
            _currentEncaps->encoder = new EncapsEncoder11(this, _currentEncaps);
        }
    }
}

Ice::OutputStream::EncapsEncoder::~EncapsEncoder()
{
    // Out of line to avoid weak vtable
}

Int
Ice::OutputStream::EncapsEncoder::registerTypeId(const string& typeId)
{
    TypeIdMap::const_iterator p = _typeIdMap.find(typeId);
    if(p != _typeIdMap.end())
    {
        return p->second;
    }
    else
    {
        _typeIdMap.insert(make_pair(typeId, ++_typeIdIndex));
        return -1;
    }
}

void
Ice::OutputStream::EncapsEncoder10::write(const ValuePtr& v)
{
    //
    // Object references are encoded as a negative integer in 1.0.
    //
    if(v)
    {
        _stream->write(-registerValue(v));
    }
    else
    {
        _stream->write(0);
    }
}

void
Ice::OutputStream::EncapsEncoder10::write(const UserException& v)
{
    //
    // User exception with the 1.0 encoding start with a boolean
    // flag that indicates whether or not the exception uses
    // classes.
    //
    // This allows reading the pending instances even if some part of
    // the exception was sliced.
    //
    bool usesClasses = v._usesClasses();
    _stream->write(usesClasses);
    v._write(_stream);
    if(usesClasses)
    {
        writePendingValues();
    }
}

void
Ice::OutputStream::EncapsEncoder10::startInstance(SliceType sliceType, const SlicedDataPtr&)
{
    _sliceType = sliceType;
}

void
Ice::OutputStream::EncapsEncoder10::endInstance()
{
    if(_sliceType == ValueSlice)
    {
        //
        // Write the Object slice.
        //
        startSlice(Object::ice_staticId(), -1, true);
        _stream->writeSize(0); // For compatibility with the old AFM.
        endSlice();
    }
    _sliceType = NoSlice;
}

void
Ice::OutputStream::EncapsEncoder10::startSlice(const string& typeId, int, bool /*last*/)
{
    //
    // For instance slices, encode a boolean to indicate how the type ID
    // is encoded and the type ID either as a string or index. For
    // exception slices, always encode the type ID as a string.
    //
    if(_sliceType == ValueSlice)
    {
        Int index = registerTypeId(typeId);
        if(index < 0)
        {
            _stream->write(false);
            _stream->write(typeId, false);
        }
        else
        {
            _stream->write(true);
            _stream->writeSize(index);
        }
    }
    else
    {
        _stream->write(typeId, false);
    }

    _stream->write(Int(0)); // Placeholder for the slice length.

    _writeSlice = _stream->b.size();
}

void
Ice::OutputStream::EncapsEncoder10::endSlice()
{
    //
    // Write the slice length.
    //
    Int sz = static_cast<Int>(_stream->b.size() - _writeSlice + sizeof(Int));
    Byte* dest = &(*(_stream->b.begin() + _writeSlice - sizeof(Int)));
    _stream->write(sz, dest);
}

void
Ice::OutputStream::EncapsEncoder10::writePendingValues()
{
    while(!_toBeMarshaledMap.empty())
    {
        //
        // Consider the to be marshalled instances as marshalled now,
        // this is necessary to avoid adding again the "to be
        // marshalled instances" into _toBeMarshaledMap while writing
        // instances.
        //
        _marshaledMap.insert(_toBeMarshaledMap.begin(), _toBeMarshaledMap.end());

        PtrToIndexMap savedMap;
        savedMap.swap(_toBeMarshaledMap);
        _stream->writeSize(static_cast<Int>(savedMap.size()));
        for(PtrToIndexMap::iterator p = savedMap.begin(); p != savedMap.end(); ++p)
        {
            //
            // Ask the instance to marshal itself. Any new class
            // instances that are triggered by the classes marshaled
            // are added to toBeMarshaledMap.
            //
            _stream->write(p->second);

            try
            {
                p->first->ice_preMarshal();
            }
            catch(const std::exception& ex)
            {
                Warning out(_stream->instance()->initializationData().logger);
                out << "std::exception raised by ice_preMarshal:\n" << ex;
            }
            catch(...)
            {
                Warning out(_stream->instance()->initializationData().logger);
                out << "unknown exception raised by ice_preMarshal";
            }

            p->first->_iceWrite(_stream);
        }
    }
    _stream->writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
}

Int
Ice::OutputStream::EncapsEncoder10::registerValue(const ValuePtr& v)
{
    assert(v);

    //
    // Look for this instance in the to-be-marshaled map.
    //
    PtrToIndexMap::const_iterator p = _toBeMarshaledMap.find(v);
    if(p != _toBeMarshaledMap.end())
    {
        return p->second;
    }

    //
    // Didn't find it, try the marshaled map next.
    //
    PtrToIndexMap::const_iterator q = _marshaledMap.find(v);
    if(q != _marshaledMap.end())
    {
        return q->second;
    }

    //
    // We haven't seen this instance previously, create a new
    // index, and insert it into the to-be-marshaled map.
    //
    _toBeMarshaledMap.insert(make_pair(v, ++_valueIdIndex));
    return _valueIdIndex;
}

void
Ice::OutputStream::EncapsEncoder11::write(const ValuePtr& v)
{
    if(!v)
    {
        _stream->writeSize(0); // Nil reference.
    }
    else if(_current && _encaps->format == ICE_ENUM(FormatType, SlicedFormat))
    {
        //
        // If writing an instance within a slice and using the sliced
        // format, write an index from the instance indirection
        // table. The indirect instance table is encoded at the end of
        // each slice and is always read (even if the Slice is
        // unknown).
        //
        PtrToIndexMap::const_iterator p = _current->indirectionMap.find(v);
        if(p == _current->indirectionMap.end())
        {
            _current->indirectionTable.push_back(v);
            Int idx = static_cast<Int>(_current->indirectionTable.size()); // Position + 1 (0 is reserved for nil)
            _current->indirectionMap.insert(make_pair(v, idx));
            _stream->writeSize(idx);
        }
        else
        {
            _stream->writeSize(p->second);
        }
    }
    else
    {
        writeInstance(v); // Write the instance or a reference if already marshaled.
    }
}

void
Ice::OutputStream::EncapsEncoder11::write(const UserException& v)
{
    v._write(_stream);
}

void
Ice::OutputStream::EncapsEncoder11::startInstance(SliceType sliceType, const SlicedDataPtr& data)
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
    _current->firstSlice = true;

    if(data)
    {
        writeSlicedData(data);
    }
}

void
Ice::OutputStream::EncapsEncoder11::endInstance()
{
    _current = _current->previous;
}

void
Ice::OutputStream::EncapsEncoder11::startSlice(const string& typeId, int compactId, bool last)
{
    assert(_current->indirectionTable.empty() && _current->indirectionMap.empty());

    _current->sliceFlagsPos = _stream->b.size();

    _current->sliceFlags = 0;
    if(_encaps->format == ICE_ENUM(FormatType, SlicedFormat))
    {
        _current->sliceFlags |= FLAG_HAS_SLICE_SIZE; // Encode the slice size if using the sliced format.
    }
    if(last)
    {
        _current->sliceFlags |= FLAG_IS_LAST_SLICE; // This is the last slice.
    }

    _stream->write(Byte(0)); // Placeholder for the slice flags

    //
    // For instance slices, encode the flag and the type ID either as a
    // string or index. For exception slices, always encode the type
    // ID a string.
    //
    if(_current->sliceType == ValueSlice)
    {
        //
        // Encode the type ID (only in the first slice for the compact
        // encoding).
        //
        if(_encaps->format == ICE_ENUM(FormatType, SlicedFormat) || _current->firstSlice)
        {
            if(compactId >= 0)
            {
                _current->sliceFlags |= FLAG_HAS_TYPE_ID_COMPACT;
                _stream->writeSize(compactId);
            }
            else
            {
                Int index = registerTypeId(typeId);
                if(index < 0)
                {
                    _current->sliceFlags |= FLAG_HAS_TYPE_ID_STRING;
                    _stream->write(typeId, false);
                }
                else
                {
                    _current->sliceFlags |= FLAG_HAS_TYPE_ID_INDEX;
                    _stream->writeSize(index);
                }
            }
        }
    }
    else
    {
        _stream->write(typeId, false);
    }

    if(_current->sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        _stream->write(Int(0)); // Placeholder for the slice length.
    }

    _current->writeSlice = _stream->b.size();
    _current->firstSlice = false;
}

void
Ice::OutputStream::EncapsEncoder11::endSlice()
{
    //
    // Write the optional member end marker if some optional members
    // were encoded. Note that the optional members are encoded before
    // the indirection table and are included in the slice size.
    //
    if(_current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS)
    {
        _stream->write(OPTIONAL_END_MARKER);
    }

    //
    // Write the slice length if necessary.
    //
    if(_current->sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        Int sz = static_cast<Int>(_stream->b.size() - _current->writeSlice + sizeof(Int));
        Byte* dest = &(*(_stream->b.begin() + _current->writeSlice - sizeof(Int)));
        _stream->write(sz, dest);
    }

    //
    // Only write the indirection table if it contains entries.
    //
    if(!_current->indirectionTable.empty())
    {
        assert(_encaps->format == ICE_ENUM(FormatType, SlicedFormat));
        _current->sliceFlags |= FLAG_HAS_INDIRECTION_TABLE;

        //
        // Write the indirect instance table.
        //
        _stream->writeSize(static_cast<Int>(_current->indirectionTable.size()));
        ValueList::const_iterator p;
        for(p = _current->indirectionTable.begin(); p != _current->indirectionTable.end(); ++p)
        {
            writeInstance(*p);
        }
        _current->indirectionTable.clear();
        _current->indirectionMap.clear();
    }

    //
    // Finally, update the slice flags.
    //
    Byte* dest = &(*(_stream->b.begin() + _current->sliceFlagsPos));
    *dest = _current->sliceFlags;
}

bool
Ice::OutputStream::EncapsEncoder11::writeOptional(Ice::Int tag, Ice::OptionalFormat format)
{
    if(!_current)
    {
        return _stream->writeOptImpl(tag, format);
    }
    else
    {
        if(_stream->writeOptImpl(tag, format))
        {
            _current->sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
            return true;
        }
        else
        {
            return false;
        }
    }
}

void
Ice::OutputStream::EncapsEncoder11::writeSlicedData(const SlicedDataPtr& slicedData)
{
    assert(slicedData);

    //
    // We only remarshal preserved slices if we are using the sliced
    // format. Otherwise, we ignore the preserved slices, which
    // essentially "slices" the instance into the most-derived type
    // known by the sender.
    //
    if(_encaps->format != ICE_ENUM(FormatType, SlicedFormat))
    {
        return;
    }

    for(SliceInfoSeq::const_iterator p = slicedData->slices.begin(); p != slicedData->slices.end(); ++p)
    {
        startSlice((*p)->typeId, (*p)->compactId, (*p)->isLastSlice);

        //
        // Write the bytes associated with this slice.
        //
        _stream->writeBlob((*p)->bytes);

        if((*p)->hasOptionalMembers)
        {
            _current->sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
        }

        //
        // Make sure to also re-write the instance indirection table.
        //
        _current->indirectionTable = (*p)->instances;

        endSlice();
    }
}

void
Ice::OutputStream::EncapsEncoder11::writeInstance(const ValuePtr& v)
{
    assert(v);

    //
    // If the instance was already marshaled, just write it's ID.
    //
    PtrToIndexMap::const_iterator q = _marshaledMap.find(v);
    if(q != _marshaledMap.end())
    {
        _stream->writeSize(q->second);
        return;
    }

    //
    // We haven't seen this instance previously, create a new ID,
    // insert it into the marshaled map, and write the instance.
    //
    _marshaledMap.insert(make_pair(v, ++_valueIdIndex));

    try
    {
        v->ice_preMarshal();
    }
    catch(const std::exception& ex)
    {
        Warning out(_stream->instance()->initializationData().logger);
        out << "std::exception raised by ice_preMarshal:\n" << ex;
    }
    catch(...)
    {
        Warning out(_stream->instance()->initializationData().logger);
        out << "unknown exception raised by ice_preMarshal";
    }

    _stream->writeSize(1); // Object instance marker.
    v->_iceWrite(_stream);
}

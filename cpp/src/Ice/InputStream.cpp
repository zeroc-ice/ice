// Copyright (c) ZeroC, Inc.

#include "Ice/InputStream.h"
#include "DefaultsAndOverrides.h"
#include "Endian.h"
#include "Ice/DefaultSliceLoader.h" // temporary
#include "Ice/LocalExceptions.h"
#include "Ice/SlicedData.h"
#include "Ice/StringConverter.h"
#include "Ice/UserExceptionFactory.h"
#include "Instance.h"
#include "ReferenceFactory.h"
#include "TraceLevels.h"
#include "TraceUtil.h"

#include "DisableWarnings.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    const char* endOfBufferMessage = "attempting to unmarshal past the end of the buffer";
}

void
IceInternal::Ex::throwUOE(const char* file, int line, const string& expectedType, const ValuePtr& v)
{
    UnknownSlicedValuePtr usv = dynamic_pointer_cast<UnknownSlicedValue>(v);
    if (usv)
    {
        throw MarshalException{
            file,
            line,
            "the Slice loader did not find a class for type ID '" + string{usv->ice_id()} + "'"};
    }

    string type = v->ice_id();

    throw MarshalException{
        file,
        line,
        "failed to unmarshal class with type ID '" + expectedType +
            "': the Slice loader returned a class with type ID '" + type + "'"};
}

void
IceInternal::Ex::throwMemoryLimitException(const char* file, int line, size_t requested, int32_t maximum)
{
    throw MarshalException{
        file,
        line,
        "cannot unmarshal Ice message: the message size of " + to_string(requested) +
            " bytes exceeds the maximum allowed of " + to_string(maximum) + " bytes (see Ice.MessageSizeMax)."};
}

void
IceInternal::Ex::throwMarshalException(const char* file, int line, string reason)
{
    throw Ice::MarshalException{file, line, std::move(reason)};
}

Ice::InputStream::InputStream(Instance* instance, EncodingVersion encoding, SliceLoaderPtr sliceLoader)
    : InputStream{instance, encoding, Buffer{}, std::move(sliceLoader)}
{
}

Ice::InputStream::InputStream(const CommunicatorPtr& communicator, const vector<byte>& v, SliceLoaderPtr sliceLoader)
    : InputStream{
          getInstance(communicator).get(),
          getInstance(communicator)->defaultsAndOverrides()->defaultEncoding,
          Buffer{v},
          std::move(sliceLoader)}
{
}

Ice::InputStream::InputStream(
    const CommunicatorPtr& communicator,
    pair<const byte*, const byte*> p,
    SliceLoaderPtr sliceLoader)
    : InputStream{
          getInstance(communicator).get(),
          getInstance(communicator)->defaultsAndOverrides()->defaultEncoding,
          Buffer{p.first, p.second},
          std::move(sliceLoader)}
{
}

Ice::InputStream::InputStream(
    const CommunicatorPtr& communicator,
    EncodingVersion encoding,
    const vector<byte>& v,
    SliceLoaderPtr sliceLoader)
    : InputStream{getInstance(communicator).get(), encoding, Buffer{v}, std::move(sliceLoader)}
{
}

Ice::InputStream::InputStream(
    const CommunicatorPtr& communicator,
    EncodingVersion encoding,
    pair<const byte*, const byte*> p,
    SliceLoaderPtr sliceLoader)
    : InputStream{getInstance(communicator).get(), encoding, Buffer{p.first, p.second}, std::move(sliceLoader)}
{
}

Ice::InputStream::InputStream(
    Instance* instance,
    EncodingVersion encoding,
    Buffer& buf,
    bool adopt,
    SliceLoaderPtr sliceLoader)
    : InputStream{instance, encoding, Buffer{buf, adopt}, std::move(sliceLoader)}
{
}

Ice::InputStream::InputStream(InputStream&& other) noexcept
    // only moves (and resets) the base class
    : InputStream{other._instance, other._encoding, std::move(other), other._sliceLoader}
{
    _closure = other._closure;
    _startSeq = other._startSeq;
    _minSeqSize = other._minSeqSize;

    // Reset other to its default state
    other.resetEncapsulation();
    other._closure = nullptr;
    other._startSeq = -1;
    other._minSeqSize = 0;
}

InputStream&
Ice::InputStream::operator=(InputStream&& other) noexcept
{
    assert(_instance == other._instance);

    if (this != &other)
    {
        Buffer::operator=(std::move(other)); // only moves (and resets) the base class

        _encoding = other._encoding;
        _closure = other._closure;
        _startSeq = other._startSeq;
        _minSeqSize = other._minSeqSize;
        _startSeq = -1;
        _minSeqSize = 0;
        resetEncapsulation();

        // Reset other to its default state.
        other.resetEncapsulation();
        other._closure = nullptr;
        other._startSeq = -1;
        other._minSeqSize = 0;
    }
    return *this;
}

void
Ice::InputStream::clear()
{
    while (_currentEncaps && _currentEncaps != &_preAllocatedEncaps)
    {
        Encaps* oldEncaps = _currentEncaps;
        _currentEncaps = _currentEncaps->previous;
        delete oldEncaps;
    }

    _startSeq = -1;
}

void*
Ice::InputStream::getClosure() const
{
    return _closure;
}

void*
Ice::InputStream::setClosure(void* p)
{
    void* prev = _closure;
    _closure = p;
    return prev;
}

void
Ice::InputStream::swap(InputStream& other) noexcept
{
    assert(_instance == other._instance);

    swapBuffer(other);

    std::swap(_encoding, other._encoding);
    std::swap(_closure, other._closure);
    std::swap(_startSeq, other._startSeq);
    std::swap(_minSeqSize, other._minSeqSize);

    //
    // Swap is never called for streams that have encapsulations being read. However,
    // encapsulations might still be set in case unmarshaling failed. We just
    // reset the encapsulations if there are still some set.
    //
    resetEncapsulation();
    other.resetEncapsulation();
}

void
Ice::InputStream::resetEncapsulation()
{
    while (_currentEncaps && _currentEncaps != &_preAllocatedEncaps)
    {
        Encaps* oldEncaps = _currentEncaps;
        _currentEncaps = _currentEncaps->previous;
        delete oldEncaps;
    }

    _preAllocatedEncaps.reset();
}

const EncodingVersion&
Ice::InputStream::startEncapsulation()
{
    Encaps* oldEncaps = _currentEncaps;
    if (!oldEncaps) // First allocated encaps?
    {
        _currentEncaps = &_preAllocatedEncaps;
    }
    else
    {
        _currentEncaps = new Encaps();
        _currentEncaps->previous = oldEncaps;
    }
    _currentEncaps->start = static_cast<size_t>(i - b.begin());

    //
    // I don't use readSize() and writeSize() for encapsulations,
    // because when creating an encapsulation, I must know in advance
    // how many bytes the size information will require in the data
    // stream. If I use an Int, it is always 4 bytes. For
    // readSize()/writeSize(), it could be 1 or 5 bytes.
    //
    std::int32_t sz;
    read(sz);
    if (sz < 6)
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    if (i - sizeof(std::int32_t) + sz > b.end())
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    _currentEncaps->sz = sz;

    read(_currentEncaps->encoding);
    IceInternal::checkSupportedEncoding(_currentEncaps->encoding); // Make sure the encoding is supported

    return _currentEncaps->encoding;
}

void
Ice::InputStream::endEncapsulation()
{
    assert(_currentEncaps);

    if (_currentEncaps->encoding != Encoding_1_0)
    {
        skipOptionals();
        if (i != b.begin() + _currentEncaps->start + _currentEncaps->sz)
        {
            throw MarshalException{__FILE__, __LINE__, "failed to unmarshal encapsulation"};
        }
    }
    else if (i != b.begin() + _currentEncaps->start + _currentEncaps->sz)
    {
        if (i + 1 != b.begin() + _currentEncaps->start + _currentEncaps->sz)
        {
            throw MarshalException{__FILE__, __LINE__, "failed to unmarshal encapsulation"};
        }

        //
        // Ice version < 3.3 had a bug where user exceptions with
        // class members could be encoded with a trailing byte
        // when dispatched with AMD. So we tolerate an extra byte
        // in the encapsulation.
        //
        ++i;
    }

    Encaps* oldEncaps = _currentEncaps;
    _currentEncaps = _currentEncaps->previous;
    if (oldEncaps == &_preAllocatedEncaps)
    {
        oldEncaps->reset();
    }
    else
    {
        delete oldEncaps;
    }
}

EncodingVersion
Ice::InputStream::skipEmptyEncapsulation()
{
    std::int32_t sz;
    read(sz);
    if (sz < 6)
    {
        throw MarshalException{__FILE__, __LINE__, to_string(sz) + " is not a valid encapsulation size"};
    }
    if (i - sizeof(std::int32_t) + sz > b.end())
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    Ice::EncodingVersion encoding;
    read(encoding);
    IceInternal::checkSupportedEncoding(encoding); // Make sure the encoding is supported

    if (encoding == Ice::Encoding_1_0)
    {
        if (sz != static_cast<std::int32_t>(sizeof(std::int32_t)) + 2)
        {
            throw MarshalException{
                __FILE__,
                __LINE__,
                to_string(sz) + " is not a valid encapsulation size for a 1.0 empty encapsulation"};
        }
    }
    else
    {
        // Skip the optional content of the encapsulation if we are expecting an
        // empty encapsulation.
        i += static_cast<size_t>(sz) - sizeof(std::int32_t) - 2;
    }
    return encoding;
}

EncodingVersion
Ice::InputStream::readEncapsulation(const std::byte*& v, std::int32_t& sz)
{
    EncodingVersion encoding;
    v = i;
    read(sz);
    if (sz < 6)
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    if (i - sizeof(std::int32_t) + sz > b.end())
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }

    read(encoding);
    i += static_cast<size_t>(sz) - sizeof(std::int32_t) - 2;
    return encoding;
}

int32_t
Ice::InputStream::getEncapsulationSize()
{
    assert(_currentEncaps);
    return _currentEncaps->sz - static_cast<int32_t>(sizeof(int32_t)) - 2;
}

EncodingVersion
Ice::InputStream::skipEncapsulation()
{
    int32_t sz;
    read(sz);
    if (sz < 6)
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    if (i - sizeof(int32_t) + sz > b.end())
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    EncodingVersion encoding;
    read(encoding.major);
    read(encoding.minor);
    i += static_cast<size_t>(sz) - sizeof(int32_t) - 2;
    return encoding;
}

void
Ice::InputStream::readPendingValues()
{
    if (_currentEncaps && _currentEncaps->decoder)
    {
        _currentEncaps->decoder->readPendingValues();
    }
    else if (getEncoding() == Ice::Encoding_1_0)
    {
        //
        // If using the 1.0 encoding and no instances were read, we
        // still read an empty sequence of pending instances if
        // requested (i.e.: if this is called).
        //
        // This is required by the 1.0 encoding, even if no instances
        // are written we do marshal an empty sequence if marshaled
        // data types use classes.
        //
        skipSize();
    }
}

int32_t
Ice::InputStream::readAndCheckSeqSize(int minSize)
{
    int32_t sz = readSize();

    if (sz == 0)
    {
        return sz;
    }

    //
    // The _startSeq variable points to the start of the sequence for which
    // we expect to read at least _minSeqSize bytes from the stream.
    //
    // If not initialized or if we already read more data than _minSeqSize,
    // we reset _startSeq and _minSeqSize for this sequence (possibly a
    // top-level sequence or enclosed sequence it doesn't really matter).
    //
    // Otherwise, we are reading an enclosed sequence and we have to bump
    // _minSeqSize by the minimum size that this sequence will require on
    // the stream.
    //
    // The goal of this check is to ensure that when we start un-marshaling
    // a new sequence, we check the minimal size of this new sequence against
    // the estimated remaining buffer size. This estimation is based on
    // the minimum size of the enclosing sequences, it's _minSeqSize.
    //
    if (_startSeq == -1 || i > (b.begin() + _startSeq + _minSeqSize))
    {
        _startSeq = static_cast<int>(i - b.begin());
        _minSeqSize = sz * minSize;
    }
    else
    {
        _minSeqSize += sz * minSize;
    }

    //
    // If there isn't enough data to read on the stream for the sequence (and
    // possibly enclosed sequences), something is wrong with the marshaled
    // data: it's claiming having more data that what is possible to read.
    //
    if (_startSeq + _minSeqSize > static_cast<int>(b.size()))
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }

    return sz;
}

void
Ice::InputStream::readBlob(vector<byte>& v, int32_t sz)
{
    if (sz > 0)
    {
        if (b.end() - i < sz)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        vector<byte>(i, i + sz).swap(v);
        i += sz;
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(std::vector<byte>& v)
{
    std::pair<const byte*, const byte*> p;
    read(p);
    if (p.first != p.second)
    {
        v.resize(static_cast<size_t>(p.second - p.first));
        copy(p.first, p.second, v.begin());
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(pair<const byte*, const byte*>& v)
{
    int32_t sz = readAndCheckSeqSize(1);
    if (sz > 0)
    {
        v.first = i;
        v.second = i + sz;
        i += sz;
    }
    else
    {
        v.first = v.second = i;
    }
}

void
Ice::InputStream::read(vector<bool>& v)
{
    int32_t sz = readAndCheckSeqSize(1);
    if (sz > 0)
    {
        v.resize(static_cast<size_t>(sz));
        copy(reinterpret_cast<uint8_t*>(i), reinterpret_cast<uint8_t*>(i) + sz, v.begin());
        i += sz;
    }
    else
    {
        v.clear();
    }
}

namespace
{
    template<size_t boolSize> struct ReadBoolHelper
    {
        static bool* read(pair<const bool*, const bool*>& v, int32_t sz, InputStream::Container::iterator& i)
        {
            bool* array = new bool[static_cast<size_t>(sz)];
            for (int idx = 0; idx < sz; ++idx)
            {
                array[idx] = static_cast<bool>(*(i + idx));
            }
            v.first = array;
            v.second = array + sz;
            return array;
        }
    };

    template<> struct ReadBoolHelper<1>
    {
        static bool* read(pair<const bool*, const bool*>& v, int32_t sz, InputStream::Container::iterator& i)
        {
            v.first = reinterpret_cast<bool*>(i);
            v.second = reinterpret_cast<bool*>(i) + sz;
            return nullptr;
        }
    };
}

void
Ice::InputStream::read(pair<const bool*, const bool*>& v)
{
    int32_t sz = readAndCheckSeqSize(1);
    if (sz > 0)
    {
        auto boolArray = ReadBoolHelper<sizeof(bool)>::read(v, sz, i);
        if (boolArray)
        {
            _deleters.emplace_back([boolArray] { delete[] boolArray; });
        }
        i += sz;
    }
    else
    {
        v.first = v.second = reinterpret_cast<bool*>(i);
    }
}

void
Ice::InputStream::read(int16_t& v)
{
    if (b.end() - i < static_cast<int>(sizeof(int16_t)))
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const byte* src = &(*i);
    i += sizeof(int16_t);
    if constexpr (endian::native == endian::big)
    {
        byte* dest = reinterpret_cast<byte*>(&v) + sizeof(int16_t) - 1;
        *dest-- = *src++;
        *dest = *src;
    }
    else
    {
        byte* dest = reinterpret_cast<byte*>(&v);
        *dest++ = *src++;
        *dest = *src;
    }
}

void
Ice::InputStream::read(vector<int16_t>& v)
{
    int32_t sz = readAndCheckSeqSize(static_cast<int>(sizeof(int16_t)));
    if (sz > 0)
    {
        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(int16_t));
        v.resize(static_cast<size_t>(sz));
        if constexpr (endian::native == endian::big)
        {
            const byte* src = &(*begin);
            byte* dest = reinterpret_cast<byte*>(&v[0]) + sizeof(int16_t) - 1;
            for (int j = 0; j < sz; ++j)
            {
                *dest-- = *src++;
                *dest-- = *src++;
                dest += 2 * sizeof(int16_t);
            }
        }
        else
        {
            copy(begin, i, reinterpret_cast<byte*>(&v[0]));
        }
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(int32_t& v)
{
    if (b.end() - i < static_cast<int>(sizeof(int32_t)))
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const byte* src = &(*i);
    i += sizeof(int32_t);
    if constexpr (endian::native == endian::big)
    {
        byte* dest = reinterpret_cast<byte*>(&v) + sizeof(int32_t) - 1;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest = *src;
    }
    else
    {
        byte* dest = reinterpret_cast<byte*>(&v);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;
    }
}

void
Ice::InputStream::read(vector<int32_t>& v)
{
    int32_t sz = readAndCheckSeqSize(static_cast<int>(sizeof(int32_t)));
    if (sz > 0)
    {
        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(int32_t));
        v.resize(static_cast<size_t>(sz));
        if constexpr (endian::native == endian::big)
        {
            const byte* src = &(*begin);
            byte* dest = reinterpret_cast<byte*>(&v[0]) + sizeof(int32_t) - 1;
            for (int j = 0; j < sz; ++j)
            {
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                dest += 2 * sizeof(int32_t);
            }
        }
        else
        {
            copy(begin, i, reinterpret_cast<byte*>(&v[0]));
        }
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(int64_t& v)
{
    if (b.end() - i < static_cast<int>(sizeof(int64_t)))
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const byte* src = &(*i);
    i += sizeof(int64_t);
    if constexpr (endian::native == endian::big)
    {
        byte* dest = reinterpret_cast<byte*>(&v) + sizeof(int64_t) - 1;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest = *src;
    }
    else
    {
        byte* dest = reinterpret_cast<byte*>(&v);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;
    }
}

void
Ice::InputStream::read(vector<int64_t>& v)
{
    int32_t sz = readAndCheckSeqSize(static_cast<int>(sizeof(int64_t)));
    if (sz > 0)
    {
        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(int64_t));
        v.resize(static_cast<size_t>(sz));
        if constexpr (endian::native == endian::big)
        {
            const byte* src = &(*begin);
            byte* dest = reinterpret_cast<byte*>(&v[0]) + sizeof(int64_t) - 1;
            for (int j = 0; j < sz; ++j)
            {
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                dest += 2 * sizeof(int64_t);
            }
        }
        else
        {
            copy(begin, i, reinterpret_cast<byte*>(&v[0]));
        }
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(float& v)
{
    if (b.end() - i < static_cast<int>(sizeof(float)))
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const byte* src = &(*i);
    i += sizeof(float);
    if constexpr (endian::native == endian::big)
    {
        byte* dest = reinterpret_cast<byte*>(&v) + sizeof(float) - 1;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest = *src;
    }
    else
    {
        byte* dest = reinterpret_cast<byte*>(&v);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;
    }
}

void
Ice::InputStream::read(vector<float>& v)
{
    int32_t sz = readAndCheckSeqSize(static_cast<int>(sizeof(float)));
    if (sz > 0)
    {
        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(float));
        v.resize(static_cast<size_t>(sz));
        if constexpr (endian::native == endian::big)
        {
            const byte* src = &(*begin);
            byte* dest = reinterpret_cast<byte*>(&v[0]) + sizeof(float) - 1;
            for (int j = 0; j < sz; ++j)
            {
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                dest += 2 * sizeof(float);
            }
        }
        else
        {
            copy(begin, i, reinterpret_cast<byte*>(&v[0]));
        }
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(double& v)
{
    if (b.end() - i < static_cast<int>(sizeof(double)))
    {
        throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
    }
    const byte* src = &(*i);
    i += sizeof(double);
    if constexpr (endian::native == endian::big)
    {
        byte* dest = reinterpret_cast<byte*>(&v) + sizeof(double) - 1;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest = *src;
    }
    else
    {
        byte* dest = reinterpret_cast<byte*>(&v);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;
    }
}

void
Ice::InputStream::read(vector<double>& v)
{
    int32_t sz = readAndCheckSeqSize(static_cast<int>(sizeof(double)));
    if (sz > 0)
    {
        Container::iterator begin = i;
        i += sz * static_cast<int>(sizeof(double));
        v.resize(static_cast<size_t>(sz));
        if constexpr (endian::native == endian::big)
        {
            const byte* src = &(*begin);
            byte* dest = reinterpret_cast<byte*>(&v[0]) + sizeof(double) - 1;
            for (int j = 0; j < sz; ++j)
            {
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                *dest-- = *src++;
                dest += 2 * sizeof(double);
            }
        }
        else
        {
            copy(begin, i, reinterpret_cast<byte*>(&v[0]));
        }
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(std::string& v, bool convert)
{
    int32_t sz = readSize();
    if (sz > 0)
    {
        if (b.end() - i < sz)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        if (!convert || !readConverted(v, sz))
        {
            string(reinterpret_cast<const char*>(&*i), reinterpret_cast<const char*>(&*i) + sz).swap(v);
        }
        i += sz;
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(const char*& vdata, size_t& vsize, bool convert)
{
    int sz = readSize();
    if (sz > 0)
    {
        if (b.end() - i < sz)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        if (convert == false)
        {
            vdata = reinterpret_cast<const char*>(&*i);
            vsize = static_cast<size_t>(sz);
            i += sz;
        }
        else
        {
            string converted;
            if (readConverted(converted, sz))
            {
                if (converted.size() <= static_cast<size_t>(sz))
                {
                    //
                    // Write converted string directly into buffer
                    //
                    std::memcpy(i, converted.data(), converted.size());
                    vdata = reinterpret_cast<const char*>(&*i);
                    vsize = converted.size();
                }
                else
                {
                    auto holder = new string(std::move(converted));
                    _deleters.emplace_back([holder] { delete holder; });
                    vdata = holder->data();
                    vsize = holder->size();
                }
            }
            else
            {
                vdata = reinterpret_cast<const char*>(&*i);
                vsize = static_cast<size_t>(sz);
            }
            i += sz;
        }
    }
    else
    {
        vdata = nullptr;
        vsize = 0;
    }
}

bool
Ice::InputStream::readConverted(string& v, int sz)
{
    try
    {
        bool converted = false;
        const StringConverterPtr& stringConverter = _instance->getStringConverter();
        if (stringConverter)
        {
            stringConverter->fromUTF8(i, i + sz, v);
            converted = true;
        }

        return converted;
    }
    catch (const IllegalConversionException& ex)
    {
        throw MarshalException{__FILE__, __LINE__, string{"failed to unmarshal a string:\n"} + ex.what()};
    }
}

void
Ice::InputStream::read(vector<string>& v, bool convert)
{
    int32_t sz = readAndCheckSeqSize(1);
    if (sz > 0)
    {
        v.resize(static_cast<size_t>(sz));
        for (size_t j = 0; j < static_cast<size_t>(sz); ++j)
        {
            read(v[j], convert);
        }
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(wstring& v)
{
    int32_t sz = readSize();
    if (sz > 0)
    {
        if (b.end() - i < sz)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        try
        {
            const WstringConverterPtr& wstringConverter = _instance->getWstringConverter();
            wstringConverter->fromUTF8(i, i + sz, v);
            i += sz;
        }
        catch (const IllegalConversionException& ex)
        {
            throw MarshalException{__FILE__, __LINE__, string{"failed to unmarshal a string:\n"} + ex.what()};
        }
    }
    else
    {
        v.clear();
    }
}

void
Ice::InputStream::read(vector<wstring>& v)
{
    auto sz = static_cast<size_t>(readAndCheckSeqSize(1));
    if (sz > 0)
    {
        v.resize(sz);
        for (size_t j = 0; j < sz; ++j)
        {
            read(v[j]);
        }
    }
    else
    {
        v.clear();
    }
}

namespace
{
    inline Ice::SliceLoaderPtr getSliceLoader(SliceLoaderPtr sliceLoader, Instance* instance)
    {
        if (!sliceLoader)
        {
            sliceLoader = instance->sliceLoader();
        }
        return sliceLoader;
    }
}

Ice::InputStream::InputStream(Instance* instance, EncodingVersion encoding, Buffer&& buf, SliceLoaderPtr sliceLoader)
    : Buffer(std::move(buf)),
      _instance(instance),
      _encoding(encoding),
      _classGraphDepthMax(instance->classGraphDepthMax()),
      _sliceLoader{getSliceLoader(std::move(sliceLoader), instance)}
{
}

ReferencePtr
Ice::InputStream::readReference()
{
    Identity ident;
    read(ident);
    if (ident.name.empty())
    {
        return nullptr;
    }
    else
    {
        return _instance->referenceFactory()->create(std::move(ident), this);
    }
}

int32_t
Ice::InputStream::readEnum(int32_t maxValue)
{
    if (getEncoding() == Encoding_1_0)
    {
        if (maxValue < 127)
        {
            uint8_t value;
            read(value);
            return value;
        }
        else if (maxValue < 32767)
        {
            int16_t value;
            read(value);
            return value;
        }
        else
        {
            int32_t value;
            read(value);
            return value;
        }
    }
    else
    {
        return readSize();
    }
}

void
Ice::InputStream::throwException(UserExceptionFactory factory)
{
    initEncaps();
    _currentEncaps->decoder->throwException(std::move(factory));
}

bool
Ice::InputStream::readOptImpl(int32_t readTag, OptionalFormat expectedFormat)
{
    if (getEncoding() == Encoding_1_0)
    {
        return false; // Optional members aren't supported with the 1.0 encoding.
    }

    while (true)
    {
        if (i >= b.begin() + _currentEncaps->start + _currentEncaps->sz)
        {
            return false; // End of encapsulation also indicates end of optionals.
        }

        uint8_t v;
        read(v);
        if (v == OPTIONAL_END_MARKER)
        {
            --i; // Rewind
            return false;
        }

        auto format = static_cast<OptionalFormat>(v & 0x07); // First 3 bits.
        auto tag = static_cast<int32_t>(v >> 3);
        if (tag == 30)
        {
            tag = readSize();
        }

        if (tag > readTag)
        {
            i -= tag < 30 ? 1 : (tag < 255 ? 2 : 6); // Rewind
            return false;                            // No optional data members with the requested tag.
        }
        else if (tag < readTag)
        {
            skipOptional(format); // Skip optional data members
        }
        else
        {
            if (format != expectedFormat)
            {
                ostringstream os;
                os << "invalid optional data member '" << tag << "': unexpected format";
                throw MarshalException(__FILE__, __LINE__, os.str());
            }
            return true;
        }
    }
    return true; // Keep the compiler happy.
}

void
Ice::InputStream::skipOptional(OptionalFormat type)
{
    switch (type)
    {
        case OptionalFormat::F1:
        {
            skip(1);
            break;
        }
        case OptionalFormat::F2:
        {
            skip(2);
            break;
        }
        case OptionalFormat::F4:
        {
            skip(4);
            break;
        }
        case OptionalFormat::F8:
        {
            skip(8);
            break;
        }
        case OptionalFormat::Size:
        {
            skipSize();
            break;
        }
        case OptionalFormat::VSize:
        {
            skip(static_cast<size_t>(readSize()));
            break;
        }
        case OptionalFormat::FSize:
        {
            int32_t sz;
            read(sz);
            if (sz < 0)
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            skip(static_cast<size_t>(sz));
            break;
        }
        case OptionalFormat::Class:
        {
            throw MarshalException{__FILE__, __LINE__, "cannot skip optional class"};
        }
    }
}

void
Ice::InputStream::skipOptionals()
{
    //
    // Skip remaining un-read optional members.
    //
    while (true)
    {
        if (i >= b.begin() + _currentEncaps->start + _currentEncaps->sz)
        {
            return; // End of encapsulation also indicates end of optionals.
        }

        uint8_t v;
        read(v);
        if (v == OPTIONAL_END_MARKER)
        {
            return;
        }

        auto format = static_cast<OptionalFormat>(v & 0x07); // Read first 3 bits.
        if (static_cast<int32_t>(v >> 3) == 30)
        {
            skipSize();
        }
        skipOptional(format);
    }
}

void
Ice::InputStream::throwUnmarshalOutOfBoundsException(const char* file, int line)
{
    throw MarshalException{file, line, endOfBufferMessage};
}

void
Ice::InputStream::traceSkipSlice(string_view typeId, SliceType sliceType) const
{
    assert(_instance->initializationData().logger); // not null once the communicator is initialized
    if (_instance->traceLevels()->slicing > 0)
    {
        traceSlicing(
            sliceType == ExceptionSlice ? "exception" : "object",
            typeId,
            "Slicing",
            _instance->initializationData().logger);
    }
}

void
Ice::InputStream::initEncaps()
{
    if (!_currentEncaps) // Lazy initialization.
    {
        _currentEncaps = &_preAllocatedEncaps;
        _currentEncaps->encoding = _encoding;
        _currentEncaps->sz = static_cast<int32_t>(b.size());
    }

    if (!_currentEncaps->decoder) // Lazy initialization.
    {
        if (_currentEncaps->encoding == Encoding_1_0)
        {
            _currentEncaps->decoder = new EncapsDecoder10(this, _currentEncaps, _classGraphDepthMax);
        }
        else
        {
            _currentEncaps->decoder = new EncapsDecoder11(this, _currentEncaps, _classGraphDepthMax);
        }
    }
}

// Out of line to avoid weak vtable
Ice::InputStream::EncapsDecoder::~EncapsDecoder() = default;

string
Ice::InputStream::EncapsDecoder::readTypeId(bool isIndex)
{
    if (isIndex)
    {
        int32_t index = _stream->readSize();
        auto k = _typeIdMap.find(index);
        if (k == _typeIdMap.end())
        {
            throw MarshalException{__FILE__, __LINE__, endOfBufferMessage};
        }
        return k->second;
    }
    else
    {
        string typeId;
        _stream->read(typeId, false);
        _typeIdMap.insert(make_pair(++_typeIdIndex, typeId));
        return typeId;
    }
}

ValuePtr
Ice::InputStream::EncapsDecoder::newInstance(string_view typeId)
{
    return _stream->_sliceLoader->newClassInstance(typeId);
}

void
Ice::InputStream::EncapsDecoder::addPatchEntry(int32_t index, const PatchFunc& patchFunc, void* patchAddr)
{
    assert(index > 0);

    //
    // Check if we already unmarshaled the object. If that's the case, just patch the object smart
    // pointer and we're done. A null value indicates we've encountered a cycle and Ice.AllowClassCycles
    // is false.
    //
    auto p = _unmarshaledMap.find(index);
    if (p != _unmarshaledMap.end())
    {
        if (p->second == nullptr)
        {
            assert(!_stream->_instance->acceptClassCycles());
            throw MarshalException(__FILE__, __LINE__, "cycle detected during Value unmarshaling");
        }
        patchFunc(patchAddr, p->second);
        return;
    }

    //
    // Add a patch entry if the object isn't unmarshaled yet, the
    // smart pointer will be patched when the instance is
    // unmarshaled.
    //

    auto q = _patchMap.find(index);
    if (q == _patchMap.end())
    {
        //
        // We have no outstanding instances to be patched for this
        // index, so make a new entry in the patch map.
        //
        q = _patchMap.insert(make_pair(index, PatchList())).first;
    }

    //
    // Append a patch entry for this instance.
    //
    PatchEntry e;
    e.patchFunc = patchFunc;
    e.patchAddr = patchAddr;
    e.classGraphDepth = _classGraphDepth;
    q->second.push_back(e);
}

void
Ice::InputStream::EncapsDecoder::unmarshal(int32_t index, const ValuePtr& v)
{
    //
    // Add the object to the map of unmarshaled instances, this must
    // be done before reading the instances (for circular references).
    //
    // If circular references are not allowed we insert null (for cycle detection) and add
    // the object to the map once it has been fully unmarshaled.
    //
    _unmarshaledMap.insert(make_pair(index, _stream->_instance->acceptClassCycles() ? v : nullptr));

    //
    // Read the object.
    //
    v->_iceRead(_stream);

    //
    // Patch all instances now that the object is unmarshaled.
    //
    auto patchPos = _patchMap.find(index);
    if (patchPos != _patchMap.end())
    {
        assert(patchPos->second.size() > 0);

        //
        // Patch all pointers that refer to the instance.
        //
        for (const auto& k : patchPos->second)
        {
            k.patchFunc(k.patchAddr, v);
        }

        //
        // Clear out the patch map for that index -- there is nothing left
        // to patch for that index for the time being.
        //
        _patchMap.erase(patchPos);
    }

    if (_valueList.empty() && _patchMap.empty())
    {
        v->ice_postUnmarshal();
    }
    else
    {
        _valueList.push_back(v);

        if (_patchMap.empty())
        {
            // Iterate over the value list and invoke ice_postUnmarshal on
            // each value. We must do this after all values have been
            // unmarshaled in order to ensure that any value data members
            // have been properly patched.
            for (const auto& value : _valueList)
            {
                value->ice_postUnmarshal();
            }
            _valueList.clear();
        }
    }

    if (!_stream->_instance->acceptClassCycles())
    {
        // This class has been fully unmarshaled without creating any cycles
        // It can be added to the map now.
        _unmarshaledMap[index] = v;
    }
}

void
Ice::InputStream::EncapsDecoder10::read(PatchFunc patchFunc, void* patchAddr)
{
    assert(patchFunc && patchAddr);

    //
    // Object references are encoded as a negative integer in 1.0.
    //
    int32_t index;
    _stream->read(index);
    if (index > 0)
    {
        throw MarshalException(__FILE__, __LINE__, "invalid object id");
    }
    index = -index;

    if (index == 0)
    {
        //
        // Calling the patch function for null instances is necessary for correct functioning of Ice for
        // Python and Ruby.
        //
        patchFunc(patchAddr, nullptr);
    }
    else
    {
        addPatchEntry(index, patchFunc, patchAddr);
    }
}

void
Ice::InputStream::EncapsDecoder10::throwException(UserExceptionFactory exceptionFactory)
{
    assert(_sliceType == NoSlice);

    //
    // User exception with the 1.0 encoding start with a boolean flag
    // that indicates whether or not the exception has classes.
    //
    // This allows reading the pending values even if some part of
    // the exception was sliced.
    //
    bool usesClasses;
    _stream->read(usesClasses);

    _sliceType = ExceptionSlice;
    _skipFirstSlice = false;

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _typeId;
    while (true)
    {
        //
        // Look for a statically-generated factory for this ID.
        //
        if (!exceptionFactory)
        {
            std::exception_ptr exceptionPtr = _stream->_sliceLoader->newExceptionInstance(_typeId);

            if (exceptionPtr)
            {
                exceptionFactory = [exceptionPtr](string_view) { std::rethrow_exception(exceptionPtr); };
            }
        }

        //
        // We found a factory, we get out of this loop.
        // A factory that doesn't throw is equivalent to a null factory.
        //
        if (exceptionFactory)
        {
            //
            // Got factory -- ask the factory to instantiate the
            // exception, initialize the exception members, and throw
            // the exception.
            //
            try
            {
                exceptionFactory(_typeId);
            }
            catch (UserException& ex)
            {
                ex._read(_stream);
                if (usesClasses)
                {
                    readPendingValues();
                }
                throw;
            }
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice();
        try
        {
            startSlice();
        }
        catch (const MarshalException&)
        {
            //
            // An oversight in the 1.0 encoding means there is no marker to indicate
            // the last slice of an exception. As a result, we just try to read the
            // next type ID, which raises MarshalException when the
            // input buffer underflows.
            throw MarshalException{__FILE__, __LINE__, "unknown exception type '" + mostDerivedId + "'"};
        }
    }
}

void
Ice::InputStream::EncapsDecoder10::startInstance([[maybe_unused]] SliceType sliceType)
{
    assert(_sliceType == sliceType);
    _skipFirstSlice = true;
}

SlicedDataPtr
Ice::InputStream::EncapsDecoder10::endInstance()
{
    //
    // Read the Ice::Value slice.
    //
    if (_sliceType == ValueSlice)
    {
        startSlice();
        int32_t sz = _stream->readSize(); // For compatibility with the old AFM.
        if (sz != 0)
        {
            throw MarshalException(__FILE__, __LINE__, "invalid Object slice");
        }
        endSlice();
    }
    _sliceType = NoSlice;
    return nullptr;
}

void
Ice::InputStream::EncapsDecoder10::startSlice()
{
    //
    // If first slice, don't read the header, it was already read in
    // readInstance or throwException to find the factory.
    //
    if (_skipFirstSlice)
    {
        _skipFirstSlice = false;
        return;
    }

    //
    // For values, first read the type ID boolean which indicates
    // whether or not the type ID is encoded as a string or as an
    // index. For exceptions, the type ID is always encoded as a
    // string.
    //
    if (_sliceType == ValueSlice)
    {
        bool isIndex;
        _stream->read(isIndex);
        _typeId = readTypeId(isIndex);
    }
    else
    {
        _stream->read(_typeId, false);
    }

    _stream->read(_sliceSize);
    if (_sliceSize < 4)
    {
        throw MarshalException{__FILE__, __LINE__, endOfBufferMessage};
    }
}

void
Ice::InputStream::EncapsDecoder10::endSlice()
{
}

void
Ice::InputStream::EncapsDecoder10::skipSlice()
{
    _stream->traceSkipSlice(_typeId, _sliceType);
    assert(_sliceSize >= 4);
    _stream->skip(static_cast<size_t>(_sliceSize) - sizeof(int32_t));
}

void
Ice::InputStream::EncapsDecoder10::readPendingValues()
{
    int32_t num;
    do
    {
        num = _stream->readSize();
        for (int32_t k = num; k > 0; --k)
        {
            readInstance();
        }
    } while (num);

    if (!_patchMap.empty())
    {
        //
        // If any entries remain in the patch map, the sender has sent an index for an object, but
        // failed to supply the object.
        //
        throw MarshalException(__FILE__, __LINE__, "index for class received, but no instance");
    }
}

void
Ice::InputStream::EncapsDecoder10::readInstance()
{
    int32_t index;
    _stream->read(index);

    if (index <= 0)
    {
        throw MarshalException(__FILE__, __LINE__, "invalid object id");
    }

    _sliceType = ValueSlice;
    _skipFirstSlice = false;

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _typeId;
    shared_ptr<Value> v;
    while (true)
    {
        // For the 1.0 encoding, the type ID for the base Object class marks the last slice.
        if (_typeId == Value::ice_staticId())
        {
            throw MarshalException{
                __FILE__,
                __LINE__,
                "the Slice loader did not find a class for type ID '" + mostDerivedId + "'"};
        }

        v = newInstance(_typeId);

        //
        // We found a factory, we get out of this loop.
        //
        if (v)
        {
            break;
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice();
        startSlice(); // Read next Slice header for next iteration.
    }

    //
    // Compute the biggest class graph depth of this object. To compute this,
    // we get the class graph depth of each ancestor from the patch map and
    // keep the biggest one.
    //
    _classGraphDepth = 0;
    auto patchPos = _patchMap.find(index);
    if (patchPos != _patchMap.end())
    {
        assert(patchPos->second.size() > 0);
        for (const auto& k : patchPos->second)
        {
            if (k.classGraphDepth > _classGraphDepth)
            {
                _classGraphDepth = k.classGraphDepth;
            }
        }
    }

    if (++_classGraphDepth > _classGraphDepthMax)
    {
        throw MarshalException(__FILE__, __LINE__, "maximum class graph depth reached");
    }

    //
    // Unmarshal the instance and add it to the map of unmarshaled instances.
    //
    unmarshal(index, v);
}

void
Ice::InputStream::EncapsDecoder11::read(PatchFunc patchFunc, void* patchAddr)
{
    assert(patchFunc && patchAddr); // we used to support null for optional classes

    int32_t index = _stream->readSize();
    if (index < 0)
    {
        throw MarshalException(__FILE__, __LINE__, "invalid object id");
    }
    else if (index == 0)
    {
        //
        // Calling the patch function for null instances is necessary for correct functioning of Ice for
        // Python and Ruby.
        //
        patchFunc(patchAddr, nullptr);
    }
    else if (_current && _current->sliceFlags & FLAG_HAS_INDIRECTION_TABLE)
    {
        //
        // When reading an object within a slice and there's an
        // indirect object table, always read an indirect reference
        // that points to an object from the indirect object table
        // marshaled at the end of the Slice.
        //
        // Maintain a list of indirect references. Note that the
        // indirect index starts at 1, so we decrement it by one to
        // derive an index into the indirection table that we'll read
        // at the end of the slice.
        //
        IndirectPatchEntry e;
        e.index = index - 1;
        e.patchFunc = patchFunc;
        e.patchAddr = patchAddr;
        _current->indirectPatchList.push_back(e);
    }
    else
    {
        readInstance(index, patchFunc, patchAddr);
    }
}

void
Ice::InputStream::EncapsDecoder11::throwException(UserExceptionFactory exceptionFactory)
{
    assert(!_current);

    push(ExceptionSlice);

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _current->typeId;
    while (true)
    {
        //
        // Look for a statically-generated factory for this ID.
        //
        if (!exceptionFactory)
        {
            std::exception_ptr exceptionPtr = _stream->_sliceLoader->newExceptionInstance(_current->typeId);

            if (exceptionPtr)
            {
                exceptionFactory = [exceptionPtr](string_view) { std::rethrow_exception(exceptionPtr); };
            }
        }

        //
        // We found a factory, we get out of this loop.
        // A factory that doesn't throw is equivalent to a null factory.
        //
        if (exceptionFactory)
        {
            //
            // Got factory -- ask the factory to instantiate the
            // exception, initialize the exception members, and throw
            // the exception.
            //
            try
            {
                exceptionFactory(_current->typeId);
            }
            catch (UserException& ex)
            {
                ex._read(_stream);
                throw;
            }
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice();

        //
        // If this is the last slice, raise an exception and stop un-marshaling.
        //
        if (_current->sliceFlags & FLAG_IS_LAST_SLICE)
        {
            throw MarshalException{
                __FILE__,
                __LINE__,
                "cannot unmarshal exception with type ID '" + mostDerivedId + "'"};
        }

        startSlice();
    }
}

void
Ice::InputStream::EncapsDecoder11::startInstance([[maybe_unused]] SliceType sliceType)
{
    assert(_current->sliceType == sliceType);
    _current->skipFirstSlice = true;
}

SlicedDataPtr
Ice::InputStream::EncapsDecoder11::endInstance()
{
    SlicedDataPtr slicedData = readSlicedData();
    _current->indirectionTables.clear();
    _current->slices.clear();
    _current = _current->previous;
    return slicedData;
}

void
Ice::InputStream::EncapsDecoder11::startSlice()
{
    //
    // If first slice, don't read the header, it was already read in
    // readInstance or throwException to find the factory.
    //
    if (_current->skipFirstSlice)
    {
        _current->skipFirstSlice = false;
        return;
    }

    _stream->read(_current->sliceFlags);

    //
    // Read the type ID, for value slices the type ID is encoded as a
    // string or as an index, for exceptions it's always encoded as a
    // string.
    //
    if (_current->sliceType == ValueSlice)
    {
        if ((_current->sliceFlags & FLAG_HAS_TYPE_ID_COMPACT) == FLAG_HAS_TYPE_ID_COMPACT) // Must be checked first!
        {
            _current->compactId = _stream->readSize();
            _current->typeId = std::to_string(_current->compactId);
        }
        else if (_current->sliceFlags & (FLAG_HAS_TYPE_ID_STRING | FLAG_HAS_TYPE_ID_INDEX))
        {
            _current->typeId = readTypeId(_current->sliceFlags & FLAG_HAS_TYPE_ID_INDEX);
            _current->compactId = -1;
        }
        else
        {
            // Only the most derived slice encodes the type ID for the compact format.
            _current->typeId.clear();
            _current->compactId = -1;
        }
    }
    else
    {
        _stream->read(_current->typeId, false);
        _current->compactId = -1;
    }

    //
    // Read the slice size if necessary.
    //
    if (_current->sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        _stream->read(_current->sliceSize);
        if (_current->sliceSize < 4)
        {
            throw MarshalException{__FILE__, __LINE__, endOfBufferMessage};
        }
    }
    else
    {
        _current->sliceSize = 0;
    }
}

void
Ice::InputStream::EncapsDecoder11::endSlice()
{
    if (_current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS)
    {
        _stream->skipOptionals();
    }

    //
    // Read the indirect object table if one is present.
    //
    if (_current->sliceFlags & FLAG_HAS_INDIRECTION_TABLE)
    {
        IndexList indirectionTable(static_cast<size_t>(_stream->readAndCheckSeqSize(1)));
        for (auto& p : indirectionTable)
        {
            p = readInstance(_stream->readSize(), nullptr, nullptr);
        }

        //
        // Sanity checks. If there are optional members, it's possible
        // that not all object references were read if they are from
        // unknown optional data members.
        //
        if (indirectionTable.empty())
        {
            throw MarshalException(__FILE__, __LINE__, "empty indirection table");
        }
        if (_current->indirectPatchList.empty() && !(_current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS))
        {
            throw MarshalException(__FILE__, __LINE__, "no references to indirection table");
        }

        //
        // Convert indirect references into direct references.
        //
        IndirectPatchList::iterator p;
        for (p = _current->indirectPatchList.begin(); p != _current->indirectPatchList.end(); ++p)
        {
            assert(p->index >= 0);
            if (p->index >= static_cast<int32_t>(indirectionTable.size()))
            {
                throw MarshalException(__FILE__, __LINE__, "indirection out of range");
            }
            addPatchEntry(indirectionTable[static_cast<size_t>(p->index)], p->patchFunc, p->patchAddr);
        }
        _current->indirectPatchList.clear();
    }
}

void
Ice::InputStream::EncapsDecoder11::skipSlice()
{
    _stream->traceSkipSlice(_current->typeId, _current->sliceType);

    Container::iterator start = _stream->i;

    if (_current->sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        assert(_current->sliceSize >= 4);
        _stream->skip(static_cast<size_t>(_current->sliceSize) - sizeof(int32_t));
    }
    else
    {
        if (_current->sliceType == ValueSlice)
        {
            throw MarshalException{
                __FILE__,
                __LINE__,
                "the Slice loader did not find a class for type ID '" + _current->typeId +
                    "' and compact format prevents slicing"};
        }
        else
        {
            throw MarshalException{
                __FILE__,
                __LINE__,
                "the Slice loader did not find a user exception class for type ID '" + _current->typeId +
                    "' and compact format prevents slicing"};
        }
    }

    //
    // Preserve this slice if unmarshaling a value in Slice format. Exception slices are not preserved.
    //
    if (_current->sliceType == ValueSlice)
    {
        bool hasOptionalMembers = _current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS;
        vector<byte> bytes;
        if (hasOptionalMembers)
        {
            //
            // Don't include the optional member end marker. It will be re-written by
            // endSlice when the sliced data is re-written.
            //
            bytes = vector<byte>(start, _stream->i - 1);
        }
        else
        {
            bytes = vector<byte>(start, _stream->i);
        }

        SliceInfoPtr info = make_shared<SliceInfo>(
            _current->compactId == -1 ? _current->typeId : "",
            _current->compactId,
            std::move(bytes),
            hasOptionalMembers,
            _current->sliceFlags & FLAG_IS_LAST_SLICE);

        _current->slices.push_back(info);
    }

    _current->indirectionTables.emplace_back();

    //
    // Read the indirect object table. We read the instances or their
    // IDs if the instance is a reference to an already un-marshaled
    // object.
    //
    if (_current->sliceFlags & FLAG_HAS_INDIRECTION_TABLE)
    {
        IndexList& table = _current->indirectionTables.back();
        table.resize(static_cast<size_t>(_stream->readAndCheckSeqSize(1)));
        for (auto& entry : table)
        {
            entry = readInstance(_stream->readSize(), nullptr, nullptr);
        }
    }
}

bool
Ice::InputStream::EncapsDecoder11::readOptional(int32_t readTag, Ice::OptionalFormat expectedFormat)
{
    if (!_current)
    {
        return _stream->readOptImpl(readTag, expectedFormat);
    }
    else if (_current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS)
    {
        return _stream->readOptImpl(readTag, expectedFormat);
    }
    return false;
}

int32_t
Ice::InputStream::EncapsDecoder11::readInstance(int32_t index, const PatchFunc& patchFunc, void* patchAddr)
{
    assert(index > 0);

    if (index > 1)
    {
        if (patchFunc)
        {
            addPatchEntry(index, patchFunc, patchAddr);
        }
        return index;
    }

    push(ValueSlice);

    //
    // Get the object ID before we start reading slices. If some
    // slices are skiped, the indirect object table are still read and
    // might read other instances.
    //
    index = ++_valueIdIndex;

    //
    // Read the first slice header.
    //
    startSlice();
    const string mostDerivedId = _current->typeId;
    shared_ptr<Value> v;
    while (true)
    {
        if (!_current->typeId.empty())
        {
            v = newInstance(_current->typeId);

            //
            // We found a factory, we get out of this loop.
            //
            if (v)
            {
                break;
            }
        }

        //
        // Slice off what we don't understand.
        //
        skipSlice();

        //
        // If this is the last slice, keep the object as an opaque UnknownSlicedValue.
        //
        if (_current->sliceFlags & FLAG_IS_LAST_SLICE)
        {
            //
            // Provide a factory with an opportunity to supply the object.
            // We pass the "::Ice::Object" ID to indicate that this is the
            // last chance to preserve the object.
            //
            v = newInstance(Value::ice_staticId());
            if (!v)
            {
                v = make_shared<UnknownSlicedValue>(mostDerivedId);
            }

            break;
        }

        startSlice(); // Read next Slice header for next iteration.
    }

    if (++_classGraphDepth > _classGraphDepthMax)
    {
        throw MarshalException(__FILE__, __LINE__, "maximum class graph depth reached");
    }

    //
    // Unmarshal the object.
    //
    unmarshal(index, v);

    --_classGraphDepth;

    if (!_current && !_patchMap.empty())
    {
        //
        // If any entries remain in the patch map, the sender has sent an index for an object, but
        // failed to supply the object.
        //
        throw MarshalException(__FILE__, __LINE__, "index for class received, but no instance");
    }

    if (patchFunc)
    {
        patchFunc(patchAddr, v);
    }
    return index;
}

SlicedDataPtr
Ice::InputStream::EncapsDecoder11::readSlicedData()
{
    if (_current->slices.empty()) // No preserved slices.
    {
        return nullptr;
    }

    //
    // The indirectionTables member holds the indirection table for
    // each slice in slices.
    //
    assert(_current->slices.size() == _current->indirectionTables.size());
    for (SliceInfoSeq::size_type n = 0; n < _current->slices.size(); ++n)
    {
        //
        // We use the "instances" list in SliceInfo to hold references
        // to the target instances. Note that the instances might not have
        // been read yet in the case of a circular reference to an
        // enclosing instance.
        //
        const IndexList& table = _current->indirectionTables[n];
        vector<shared_ptr<Value>>& instances = _current->slices[n]->instances;
        instances.resize(table.size());
        IndexList::size_type j = 0;
        for (const auto& p : table)
        {
            addPatchEntry(p, patchValue<Value>, &instances[j++]);
        }
    }
    return make_shared<SlicedData>(_current->slices);
}

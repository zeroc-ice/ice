// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INPUT_STREAM_H
#define ICE_INPUT_STREAM_H

#include <Ice/CommunicatorF.h>
#include <Ice/InstanceF.h>
#include <Ice/Object.h>
#include <Ice/ValueF.h>
#include <Ice/ProxyF.h>
#include <Ice/LoggerF.h>
#include <Ice/ValueFactory.h>
#include <Ice/Buffer.h>
#include <Ice/Protocol.h>
#include <Ice/SlicedDataF.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/StreamHelpers.h>
#include <Ice/FactoryTable.h>

namespace Ice
{

class UserException;

/// \cond INTERNAL
template<typename T> inline void
patchHandle(void* addr, const ValuePtr& v)
{
#ifdef ICE_CPP11_MAPPING
    ::std::shared_ptr<T>* handle = static_cast<::std::shared_ptr<T>*>(addr);
    *handle = ::std::dynamic_pointer_cast<T>(v);
    if(v && !(*handle))
    {
        IceInternal::Ex::throwUOE(T::ice_staticId(), v);
    }
#else
    IceInternal::Handle<T>* p = static_cast<IceInternal::Handle<T>*>(addr);
    _icePatchObjectPtr(*p, v); // Generated _icePatchObjectPtr function, necessary for forward declarations.
#endif
}
/// \endcond

/**
 * Interface for input streams used to extract Slice types from a sequence of bytes.
 * \headerfile Ice/Ice.h
 */
class ICE_API InputStream : public IceInternal::Buffer
{
public:

    typedef size_t size_type;

    /**
     * Signature for a patch function, used to receive an unmarshaled value.
     * @param addr The target address.
     * @param v The unmarshaled value.
     */
    typedef void (*PatchFunc)(void* addr, const ValuePtr& v);

    /**
     * Constructs a stream using the latest encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     */
    InputStream();

    /**
     * Constructs a stream using the latest encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     * @param bytes The encoded data.
     */
    InputStream(const std::vector<Byte>& bytes);

    /**
     * Constructs a stream using the latest encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     * @param bytes The encoded data.
     */
    InputStream(const std::pair<const Byte*, const Byte*>& bytes);

    /// \cond INTERNAL
    InputStream(IceInternal::Buffer&, bool = false);
    /// \endcond

    /**
     * Constructs a stream using the communicator's default encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     */
    InputStream(const CommunicatorPtr& communicator);

    /**
     * Constructs a stream using the communicator's default encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param bytes The encoded data.
     */
    InputStream(const CommunicatorPtr& communicator, const std::vector<Byte>& bytes);

    /**
     * Constructs a stream using the communicator's default encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param bytes The encoded data.
     */
    InputStream(const CommunicatorPtr& communicator, const std::pair<const Byte*, const Byte*>& bytes);

    /// \cond INTERNAL
    InputStream(const CommunicatorPtr& communicator, IceInternal::Buffer&, bool = false);
    /// \endcond

    /**
     * Constructs a stream using the given encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     * @param version The encoding version used to encode the data to be unmarshaled.
     */
    InputStream(const EncodingVersion& version);

    /**
     * Constructs a stream using the given encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     * @param version The encoding version used to encode the data to be unmarshaled.
     * @param bytes The encoded data.
     */
    InputStream(const EncodingVersion& version, const std::vector<Byte>& bytes);

    /**
     * Constructs a stream using the given encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     * @param version The encoding version used to encode the data to be unmarshaled.
     * @param bytes The encoded data.
     */
    InputStream(const EncodingVersion& version, const std::pair<const Byte*, const Byte*>& bytes);

    /// \cond INTERNAL
    InputStream(const EncodingVersion&, IceInternal::Buffer&, bool = false);
    /// \endcond

    /**
     * Constructs a stream using the given communicator and encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param version The encoding version used to encode the data to be unmarshaled.
     */
    InputStream(const CommunicatorPtr& communicator, const EncodingVersion& version);

    /**
     * Constructs a stream using the given communicator and encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param version The encoding version used to encode the data to be unmarshaled.
     * @param bytes The encoded data.
     */
    InputStream(const CommunicatorPtr& communicator, const EncodingVersion& version, const std::vector<Byte>& bytes);

    /**
     * Constructs a stream using the given communicator and encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param version The encoding version used to encode the data to be unmarshaled.
     * @param bytes The encoded data.
     */
    InputStream(const CommunicatorPtr& communicator, const EncodingVersion& version,
                const std::pair<const Byte*, const Byte*>& bytes);

    /// \cond INTERNAL
    InputStream(const CommunicatorPtr&, const EncodingVersion&, IceInternal::Buffer&, bool = false);
    /// \endcond

    ~InputStream()
    {
        // Inlined for performance reasons.

        if(_currentEncaps != &_preAllocatedEncaps)
        {
            clear(); // Not inlined.
        }

#ifdef ICE_CPP11_MAPPING

        for(auto d: _deleters)
        {
            d();
        }
#endif
    }

    /**
     * Initializes the stream to use the communicator's default encoding version.
     * Use initialize() if you originally constructed the stream without a communicator.
     * @param communicator The communicator to use for unmarshaling tasks.
     */
    void initialize(const CommunicatorPtr& communicator);

    /**
     * Initializes the stream to use the given communicator and encoding version.
     * Use initialize() if you originally constructed the stream without a communicator.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param version The encoding version used to encode the data to be unmarshaled.
     */
    void initialize(const CommunicatorPtr& communicator, const EncodingVersion& version);

    /**
     * Releases any data retained by encapsulations.
     */
    void clear();

    /// \cond INTERNAL
    //
    // Must return Instance*, because we don't hold an InstancePtr for
    // optimization reasons (see comments below).
    //
    IceInternal::Instance* instance() const { return _instance; } // Inlined for performance reasons.
    /// \endcond

    /**
     * Sets the value factory manager to use when unmarshaling value instances. If the stream
     * was initialized with a communicator, the communicator's value factory manager will
     * be used by default.
     *
     * @param vfm The value factory manager.
     */
    void setValueFactoryManager(const ValueFactoryManagerPtr& vfm);

    /**
     * Sets the logger to use when logging trace messages. If the stream
     * was initialized with a communicator, the communicator's logger will
     * be used by default.
     *
     * @param logger The logger to use for logging trace messages.
     */
    void setLogger(const LoggerPtr& logger);

    /**
     * Sets the compact ID resolver to use when unmarshaling value and exception
     * instances. If the stream was initialized with a communicator, the communicator's
     * resolver will be used by default.
     *
     * @param r The compact ID resolver.
     */
#ifdef ICE_CPP11_MAPPING
    void setCompactIdResolver(std::function<std::string(int)> r);
#else
    void setCompactIdResolver(const CompactIdResolverPtr& r);
#endif

#ifndef ICE_CPP11_MAPPING
    /**
     * Indicates whether to mark instances of Slice classes as collectable. If the stream is
     * initialized with a communicator, this setting defaults to the value of the
     * Ice.CollectObjects property, otherwise the setting defaults to false.
     * @param b True to mark instances as collectable, false otherwise.
     */
    void setCollectObjects(bool b);
#endif

    /**
     * Indicates whether to slice instances of Slice classes to a known Slice type when a more
     * derived type is unknown. An instance is "sliced" when no static information is available
     * for a Slice type ID and no factory can be found for that type, resulting in the creation
     * of an instance of a less-derived type. If slicing is disabled in this situation, the
     * stream raises the exception NoValueFactoryException. The default behavior is to allow slicing.
     * @param b True to enable slicing, false otherwise.
     */
    void setSliceValues(bool b);

    /**
     * Indicates whether to log messages when instances of Slice classes are sliced. If the stream
     * is initialized with a communicator, this setting defaults to the value of the Ice.Trace.Slicing
     * property, otherwise the setting defaults to false.
     * @param b True to enable logging, false otherwise.
     */
    void setTraceSlicing(bool b);

    /**
     * Sets an upper limit on the depth of a class graph. If this limit is exceeded during
     * unmarshaling, the stream raises MarshalException.
     * @param n The maximum depth.
     */
    void setClassGraphDepthMax(size_t n);

    /**
     * Obtains the closure data associated with this stream.
     * @return The data as a void pointer.
     */
    void* getClosure() const;

    /**
     * Associates closure data with this stream.
     * @param p The data as a void pointer.
     * @return The previous closure data, or nil.
     */
    void* setClosure(void* p);

    /**
     * Swaps the contents of one stream with another.
     *
     * @param other The other stream.
     */
    void swap(InputStream& other);

    /// \cond INTERNAL
    void resetEncapsulation();
    /// \endcond

    /**
     * Resizes the stream to a new size.
     *
     * @param sz The new size.
     */
    void resize(Container::size_type sz)
    {
        b.resize(sz);
        i = b.end();
    }

    /**
     * Marks the start of a class instance.
     */
    void startValue()
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        _currentEncaps->decoder->startInstance(ValueSlice);
    }

    /**
     * Marks the end of a class instance.
     *
     * @param preserve Pass true and the stream will preserve the unknown slices of the instance, or false
     * to discard the unknown slices.
     * @return An object that encapsulates the unknown slice data.
     */
    SlicedDataPtr endValue(bool preserve)
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        return _currentEncaps->decoder->endInstance(preserve);
    }

    /**
     * Marks the start of a user exception.
     */
    void startException()
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        _currentEncaps->decoder->startInstance(ExceptionSlice);
    }

    /**
     * Marks the end of a user exception.
     *
     * @param preserve Pass true and the stream will preserve the unknown slices of the exception, or false
     * to discard the unknown slices.
     * @return An object that encapsulates the unknown slice data.
     */
    SlicedDataPtr endException(bool preserve)
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        return _currentEncaps->decoder->endInstance(preserve);
    }

    /**
     * Reads the start of an encapsulation.
     *
     * @return The encoding version used by the encapsulation.
     */
    const EncodingVersion& startEncapsulation()
    {
        Encaps* oldEncaps = _currentEncaps;
        if(!oldEncaps) // First allocated encaps?
        {
            _currentEncaps = &_preAllocatedEncaps;
        }
        else
        {
            _currentEncaps = new Encaps();
            _currentEncaps->previous = oldEncaps;
        }
        _currentEncaps->start = i - b.begin();

        //
        // I don't use readSize() and writeSize() for encapsulations,
        // because when creating an encapsulation, I must know in advance
        // how many bytes the size information will require in the data
        // stream. If I use an Int, it is always 4 bytes. For
        // readSize()/writeSize(), it could be 1 or 5 bytes.
        //
        Int sz;
        read(sz);
        if(sz < 6)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        if(i - sizeof(Int) + sz > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        _currentEncaps->sz = sz;

        read(_currentEncaps->encoding);
        IceInternal::checkSupportedEncoding(_currentEncaps->encoding); // Make sure the encoding is supported

        return _currentEncaps->encoding;
    }

    /**
     * Ends the current encapsulation.
     */
    void endEncapsulation()
    {
        assert(_currentEncaps);

        if(_currentEncaps->encoding != Encoding_1_0)
        {
            skipOptionals();
            if(i != b.begin() + _currentEncaps->start + _currentEncaps->sz)
            {
                throwEncapsulationException(__FILE__, __LINE__);
            }
        }
        else if(i != b.begin() + _currentEncaps->start + _currentEncaps->sz)
        {
            if(i + 1 != b.begin() + _currentEncaps->start + _currentEncaps->sz)
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

        Encaps* oldEncaps = _currentEncaps;
        _currentEncaps = _currentEncaps->previous;
        if(oldEncaps == &_preAllocatedEncaps)
        {
            oldEncaps->reset();
        }
        else
        {
            delete oldEncaps;
        }
    }

    /**
     * Skips an empty encapsulation.
     *
     * @return The encapsulation's encoding version.
     */
    EncodingVersion skipEmptyEncapsulation()
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
        IceInternal::checkSupportedEncoding(encoding); // Make sure the encoding is supported

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

    /**
     * Returns a blob of bytes representing an encapsulation.
     *
     * @param v A pointer into the internal marshaling buffer representing the start of the encoded encapsulation.
     * @param sz The number of bytes in the encapsulation.
     * @return encoding The encapsulation's encoding version.
     */
    EncodingVersion readEncapsulation(const Byte*& v, Int& sz)
    {
        EncodingVersion encoding;
        v = i;
        read(sz);
        if(sz < 6)
        {
            throwEncapsulationException(__FILE__, __LINE__);
        }
        if(i - sizeof(Int) + sz > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        read(encoding);
        i += sz - sizeof(Int) - 2;
        return encoding;
    }

    /**
     * Determines the current encoding version.
     *
     * @return The encoding version.
     */
    const EncodingVersion& getEncoding() const
    {
        return _currentEncaps ? _currentEncaps->encoding : _encoding;
    }

    /**
     * Determines the size of the current encapsulation, excluding the encapsulation header.
     *
     * @return The size of the encapsulated data.
     */
    Int getEncapsulationSize();

    /**
     * Skips over an encapsulation.
     *
     * @return The encoding version of the skipped encapsulation.
     */
    EncodingVersion skipEncapsulation();

    /**
     * Reads the start of a value or exception slice.
     *
     * @return The Slice type ID for this slice.
     */
    std::string startSlice()
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        return _currentEncaps->decoder->startSlice();
    }

    /**
     * Indicates that the end of a value or exception slice has been reached.
     */
    void endSlice()
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        _currentEncaps->decoder->endSlice();
    }

    /**
     * Skips over a value or exception slice.
     */
    void skipSlice()
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        _currentEncaps->decoder->skipSlice();
    }

    /**
     * Indicates that unmarshaling is complete, except for any class instances. The application must call this method
     * only if the stream actually contains class instances. Calling readPendingValues triggers the
     * patch callbacks to inform the application that unmarshaling of an instance is complete.
     */
    void readPendingValues();

    /**
     * Extracts a size from the stream.
     *
     * @return The extracted size.
     */
    Int readSize() // Inlined for performance reasons.
    {
        Byte byte;
        read(byte);
        unsigned char val = static_cast<unsigned char>(byte);
        if(val == 255)
        {
            Int v;
            read(v);
            if(v < 0)
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            return v;
        }
        else
        {
            return static_cast<Int>(static_cast<unsigned char>(byte));
        }
    }

    /**
     * Reads and validates a sequence size.
     *
     * @param minSize The minimum size required by the sequence type.
     * @return The extracted size.
     */
    Int readAndCheckSeqSize(int minSize);

    /**
     * Reads a blob of bytes from the stream.
     *
     * @param bytes The vector to hold a copy of the bytes from the marshaling buffer.
     * @param sz The number of bytes to read.
     */
    void readBlob(std::vector<Byte>& bytes, Int sz);

    /**
     * Reads a blob of bytes from the stream.
     *
     * @param v A pointer into the internal marshaling buffer representing the start of the blob.
     * @param sz The number of bytes to read.
     */
    void readBlob(const Byte*& v, Container::size_type sz)
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

    /**
     * Reads a data value from the stream.
     * @param v Holds the extracted data.
     */
    template<typename T> void read(T& v)
    {
        StreamHelper<T, StreamableTraits<T>::helper>::read(this, v);
    }

    /**
     * Reads an optional data value from the stream.
     * @param tag The tag ID.
     * @param v Holds the extracted data (if any).
     */
    template<typename T> void read(Int tag, IceUtil::Optional<T>& v)
    {
        if(readOptional(tag, StreamOptionalHelper<T,
                                             StreamableTraits<T>::helper,
                                             StreamableTraits<T>::fixedLength>::optionalFormat))
        {
#ifdef ICE_CPP11_MAPPING
            v.emplace();
#else
            v.__setIsSet();
#endif
            StreamOptionalHelper<T,
                                 StreamableTraits<T>::helper,
                                 StreamableTraits<T>::fixedLength>::read(this, *v);
        }
        else
        {
            v = IceUtil::None;
        }
    }

#ifdef ICE_CPP11_MAPPING

    /**
     * Extracts a sequence of data values from the stream.
     * @param v A pair of pointers representing the beginning and end of the sequence elements.
     */
    template<typename T> void read(std::pair<const T*, const T*>& v)
    {
        auto holder = new std::vector<T>;
        _deleters.push_back([holder] { delete holder; });
        read(*holder);
        if(holder->size() > 0)
        {
            v.first = holder->data();
            v.second = holder->data() + holder->size();
        }
        else
        {
            v.first = 0;
            v.second = 0;
        }
    }

    /**
     * Reads a list of mandatory data values.
     */
    template<typename T> void readAll(T& v)
    {
        read(v);
    }

    /**
     * Reads a list of mandatory data values.
     */
    template<typename T, typename... Te> void readAll(T& v, Te&... ve)
    {
        read(v);
        readAll(ve...);
    }

    /**
     * Reads a list of optional data values.
     */
    template<typename T>
    void readAll(std::initializer_list<int> tags, IceUtil::Optional<T>& v)
    {
        read(*(tags.begin() + tags.size() - 1), v);
    }

    /**
     * Reads a list of optional data values.
     */
    template<typename T, typename... Te>
    void readAll(std::initializer_list<int> tags, IceUtil::Optional<T>& v, IceUtil::Optional<Te>&... ve)
    {
        size_t index = tags.size() - sizeof...(ve) - 1;
        read(*(tags.begin() + index), v);
        readAll(tags, ve...);
    }

#endif

    /**
     * Determine if an optional value is available for reading.
     *
     * @param tag The tag associated with the value.
     * @param expectedFormat The optional format for the value.
     * @return True if the value is present, false otherwise.
     */
    bool readOptional(Int tag, OptionalFormat expectedFormat)
    {
        assert(_currentEncaps);
        if(_currentEncaps->decoder)
        {
            return _currentEncaps->decoder->readOptional(tag, expectedFormat);
        }
        else
        {
            return readOptImpl(tag, expectedFormat);
        }
    }

    /**
     * Reads a byte from the stream.
     * @param v The extracted byte.
     */
    void read(Byte& v)
    {
        if(i >= b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        v = *i++;
    }

    /**
     * Reads a sequence of bytes from the stream.
     * @param v A vector to hold a copy of the bytes.
     */
    void read(std::vector<Byte>& v);

    /**
     * Reads a sequence of bytes from the stream.
     * @param v A pair of pointers into the internal marshaling buffer representing the start and end of the
     * sequence elements.
     */
    void read(std::pair<const Byte*, const Byte*>& v);

#ifndef ICE_CPP11_MAPPING
    /**
     * Reads a sequence of bytes from the stream.
     * @param v A pair of pointers into the internal marshaling buffer representing the start and end of the
     * sequence elements.
     * @param arr A scoped array.
     */
    void read(std::pair<const Byte*, const Byte*>& v, ::IceUtil::ScopedArray<Byte>& arr)
    {
        arr.reset();
        read(v);
    }
#endif

    /**
     * Reads a bool from the stream.
     * @param v The extracted bool.
     */
    void read(bool& v)
    {
        if(i >= b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        v = (0 != *i++);
    }

    /**
     * Reads a sequence of boolean values from the stream.
     * @param v A vector to hold a copy of the boolean values.
     */
    void read(std::vector<bool>& v);

#ifdef ICE_CPP11_MAPPING
    /**
     * Reads a sequence of boolean values from the stream.
     * @param v A pair of pointers into the internal marshaling buffer representing the start and end of the
     * sequence elements.
     */
    void read(std::pair<const bool*, const bool*>& v);
#else
    /**
     * Reads a sequence of boolean values from the stream.
     * @param v A pair of pointers into the internal marshaling buffer representing the start and end of the
     * sequence elements.
     * @param arr A scoped array.
     */
    void read(std::pair<const bool*, const bool*>& v, ::IceUtil::ScopedArray<bool>& arr);
#endif

    /**
     * Reads a short from the stream.
     * @param v The extracted short.
     */
    void read(Short& v);

    /**
     * Reads a sequence of shorts from the stream.
     * @param v A vector to hold a copy of the short values.
     */
    void read(std::vector<Short>& v);

#ifdef ICE_CPP11_MAPPING
    /**
     * Reads a sequence of boolean values from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     */
    void read(std::pair<const short*, const short*>& v);
#else
    /**
     * Reads a sequence of shorts from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     * @param arr A scoped array.
     */
    void read(std::pair<const Short*, const Short*>& v, ::IceUtil::ScopedArray<Short>& arr);
#endif

    /**
     * Reads an int from the stream.
     * @param v The extracted int.
     */
    void read(Int& v) // Inlined for performance reasons.
    {
        if(b.end() - i < static_cast<int>(sizeof(Int)))
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        const Byte* src = &(*i);
        i += sizeof(Int);
#ifdef ICE_BIG_ENDIAN
        Byte* dest = reinterpret_cast<Byte*>(&v) + sizeof(Int) - 1;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest = *src;
#else
        Byte* dest = reinterpret_cast<Byte*>(&v);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;
#endif
    }

    /**
     * Reads a sequence of ints from the stream.
     * @param v A vector to hold a copy of the int values.
     */
    void read(std::vector<Int>& v);

#ifdef ICE_CPP11_MAPPING
    /**
     * Reads a sequence of ints from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     */
    void read(std::pair<const int*, const int*>& v);
#else
    /**
     * Reads a sequence of ints from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     * @param arr A scoped array.
     */
    void read(std::pair<const Int*, const Int*>& v, ::IceUtil::ScopedArray<Int>& arr);
#endif

    /**
     * Reads a long from the stream.
     * @param v The extracted long.
     */
    void read(Long& v);

    /**
     * Reads a sequence of longs from the stream.
     * @param v A vector to hold a copy of the long values.
     */
    void read(std::vector<Long>& v);

#ifdef ICE_CPP11_MAPPING
    /**
     * Reads a sequence of longs from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     */
    void read(std::pair<const long long*, const long long*>& v);
#else
    /**
     * Reads a sequence of longs from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     * @param arr A scoped array.
     */
    void read(std::pair<const Long*, const Long*>& v, ::IceUtil::ScopedArray<Long>& arr);
#endif

    /**
     * Reads a float from the stream.
     * @param v The extracted float.
     */
    void read(Float& v);

    /**
     * Reads a sequence of floats from the stream.
     * @param v A vector to hold a copy of the float values.
     */
    void read(std::vector<Float>& v);

#ifdef ICE_CPP11_MAPPING
    /**
     * Reads a sequence of floats from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     */
    void read(std::pair<const float*, const float*>& v);
#else
    /**
     * Reads a sequence of floats from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     * @param arr A scoped array.
     */
    void read(std::pair<const Float*, const Float*>& v, ::IceUtil::ScopedArray<Float>& arr);
#endif

    /**
     * Reads a double from the stream.
     * @param v The extracted double.
     */
    void read(Double& v);

    /**
     * Reads a sequence of doubles from the stream.
     * @param v A vector to hold a copy of the double values.
     */
    void read(std::vector<Double>& v);

#ifdef ICE_CPP11_MAPPING
    /**
     * Reads a sequence of doubles from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     */
    void read(std::pair<const double*, const double*>& v);
#else
    /**
     * Reads a sequence of doubles from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     * @param arr A scoped array.
     */
    void read(std::pair<const Double*, const Double*>& v, ::IceUtil::ScopedArray<Double>& arr);
#endif

    /**
     * Reads a string from the stream.
     * @param v The extracted string.
     * @param convert Determines whether the string is processed by the string converter, if one
     * is installed. The default behavior is to convert strings.
     */
    void read(std::string& v, bool convert = true);

#ifdef ICE_CPP11_MAPPING
    /**
     * Reads a string from the stream.
     * @param vdata A pointer to the beginning of the string.
     * @param vsize The number of bytes in the string.
     * @param convert Determines whether the string is processed by the string converter, if one
     * is installed. The default behavior is to convert strings.
     */
    void read(const char*& vdata, size_t& vsize, bool convert = true);
#else
    // For custom strings, convert = false
    /**
     * Reads a string from the stream. String conversion is disabled.
     * @param vdata A pointer to the beginning of the string.
     * @param vsize The number of bytes in the string.
     */
    void read(const char*& vdata, size_t& vsize);

    // For custom strings, convert = true
    /**
     * Reads a string from the stream. String conversion is enabled.
     * @param vdata A pointer to the beginning of the string.
     * @param vsize The number of bytes in the string.
     * @param holder Holds the string contents.
     */
    void read(const char*& vdata, size_t& vsize, std::string& holder);
#endif

    /**
     * Reads a sequence of strings from the stream.
     * @param v The extracted string sequence.
     * @param convert Determines whether strings are processed by the string converter, if one
     * is installed. The default behavior is to convert the strings.
     */
    void read(std::vector<std::string>& v, bool convert = true);

    /**
     * Reads a wide string from the stream.
     * @param v The extracted string.
     */
    void read(std::wstring& v);

    /**
     * Reads a sequence of wide strings from the stream.
     * @param v The extracted sequence.
     */
    void read(std::vector<std::wstring>& v);

#ifdef ICE_CPP11_MAPPING
    /**
     * Reads a proxy from the stream.
     * @return The proxy as the base ObjectPrx type.
     */
    std::shared_ptr<ObjectPrx> readProxy();

    /**
     * Reads a typed proxy from the stream.
     * @param v The proxy as a user-defined type.
     */
    template<typename T, typename ::std::enable_if<::std::is_base_of<ObjectPrx, T>::value>::type* = nullptr>
    void read(::std::shared_ptr<T>& v)
    {
        ::std::shared_ptr<ObjectPrx> proxy(readProxy());
        if(!proxy)
        {
            v = 0;
        }
        else
        {
            v = ::IceInternal::createProxy<T>();
            v->_copyFrom(proxy);
        }
    }
#else
    /**
     * Reads a proxy from the stream.
     * @param v The proxy as the base ObjectPrx type.
     */
    void read(ObjectPrx& v);

    /**
     * Reads a typed proxy from the stream.
     * @param v The proxy as a user-defined type.
     */
    template<typename T> void read(IceInternal::ProxyHandle<T>& v)
    {
        _readProxy(this, v); // Generated _readProxy method, necessary for forward declarations.
    }
#endif

    /**
     * Reads a value (instance of a Slice class) from the stream.
     * @param v The instance.
     */
#ifdef ICE_CPP11_MAPPING // C++11 mapping
    template<typename T, typename ::std::enable_if<::std::is_base_of<Value, T>::value>::type* = nullptr>
    void read(::std::shared_ptr<T>& v)
    {
        read(&patchHandle<T>, &v);
    }
#else // C++98 mapping
    template<typename T> void read(IceInternal::Handle<T>& v)
    {
        read(&patchHandle<T>, &v);
    }
#endif

    /**
     * Reads a value (instance of a Slice class) from the stream.
     * @param patchFunc The patch callback function.
     * @param patchAddr Closure data passed to the callback.
     */
    void read(PatchFunc patchFunc, void* patchAddr)
    {
        initEncaps();
        _currentEncaps->decoder->read(patchFunc, patchAddr);
    }

    /**
     * Reads an enumerator from the stream.
     * @param maxValue The maximum enumerator value in the definition.
     * @return The enumerator value.
     */
    Int readEnum(Int maxValue);

    /**
     * Extracts a user exception from the stream and throws it.
     * @param factory If provided, this factory is given the first opportunity to instantiate
     * the exception. If not provided, or if the factory does not throw an exception when invoked,
     * the stream will attempt to instantiate the exception using static type information.
     * @throws UserException The user exception that was unmarshaled.
     */
    void throwException(ICE_IN(ICE_DELEGATE(UserExceptionFactory)) factory = ICE_NULLPTR);

    /**
     * Skips one optional value with the given format.
     * @param format The expected format of the optional, if present.
     */
    void skipOptional(OptionalFormat format);

    /**
     * Skips all remaining optional values.
     */
    void skipOptionals();

    /**
     * Advances the current stream position by the given number of bytes.
     * @param size The number of bytes to skip.
     */
    void skip(size_type size)
    {
        if(i + size > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        i += size;
    }

    /**
     * Reads a size at the current position and skips that number of bytes.
     */
    void skipSize()
    {
        Byte bt;
        read(bt);
        if(static_cast<unsigned char>(bt) == 255)
        {
            skip(4);
        }
    }

    /**
     * Obtains the current position of the stream.
     * @return The current position.
     */
    size_type pos()
    {
        return i - b.begin();
    }

    /**
     * Sets a new position for the stream.
     * @param p The new position.
     */
    void pos(size_type p)
    {
        i = b.begin() + p;
    }

    /// \cond INTERNAL
    InputStream(IceInternal::Instance*, const EncodingVersion&);
    InputStream(IceInternal::Instance*, const EncodingVersion&, IceInternal::Buffer&, bool = false);

    void initialize(IceInternal::Instance*, const EncodingVersion&);

    bool readOptImpl(Int, OptionalFormat);
    /// \endcond

private:

    void initialize(const EncodingVersion&);

    //
    // String
    //
    bool readConverted(std::string&, Int);

    //
    // We can't throw these exception from inline functions from within
    // this file, because we cannot include the header with the
    // exceptions. Doing so would screw up the whole include file
    // ordering.
    //
    void throwUnmarshalOutOfBoundsException(const char*, int);
    void throwEncapsulationException(const char*, int);

    std::string resolveCompactId(int) const;

    void postUnmarshal(const ValuePtr&) const;

    class Encaps;
    enum SliceType { NoSlice, ValueSlice, ExceptionSlice };

    void traceSkipSlice(const std::string&, SliceType) const;

    ValueFactoryManagerPtr valueFactoryManager() const;

    LoggerPtr logger() const;

#ifdef ICE_CPP11_MAPPING
    std::function<std::string(int)> compactIdResolver() const;
#else
    CompactIdResolverPtr compactIdResolver() const;
#endif

    typedef std::vector<ValuePtr> ValueList;

    class ICE_API EncapsDecoder : private ::IceUtil::noncopyable
    {
    public:

        virtual ~EncapsDecoder();

        virtual void read(PatchFunc, void*) = 0;
        virtual void throwException(ICE_IN(ICE_DELEGATE(UserExceptionFactory))) = 0;

        virtual void startInstance(SliceType) = 0;
        virtual SlicedDataPtr endInstance(bool) = 0;
        virtual const std::string& startSlice() = 0;
        virtual void endSlice() = 0;
        virtual void skipSlice() = 0;

        virtual bool readOptional(Int, OptionalFormat)
        {
            return false;
        }

        virtual void readPendingValues()
        {
        }

    protected:

        EncapsDecoder(InputStream* stream, Encaps* encaps, bool sliceValues, size_t classGraphDepthMax,
                      const Ice::ValueFactoryManagerPtr& f) :
            _stream(stream), _encaps(encaps), _sliceValues(sliceValues), _classGraphDepthMax(classGraphDepthMax),
            _classGraphDepth(0), _valueFactoryManager(f), _typeIdIndex(0)
        {
        }

        std::string readTypeId(bool);
        ValuePtr newInstance(const std::string&);

        void addPatchEntry(Int, PatchFunc, void*);
        void unmarshal(Int, const ValuePtr&);

        typedef std::map<Int, ValuePtr> IndexToPtrMap;
        typedef std::map<Int, std::string> TypeIdMap;

        struct PatchEntry
        {
            PatchFunc patchFunc;
            void* patchAddr;
            size_t classGraphDepth;
        };
        typedef std::vector<PatchEntry> PatchList;
        typedef std::map<Int, PatchList> PatchMap;

        InputStream* _stream;
        Encaps* _encaps;
        const bool _sliceValues;
        const size_t _classGraphDepthMax;
        size_t _classGraphDepth;
        Ice::ValueFactoryManagerPtr _valueFactoryManager;

        // Encapsulation attributes for object un-marshalling
        PatchMap _patchMap;

    private:

        // Encapsulation attributes for object un-marshalling
        IndexToPtrMap _unmarshaledMap;
        TypeIdMap _typeIdMap;
        Int _typeIdIndex;
        ValueList _valueList;
    };

    class ICE_API EncapsDecoder10 : public EncapsDecoder
    {
    public:

        EncapsDecoder10(InputStream* stream, Encaps* encaps, bool sliceValues, size_t classGraphDepthMax,
                        const Ice::ValueFactoryManagerPtr& f) :
            EncapsDecoder(stream, encaps, sliceValues, classGraphDepthMax, f),
            _sliceType(NoSlice)
        {
        }

        virtual void read(PatchFunc, void*);
        virtual void throwException(ICE_IN(ICE_DELEGATE(UserExceptionFactory)));

        virtual void startInstance(SliceType);
        virtual SlicedDataPtr endInstance(bool);
        virtual const std::string& startSlice();
        virtual void endSlice();
        virtual void skipSlice();

        virtual void readPendingValues();

    private:

        void readInstance();

        // Instance attributes
        SliceType _sliceType;
        bool _skipFirstSlice;

        // Slice attributes
        Int _sliceSize;
        std::string _typeId;
    };

    class ICE_API EncapsDecoder11 : public EncapsDecoder
    {
    public:

        EncapsDecoder11(InputStream* stream, Encaps* encaps, bool sliceValues, size_t classGraphDepthMax,
                        const Ice::ValueFactoryManagerPtr& f) :
            EncapsDecoder(stream, encaps, sliceValues, classGraphDepthMax, f),
            _preAllocatedInstanceData(0), _current(0), _valueIdIndex(1)
        {
        }

        virtual void read(PatchFunc, void*);
        virtual void throwException(ICE_IN(ICE_DELEGATE(UserExceptionFactory)));

        virtual void startInstance(SliceType);
        virtual SlicedDataPtr endInstance(bool);
        virtual const std::string& startSlice();
        virtual void endSlice();
        virtual void skipSlice();

        virtual bool readOptional(Int, OptionalFormat);

    private:

        Int readInstance(Int, PatchFunc, void*);
        SlicedDataPtr readSlicedData();

        struct IndirectPatchEntry
        {
            Int index;
            PatchFunc patchFunc;
            void* patchAddr;
        };
        typedef std::vector<IndirectPatchEntry> IndirectPatchList;

        typedef std::vector<Int> IndexList;
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
            SliceInfoSeq slices; // Preserved slices.
            IndexListList indirectionTables;

            // Slice attributes
            Byte sliceFlags;
            Int sliceSize;
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

        Int _valueIdIndex; // The ID of the next value to unmarshal.
    };

    class Encaps : private ::IceUtil::noncopyable
    {
    public:

        Encaps() : start(0), decoder(0), previous(0)
        {
            // Inlined for performance reasons.
        }
        ~Encaps()
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
        Int sz;
        EncodingVersion encoding;

        EncapsDecoder* decoder;

        Encaps* previous;
    };

    //
    // Optimization. The instance may not be deleted while a
    // stack-allocated stream still holds it.
    //
    IceInternal::Instance* _instance;

    //
    // The encoding version to use when there's no encapsulation to
    // read from. This is for example used to read message headers.
    //
    EncodingVersion _encoding;

    Encaps* _currentEncaps;

    void initEncaps();

    Encaps _preAllocatedEncaps;

#ifndef ICE_CPP11_MAPPING
    bool _collectObjects;
#endif

    bool _traceSlicing;

    size_t _classGraphDepthMax;

    void* _closure;

    bool _sliceValues;

    int _startSeq;
    int _minSeqSize;

    ValueFactoryManagerPtr _valueFactoryManager;
    LoggerPtr _logger;
#ifdef ICE_CPP11_MAPPING
    std::function<std::string(int)> _compactIdResolver;
#else
    CompactIdResolverPtr _compactIdResolver;
#endif

#ifdef ICE_CPP11_MAPPING
    std::vector<std::function<void()>> _deleters;
#endif

};

} // End namespace Ice

#endif

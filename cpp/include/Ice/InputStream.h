// Copyright (c) ZeroC, Inc.

#ifndef ICE_INPUT_STREAM_H
#define ICE_INPUT_STREAM_H

#include "Buffer.h"
#include "CommunicatorF.h"
#include "Ice/Version.h"
#include "InstanceF.h"
#include "LocalException.h"
#include "Logger.h"
#include "ReferenceF.h"
#include "SlicedDataF.h"
#include "StreamableTraits.h"
#include "UserExceptionFactory.h"
#include "ValueF.h"
#include "ValueFactory.h"

#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>

namespace IceInternal::Ex
{
    ICE_API void throwUOE(const char* file, int line, const std::string&, const Ice::ValuePtr&);
    ICE_API void throwMemoryLimitException(const char* file, int line, size_t, size_t);
    ICE_API void throwMarshalException(const char* file, int line, std::string);
}

namespace Ice
{
    class ObjectPrx;

    /// \cond INTERNAL
    template<typename T> inline void patchValue(void* addr, const ValuePtr& v)
    {
        std::shared_ptr<T>* ptr = static_cast<std::shared_ptr<T>*>(addr);
        *ptr = std::dynamic_pointer_cast<T>(v);
        if (v && !(*ptr))
        {
            IceInternal::Ex::throwUOE(__FILE__, __LINE__, std::string{T::ice_staticId()}, v);
        }
    }
    /// \endcond

    /**
     * Interface for input streams used to extract Slice types from a sequence of bytes.
     * \headerfile Ice/Ice.h
     */
    class ICE_API InputStream final : public IceInternal::Buffer
    {
    public:
        using size_type = size_t;

        /**
         * Signature for a patch function, used to receive an unmarshaled value.
         * @param addr The target address.
         * @param v The unmarshaled value.
         */
        using PatchFunc = std::function<void(void* addr, const ValuePtr& v)>;

        /**
         * Constructs a stream using the communicator's default encoding version.
         * @param communicator The communicator to use for unmarshaling tasks.
         * @param bytes The encoded data.
         */
        InputStream(const CommunicatorPtr& communicator, const std::vector<std::byte>& bytes);

        /**
         * Constructs a stream using the communicator's default encoding version.
         * @param communicator The communicator to use for unmarshaling tasks.
         * @param bytes The encoded data.
         */
        InputStream(const CommunicatorPtr& communicator, std::pair<const std::byte*, const std::byte*> bytes);

        /**
         * Constructs a stream using the given communicator and encoding version.
         * @param communicator The communicator to use for unmarshaling tasks.
         * @param encoding The encoding version used to encode the data to be unmarshaled.
         * @param bytes The encoded data.
         */
        InputStream(const CommunicatorPtr& communicator, EncodingVersion encoding, const std::vector<std::byte>& bytes);

        /**
         * Constructs a stream using the given communicator and encoding version.
         * @param communicator The communicator to use for unmarshaling tasks.
         * @param encoding The encoding version used to encode the data to be unmarshaled.
         * @param bytes The encoded data.
         */
        InputStream(
            const CommunicatorPtr& communicator,
            EncodingVersion encoding,
            std::pair<const std::byte*, const std::byte*> bytes);

        /// \cond INTERNAL

        // Constructs a stream with an empty buffer.
        explicit InputStream(IceInternal::Instance* instance, EncodingVersion encoding);

        // Constructs a stream with the specified encoding and buffer.
        InputStream(IceInternal::Instance* instance, EncodingVersion encoding, IceInternal::Buffer& buf, bool adopt);
        /// \endcond

        /**
         * Move constructor.
         * @param other The input stream to move into this input stream.
         */
        InputStream(InputStream&& other) noexcept;

        /**
         * Move assignment operator.
         * @param other The input stream to move into this input stream.
         */
        InputStream& operator=(InputStream&& other) noexcept;

        ~InputStream()
        {
            // Inlined for performance reasons.

            if (_currentEncaps != &_preAllocatedEncaps)
            {
                clear(); // Not inlined.
            }

            for (auto d : _deleters)
            {
                d();
            }
        }

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
        void swap(InputStream& other) noexcept;

        /// \cond INTERNAL
        void resetEncapsulation();

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
        /// \endcond

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
         * @return An object that encapsulates the unknown slice data.
         */
        SlicedDataPtr endValue()
        {
            assert(_currentEncaps && _currentEncaps->decoder);
            return _currentEncaps->decoder->endInstance();
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
         */
        void endException()
        {
            assert(_currentEncaps && _currentEncaps->decoder);
            _currentEncaps->decoder->endInstance();
        }

        /**
         * Reads the start of an encapsulation.
         *
         * @return The encoding version used by the encapsulation.
         */
        const EncodingVersion& startEncapsulation();

        /**
         * Ends the current encapsulation.
         */
        void endEncapsulation();

        /**
         * Skips an empty encapsulation.
         *
         * @return The encapsulation's encoding version.
         */
        EncodingVersion skipEmptyEncapsulation();

        /**
         * Returns a blob of bytes representing an encapsulation.
         *
         * @param v A pointer into the internal marshaling buffer representing the start of the encoded encapsulation.
         * @param sz The number of bytes in the encapsulation.
         * @return encoding The encapsulation's encoding version.
         */
        EncodingVersion readEncapsulation(const std::byte*& v, std::int32_t& sz);

        /**
         * Determines the current encoding version.
         *
         * @return The encoding version.
         */
        const EncodingVersion& getEncoding() const { return _currentEncaps ? _currentEncaps->encoding : _encoding; }

        /**
         * Determines the size of the current encapsulation, excluding the encapsulation header.
         *
         * @return The size of the encapsulated data.
         */
        std::int32_t getEncapsulationSize();

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
         * Indicates that unmarshaling is complete, except for any class instances. The application must call this
         * method only if the stream actually contains class instances. Calling readPendingValues triggers the patch
         * callbacks to inform the application that unmarshaling of an instance is complete.
         */
        void readPendingValues();

        /**
         * Extracts a size from the stream.
         *
         * @return The extracted size.
         */
        std::int32_t readSize() // Inlined for performance reasons.
        {
            std::uint8_t byte;
            read(byte);
            unsigned char val = static_cast<unsigned char>(byte);
            if (val == 255)
            {
                std::int32_t v;
                read(v);
                if (v < 0)
                {
                    throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
                }
                return v;
            }
            else
            {
                return static_cast<std::int32_t>(static_cast<unsigned char>(byte));
            }
        }

        /**
         * Reads and validates a sequence size.
         *
         * @param minSize The minimum size required by the sequence type.
         * @return The extracted size.
         */
        std::int32_t readAndCheckSeqSize(int minSize);

        /**
         * Reads a blob of bytes from the stream.
         *
         * @param bytes The vector to hold a copy of the bytes from the marshaling buffer.
         * @param sz The number of bytes to read.
         */
        void readBlob(std::vector<std::byte>& bytes, std::int32_t sz);

        /**
         * Reads a blob of bytes from the stream.
         *
         * @param v A pointer into the internal marshaling buffer representing the start of the blob.
         * @param sz The number of bytes to read.
         */
        void readBlob(const std::byte*& v, Container::size_type sz)
        {
            if (sz > 0)
            {
                v = i;
                if (static_cast<Container::size_type>(b.end() - i) < sz)
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
        template<typename T> void read(T& v) { StreamHelper<T, StreamableTraits<T>::helper>::read(this, v); }

        /**
         * Reads an optional data value from the stream. For all types except proxies.
         * @param tag The tag ID.
         * @param v Holds the extracted data (if any).
         */
        template<typename T, std::enable_if_t<!std::is_base_of<ObjectPrx, T>::value, bool> = true>
        void read(std::int32_t tag, std::optional<T>& v)
        {
            if (readOptional(
                    tag,
                    StreamOptionalHelper<T, StreamableTraits<T>::helper, StreamableTraits<T>::fixedLength>::
                        optionalFormat))
            {
                v.emplace();
                StreamOptionalHelper<T, StreamableTraits<T>::helper, StreamableTraits<T>::fixedLength>::read(this, *v);
            }
            else
            {
                v = std::nullopt;
            }
        }

        /**
         * Reads an optional proxy from the stream.
         * @param tag The tag ID.
         * @param v The proxy unmarshaled by this function. If nullopt, the proxy was not present in the stream or
         * was set to nullopt (set to nullopt is supported for backward compatibility with Ice 3.7 and earlier
         * releases).
         */
        template<typename T, std::enable_if_t<std::is_base_of<ObjectPrx, T>::value, bool> = true>
        void read(std::int32_t tag, std::optional<T>& v)
        {
            if (readOptional(tag, OptionalFormat::FSize))
            {
                skip(4); // the fixed-length size on 4 bytes
                read(v); // can be nullopt
            }
            else
            {
                v = std::nullopt;
            }
        }

        /**
         * Extracts a sequence of data values from the stream.
         * @param v A pair of pointers representing the beginning and end of the sequence elements.
         */
        template<typename T> void read(std::pair<const T*, const T*>& v)
        {
            auto holder = new std::vector<T>;
            _deleters.push_back([holder] { delete holder; });
            read(*holder);
            if (holder->size() > 0)
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

#if defined(ICE_UNALIGNED) || (defined(_WIN32) && defined(ICE_API_EXPORTS))
        // Optimization with unaligned reads
        void read(std::pair<const std::int16_t*, const std::int16_t*>& v) { unalignedRead(v); }
        void read(std::pair<const std::int32_t*, const std::int32_t*>& v) { unalignedRead(v); }
        void read(std::pair<const std::int64_t*, const std::int64_t*>& v) { unalignedRead(v); }
        void read(std::pair<const float*, const float*>& v) { unalignedRead(v); }
        void read(std::pair<const double*, const double*>& v) { unalignedRead(v); }
#endif

        /**
         * Reads a list of mandatory data values.
         */
        template<typename T> void readAll(T& v) { read(v); }

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
        template<typename T> void readAll(std::initializer_list<std::int32_t> tags, std::optional<T>& v)
        {
            read(*(tags.begin() + tags.size() - 1), v);
        }

        /**
         * Reads a list of optional data values.
         */
        template<typename T, typename... Te>
        void readAll(std::initializer_list<std::int32_t> tags, std::optional<T>& v, std::optional<Te>&... ve)
        {
            size_t index = tags.size() - sizeof...(ve) - 1;
            read(*(tags.begin() + index), v);
            readAll(tags, ve...);
        }

        /**
         * Determine if an optional value is available for reading.
         *
         * @param tag The tag associated with the value.
         * @param expectedFormat The optional format for the value.
         * @return True if the value is present, false otherwise.
         */
        bool readOptional(std::int32_t tag, OptionalFormat expectedFormat)
        {
            assert(_currentEncaps);
            if (_currentEncaps->decoder)
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
        void read(std::byte& v)
        {
            if (i >= b.end())
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            v = *i++;
        }

        /**
         * Reads a byte from the stream.
         * @param v The extracted byte.
         */
        void read(std::uint8_t& v)
        {
            if (i >= b.end())
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            v = static_cast<std::uint8_t>(*i++);
        }

        /**
         * Reads a sequence of bytes from the stream.
         * @param v A vector to hold a copy of the bytes.
         */
        void read(std::vector<std::byte>& v);

        /**
         * Reads a sequence of bytes from the stream.
         * @param v A pair of pointers into the internal marshaling buffer representing the start and end of the
         * sequence elements.
         */
        void read(std::pair<const std::byte*, const std::byte*>& v);

        /**
         * Reads a bool from the stream.
         * @param v The extracted bool.
         */
        void read(bool& v)
        {
            if (i >= b.end())
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            v = (std::byte{0} != *i++);
        }

        /**
         * Reads a sequence of boolean values from the stream.
         * @param v A vector to hold a copy of the boolean values.
         */
        void read(std::vector<bool>& v);

        /**
         * Reads a sequence of boolean values from the stream.
         * @param v A pair of pointers into the internal marshaling buffer representing the start and end of the
         * sequence elements.
         */
        void read(std::pair<const bool*, const bool*>& v);

        /**
         * Unmarshals a Slice short into an int16_t.
         * @param v The extracted int16_t.
         */
        void read(std::int16_t& v);

        /**
         * Unmarshals a sequence of Slice shorts into a vector of int16_t.
         * @param v A vector to hold a copy of the int16_t values.
         */
        void read(std::vector<std::int16_t>& v);

        /**
         * Reads an int from the stream.
         * @param v The extracted int.
         */
        void read(std::int32_t& v);

        /**
         * Reads a sequence of ints from the stream.
         * @param v A vector to hold a copy of the int values.
         */
        void read(std::vector<std::int32_t>& v);

        /**
         * Reads a long from the stream.
         * @param v The extracted long.
         */
        void read(std::int64_t& v);

        /**
         * Reads a sequence of longs from the stream.
         * @param v A vector to hold a copy of the long values.
         */
        void read(std::vector<std::int64_t>& v);

        /**
         * Unmarshals a Slice float into a float.
         * @param v The extracted float.
         */
        void read(float& v);

        /**
         * Unmarshals a sequence of Slice floats into a vector of float.
         * @param v An output vector filled by this function.
         */
        void read(std::vector<float>& v);

        /**
         * Unmarshals a Slice double into a double.
         * @param v The extracted double.
         */
        void read(double& v);

        /**
         * Unmarshals a sequence of Slice doubles into a vector of double.
         * @param v An output vector filled by this function.
         */
        void read(std::vector<double>& v);

        /**
         * Reads a string from the stream.
         * @param v The extracted string.
         * @param convert Determines whether the string is processed by the string converter, if one
         * is installed. The default behavior is to convert strings.
         */
        void read(std::string& v, bool convert = true);

        /**
         * Reads a string from the stream.
         * @param vdata A pointer to the beginning of the string.
         * @param vsize The number of bytes in the string.
         * @param convert Determines whether the string is processed by the string converter, if one
         * is installed. The default behavior is to convert strings.
         */
        void read(const char*& vdata, size_t& vsize, bool convert = true);

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
        /**
         * Reads a typed proxy from the stream.
         * @param v The proxy as a user-defined type.
         */
        template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
        void read(std::optional<Prx>& v)
        {
            IceInternal::ReferencePtr ref = readReference();
            if (ref)
            {
                v = Prx::_fromReference(ref);
            }
            else
            {
                v = std::nullopt;
            }
        }

        /**
         * Reads a value (instance of a Slice class) from the stream (New mapping).
         * @param v The instance.
         */
        template<typename T, typename std::enable_if<std::is_base_of<Value, T>::value>::type* = nullptr>
        void read(std::shared_ptr<T>& v)
        {
            read(patchValue<T>, &v);
        }

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
        std::int32_t readEnum(std::int32_t maxValue);

        /**
         * Extracts a user exception from the stream and throws it.
         * @param factory If provided, this factory is given the first opportunity to instantiate
         * the exception. If not provided, or if the factory does not throw an exception when invoked,
         * the stream will attempt to instantiate the exception using static type information.
         * @throws UserException The user exception that was unmarshaled.
         */
        void throwException(UserExceptionFactory factory = nullptr);

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
            if (i + size > b.end())
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
            std::uint8_t bt;
            read(bt);
            if (static_cast<unsigned char>(bt) == 255)
            {
                skip(4);
            }
        }

        /**
         * Obtains the current position of the stream.
         * @return The current position.
         */
        size_type pos() { return static_cast<size_t>(i - b.begin()); }

        /**
         * Sets a new position for the stream.
         * @param p The new position.
         */
        void pos(size_type p) { i = b.begin() + p; }

        /// \cond INTERNAL
        bool readOptImpl(std::int32_t, OptionalFormat);
        /// \endcond

    private:
#if defined(ICE_UNALIGNED) || (defined(_WIN32) && defined(ICE_API_EXPORTS))
        template<typename T> void unalignedRead(std::pair<const T*, const T*>& v)
        {
            int sz = readAndCheckSeqSize(static_cast<int>(sizeof(T)));

            if (sz > 0)
            {
                v.first = reinterpret_cast<T*>(i);
                i += sz * static_cast<int>(sizeof(T));
                v.second = reinterpret_cast<T*>(i);
            }
            else
            {
                v.first = v.second = nullptr;
            }
        }
#endif

        // The primary constructor, called by all other constructors. It requires a non-null instance.
        InputStream(IceInternal::Instance* instance, EncodingVersion encoding, IceInternal::Buffer&& buf);

        // Reads a reference from the stream; the return value can be null.
        IceInternal::ReferencePtr readReference();

        bool readConverted(std::string&, std::int32_t);

        void throwUnmarshalOutOfBoundsException(const char*, int);

        std::string resolveCompactId(int) const;

        class Encaps;
        enum SliceType
        {
            NoSlice,
            ValueSlice,
            ExceptionSlice
        };

        void traceSkipSlice(std::string_view, SliceType) const;

        using ValueList = std::vector<ValuePtr>;

        class ICE_API EncapsDecoder
        {
        public:
            EncapsDecoder(const EncapsDecoder&) = delete;
            virtual ~EncapsDecoder();

            EncapsDecoder& operator=(const EncapsDecoder&) = delete;

            virtual void read(PatchFunc, void*) = 0;
            virtual void throwException(UserExceptionFactory) = 0;

            virtual void startInstance(SliceType) = 0;
            virtual SlicedDataPtr endInstance() = 0;
            virtual const std::string& startSlice() = 0;
            virtual void endSlice() = 0;
            virtual void skipSlice() = 0;

            virtual bool readOptional(std::int32_t, OptionalFormat) { return false; }

            virtual void readPendingValues() {}

        protected:
            EncapsDecoder(
                InputStream* stream,
                Encaps* encaps,
                size_t classGraphDepthMax,
                const Ice::ValueFactoryManagerPtr& f)
                : _stream(stream),
                  _encaps(encaps),
                  _classGraphDepthMax(classGraphDepthMax),
                  _classGraphDepth(0),
                  _valueFactoryManager(f),
                  _typeIdIndex(0)
            {
            }

            std::string readTypeId(bool);
            ValuePtr newInstance(std::string_view);

            void addPatchEntry(std::int32_t, const PatchFunc&, void*);
            void unmarshal(std::int32_t, const ValuePtr&);

            using IndexToPtrMap = std::map<std::int32_t, ValuePtr>;
            using TypeIdMap = std::map<std::int32_t, std::string>;

            struct PatchEntry
            {
                PatchFunc patchFunc;
                void* patchAddr;
                size_t classGraphDepth;
            };
            using PatchList = std::vector<PatchEntry>;
            using PatchMap = std::map<std::int32_t, PatchList>;

            InputStream* _stream;
            Encaps* _encaps;
            const size_t _classGraphDepthMax;
            size_t _classGraphDepth;
            Ice::ValueFactoryManagerPtr _valueFactoryManager;

            // Encapsulation attributes for object un-marshaling
            PatchMap _patchMap;

        private:
            // Encapsulation attributes for object un-marshaling
            IndexToPtrMap _unmarshaledMap;
            TypeIdMap _typeIdMap;
            std::int32_t _typeIdIndex;
            ValueList _valueList;
        };

        class ICE_API EncapsDecoder10 : public EncapsDecoder
        {
        public:
            EncapsDecoder10(
                InputStream* stream,
                Encaps* encaps,
                size_t classGraphDepthMax,
                const Ice::ValueFactoryManagerPtr& f)
                : EncapsDecoder(stream, encaps, classGraphDepthMax, f),
                  _sliceType(NoSlice)
            {
            }

            void read(PatchFunc, void*) override;
            void throwException(UserExceptionFactory) override;

            void startInstance(SliceType) override;
            SlicedDataPtr endInstance() override;
            const std::string& startSlice() override;
            void endSlice() override;
            void skipSlice() override;

            void readPendingValues() override;

        private:
            void readInstance();

            // Instance attributes
            SliceType _sliceType;
            bool _skipFirstSlice;

            // Slice attributes
            std::int32_t _sliceSize;
            std::string _typeId;
        };

        class ICE_API EncapsDecoder11 : public EncapsDecoder
        {
        public:
            EncapsDecoder11(
                InputStream* stream,
                Encaps* encaps,
                size_t classGraphDepthMax,
                const Ice::ValueFactoryManagerPtr& f)
                : EncapsDecoder(stream, encaps, classGraphDepthMax, f),
                  _preAllocatedInstanceData(nullptr),
                  _current(nullptr),
                  _valueIdIndex(1)
            {
            }

            void read(PatchFunc, void*) override;
            void throwException(UserExceptionFactory) override;

            void startInstance(SliceType) override;
            SlicedDataPtr endInstance() override;
            const std::string& startSlice() override;
            void endSlice() override;
            void skipSlice() override;

            bool readOptional(std::int32_t, OptionalFormat) override;

        private:
            std::int32_t readInstance(std::int32_t, const PatchFunc&, void*);
            SlicedDataPtr readSlicedData();

            struct IndirectPatchEntry
            {
                std::int32_t index;
                PatchFunc patchFunc;
                void* patchAddr;
            };
            using IndirectPatchList = std::vector<IndirectPatchEntry>;

            using IndexList = std::vector<std::int32_t>;
            using IndexListList = std::vector<IndexList>;

            struct InstanceData
            {
                InstanceData(InstanceData* p) : previous(p), next(nullptr)
                {
                    if (previous)
                    {
                        previous->next = this;
                    }
                }

                ~InstanceData()
                {
                    if (next)
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
                std::uint8_t sliceFlags;
                std::int32_t sliceSize;
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
                if (!_current)
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

            std::int32_t _valueIdIndex; // The ID of the next value to unmarshal.
        };

        class Encaps
        {
        public:
            Encaps() : start(0), decoder(nullptr), previous(nullptr)
            {
                // Inlined for performance reasons.
            }
            Encaps(const Encaps&) = delete;
            ~Encaps()
            {
                // Inlined for performance reasons.
                delete decoder;
            }

            Encaps& operator=(const Encaps&) = delete;

            void reset()
            {
                // Inlined for performance reasons.
                delete decoder;
                decoder = nullptr;

                previous = nullptr;
            }

            Container::size_type start;
            std::int32_t sz;
            EncodingVersion encoding;

            EncapsDecoder* decoder;

            Encaps* previous;
        };

        // Optimization. The instance may not be deleted while a stack-allocated stream still holds it.
        IceInternal::Instance* const _instance;

        //
        // The encoding version to use when there's no encapsulation to
        // read from. This is for example used to read message headers.
        //
        EncodingVersion _encoding;

        Encaps* _currentEncaps;

        void initEncaps();

        Encaps _preAllocatedEncaps;

        // Retrieved during construction and cached.
        const size_t _classGraphDepthMax;

        void* _closure;

        int _startSeq;
        int _minSeqSize;

        // Retrieved from instance during construction and cached. Never null.
        const ValueFactoryManagerPtr _valueFactoryManager;

        std::vector<std::function<void()>> _deleters;
    };

} // End namespace Ice

#endif

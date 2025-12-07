// Copyright (c) ZeroC, Inc.

#ifndef ICE_OUTPUT_STREAM_H
#define ICE_OUTPUT_STREAM_H

#include "Buffer.h"
#include "CommunicatorF.h"
#include "Ice/Format.h"
#include "Ice/StringConverter.h"
#include "Ice/Version.h"
#include "Ice/VersionFunctions.h"
#include "InstanceF.h"
#include "SlicedDataF.h"
#include "StreamableTraits.h"
#include "ValueF.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Ice
{
    class ObjectPrx;

    /// Represents a byte buffer used for marshaling data using the Slice encoding.
    /// @headerfile Ice/Ice.h
    class ICE_API OutputStream : public IceInternal::Buffer
    {
    public:
        /// The size type for this byte buffer.
        using size_type = std::size_t;

        /// Constructs an OutputStream.
        /// @param encoding The encoding version to use.
        /// @param format The class format to use.
        /// @param stringConverter The narrow string converter to use.
        /// @param wstringConverter The wide string converter to use.
        OutputStream(
            EncodingVersion encoding = Encoding_1_1,
            FormatType format = FormatType::CompactFormat,
            StringConverterPtr stringConverter = nullptr,
            WstringConverterPtr wstringConverter = nullptr);

        /// Constructs an OutputStream using the format, string converter and wstring converter provided by the
        /// communicator, and the specified encoding.
        /// @param communicator The communicator.
        /// @param encoding The encoding version to use.
        OutputStream(const CommunicatorPtr& communicator, EncodingVersion encoding);

        /// Constructs an OutputStream using the encoding, format, string converter and wstring converter provided by
        /// the communicator.
        /// @param communicator The communicator.
        OutputStream(const CommunicatorPtr& communicator);

        /// Constructs an OutputStream over an application-supplied buffer.
        /// @param bytes Application-supplied memory that the OutputStream uses as its initial marshaling buffer. The
        /// stream will reallocate if the size of the marshaled data exceeds the application's buffer.
        /// @param encoding The encoding version to use.
        /// @param format The class format to use.
        /// @param stringConverter The narrow string converter to use.
        /// @param wstringConverter The wide string converter to use.
        OutputStream(
            std::pair<const std::byte*, const std::byte*> bytes,
            EncodingVersion encoding = Encoding_1_1,
            FormatType format = FormatType::CompactFormat,
            StringConverterPtr stringConverter = nullptr,
            WstringConverterPtr wstringConverter = nullptr);

        /// Move constructor.
        /// @param other The output stream to move into this output stream.
        OutputStream(OutputStream&& other) noexcept;

        /// Move assignment operator.
        /// @param other The output stream to move into this output stream.
        /// @return A reference to this output stream.
        OutputStream& operator=(OutputStream&& other) noexcept;

        ~OutputStream()
        {
            // Inlined for performance reasons.

            if (_currentEncaps != &_preAllocatedEncaps)
            {
                clear(); // Not inlined.
            }
        }

        /// Releases any data retained by encapsulations.
        void clear();

        /// Swaps the contents of one stream with another.
        /// @param other The other stream.
        void swap(OutputStream& other) noexcept;

        /// @cond INTERNAL

        /// Gets the closure associated with this stream.
        /// @return The closure.
        [[nodiscard]] void* getClosure() const;

        /// Associates a closure with this stream.
        /// @param p The closure
        /// @return The previous closure, or nullptr.
        void* setClosure(void* p);

        void resetEncapsulation();
        /// @endcond

        /// Resizes the stream to a new size.
        /// @param sz The new size.
        void resize(Container::size_type sz) { b.resize(sz); }

        /// Marks the start of a class instance.
        /// @param data Contains the marshaled form of unknown slices from the class instance. If not nullptr, these
        /// slices will be marshaled with the instance.
        void startValue(const SlicedDataPtr& data)
        {
            assert(_currentEncaps && _currentEncaps->encoder);
            _currentEncaps->encoder->startInstance(ValueSlice, data);
        }

        /// Marks the end of a class instance.
        void endValue()
        {
            assert(_currentEncaps && _currentEncaps->encoder);
            _currentEncaps->encoder->endInstance();
        }

        /// Marks the start of an exception instance.
        void startException()
        {
            assert(_currentEncaps && _currentEncaps->encoder);
            _currentEncaps->encoder->startInstance(ExceptionSlice, nullptr);
        }

        /// Marks the end of an exception instance.
        void endException()
        {
            assert(_currentEncaps && _currentEncaps->encoder);
            _currentEncaps->encoder->endInstance();
        }

        /// Writes the start of an encapsulation using the default encoding version and class encoding format.
        void startEncapsulation();

        /// Writes the start of an encapsulation using the specified encoding version and class encoding format.
        /// @param encoding The encoding version to use for the encapsulation.
        /// @param format The class format to use for the encapsulation. nullopt is equivalent to the OutputStream's
        /// class format.
        void startEncapsulation(const EncodingVersion& encoding, std::optional<FormatType> format);

        /// Ends the current encapsulation.
        void endEncapsulation();

        /// Writes an empty encapsulation using the specified encoding version.
        /// @param encoding The encoding version to use for the encapsulation.
        void writeEmptyEncapsulation(const EncodingVersion& encoding);

        /// Copies the marshaled form of an encapsulation to the buffer.
        /// @param v The start of the buffer.
        /// @param sz The number of bytes to copy.
        void writeEncapsulation(const std::byte* v, std::int32_t sz);

        /// Gets the current encoding version.
        /// @return The encoding version.
        [[nodiscard]] const EncodingVersion& getEncoding() const
        {
            return _currentEncaps ? _currentEncaps->encoding : _encoding;
        }

        /// Writes the start of a class instance or exception slice.
        /// @param typeId The Slice type ID for this slice.
        /// @param compactId The compact ID corresponding to the type, or -1 if no compact ID is used.
        /// @param last `true` if this is the last slice, `false` otherwise.
        void startSlice(std::string_view typeId, int compactId, bool last)
        {
            assert(_currentEncaps && _currentEncaps->encoder);
            _currentEncaps->encoder->startSlice(typeId, compactId, last);
        }

        /// Marks the end of a class instance or exception slice.
        void endSlice()
        {
            assert(_currentEncaps && _currentEncaps->encoder);
            _currentEncaps->encoder->endSlice();
        }

        /// Encodes the state of class instances whose insertion was delayed during a previous call `write`. This
        /// function must be called only once. For backward compatibility with encoding version 1.0, this function must
        /// be called only when non-optional fields or parameters use class types.
        void writePendingValues();

        /// Writes a size value.
        /// @param v A non-negative integer.
        void writeSize(std::int32_t v) // Inlined for performance reasons.
        {
            assert(v >= 0);
            if (v > 254)
            {
                write(std::uint8_t(255));
                write(v);
            }
            else
            {
                write(static_cast<std::uint8_t>(v));
            }
        }

        /// Replaces a size value at the specified position in the stream. This function does not change the stream's
        /// current position.
        /// @param v A non-negative integer representing the size.
        /// @param dest The buffer destination for the size.
        void rewriteSize(std::int32_t v, Container::iterator dest)
        {
            assert(v >= 0);
            if (v > 254)
            {
                *dest++ = std::byte{255};
                write(v, dest);
            }
            else
            {
                *dest = static_cast<std::byte>(v);
            }
        }

        /// Writes a placeholder value for the size and returns the starting position of the size value; after writing
        /// the data, call #endSize to patch the placeholder with the actual size at the given position.
        /// @return The starting position of the size value.
        size_type startSize()
        {
            size_type position = b.size();
            write(std::int32_t(0));
            return position;
        }

        /// Updates the size value at the given position. The new size is computed from stream's current position.
        /// @param position The starting position of the size value as returned by #startSize.
        void endSize(size_type position) { rewrite(static_cast<std::int32_t>(b.size() - position) - 4, position); }

        /// Copies the specified blob of bytes to the stream without modification.
        /// @param v The bytes to copy.
        void writeBlob(const std::vector<std::byte>& v);

        /// Copies the specified blob of bytes to the stream without modification.
        /// @param v The start of the buffer to copy.
        /// @param sz The number of bytes to copy.
        void writeBlob(const std::byte* v, Container::size_type sz)
        {
            if (sz > 0)
            {
                Container::size_type position = b.size();
                resize(position + sz);
                memcpy(&b[position], &v[0], sz);
            }
        }

        /// Writes a value to the stream.
        /// @tparam T The type of the value to marshal.
        /// @param v The value to marshal.
        template<typename T> void write(const T& v) { StreamHelper<T, StreamableTraits<T>::helper>::write(this, v); }

#ifdef ICE_DOXYGEN
        /// Writes an optional value to the stream.
        /// @tparam T The type of the value to marshal.
        /// @param tag The tag.
        /// @param v The value to marshal.
        template<typename T> void write(std::int32_t tag, const std::optional<T>& v);

        /// Writes a tuple to the stream.
        /// @tparam I The index of the first element to marshal.
        /// @tparam Te The types of the values in the tuple, starting at index @p I.
        /// @param tuple The tuple to marshal.
        // Declared here because the actual definition below breaks doxygen 1.13.2.
        template<size_t I = 0, typename... Te >> writeAll(std::tuple<Te...> tuple);
#endif

        /// Writes a value (single element list) to the stream.
        /// @tparam T The type of the value.
        /// @param v The value to marshal.
        template<typename T> void writeAll(const T& v) { write(v); }

        /// Writes a list of values to the stream.
        /// @tparam T The type of the first value.
        /// @tparam Te The types of the remaining values.
        /// @param v The first value to marshal.
        /// @param ve The remaining values.
        template<typename T, typename... Te> void writeAll(const T& v, const Te&... ve)
        {
            write(v);
            writeAll(ve...);
        }

        /// Writes an optional value (single element list) to the stream.
        /// @tparam T The type of the value.
        /// @param tags The tag list. The last tag is used to marshal the value.
        /// @param v The value to marshal.
        template<typename T> void writeAll(std::initializer_list<std::int32_t> tags, const std::optional<T>& v)
        {
            write(*(tags.begin() + tags.size() - 1), v);
        }

        /// Writes a list of optional values to the stream.
        /// @tparam T The type of the first value.
        /// @tparam Te The types of the remaining values.
        /// @param tags The tag list.
        /// @param v The first value to marshal.
        /// @param ve The remaining values.
        template<typename T, typename... Te>
        void
        writeAll(std::initializer_list<std::int32_t> tags, const std::optional<T>& v, const std::optional<Te>&... ve)
        {
            size_t index = tags.size() - sizeof...(ve) - 1;
            write(*(tags.begin() + index), v);
            writeAll(tags, ve...);
        }

        /// Writes the tag and format of an optional value.
        /// @param tag The tag.
        /// @param format The optional format.
        /// @return `true` if the current encoding version supports optional values, `false` otherwise.
        /// If `true`, the data associated with the optional value must be written next.
        bool writeOptional(std::int32_t tag, OptionalFormat format)
        {
            assert(_currentEncaps);
            if (_currentEncaps->encoder)
            {
                return _currentEncaps->encoder->writeOptional(tag, format);
            }
            else
            {
                return writeOptImpl(tag, format);
            }
        }

        /// @cond INTERNAL

        // We don't document all the write "specializations" in Doxygen. But we do keep them mostly documented for
        // tool-tips.

        /// Writes an optional value to the stream.
        /// @tparam T The type of the value to marshal. Not for proxy types.
        /// @param tag The tag.
        /// @param v The value to marshal.
        template<typename T, std::enable_if_t<!std::is_base_of_v<ObjectPrx, T>, bool> = true>
        void write(std::int32_t tag, const std::optional<T>& v)
        {
            if (!v)
            {
                return; // Optional not set
            }

            if (writeOptional(
                    tag,
                    StreamOptionalHelper<T, StreamableTraits<T>::helper, StreamableTraits<T>::fixedLength>::
                        optionalFormat))
            {
                StreamOptionalHelper<T, StreamableTraits<T>::helper, StreamableTraits<T>::fixedLength>::write(this, *v);
            }
        }

        /// Writes an optional value to the stream.
        /// @tparam T The proxy type.
        /// @param tag The tag.
        /// @param v The value to marshal.
        template<typename T, std::enable_if_t<std::is_base_of_v<ObjectPrx, T>, bool> = true>
        void write(std::int32_t tag, const std::optional<T>& v)
        {
            if (!v)
            {
                return; // Optional not set
            }

            if (writeOptional(tag, OptionalFormat::FSize))
            {
                size_type pos = startSize();
                writeProxy(*v);
                endSize(pos);
            }
        }

        /// Writes a sequence to the stream.
        /// @tparam T The type of the elements in the sequence.
        /// @param v The sequence to marshal.
        template<typename T> void write(const std::vector<T>& v)
        {
            if (v.empty())
            {
                writeSize(0);
            }
            else
            {
                write(&v[0], &v[0] + v.size());
            }
        }

        /// Writes a sequence to the stream.
        /// @tparam T The type of the elements in the sequence.
        /// @param begin The beginning of the sequence.
        /// @param end The end of the sequence.
        template<typename T> void write(const T* begin, const T* end)
        {
            writeSize(static_cast<std::int32_t>(end - begin));
            for (const T* p = begin; p != end; ++p)
            {
                write(*p);
            }
        }

        // Helper function for the next writeAll.
        template<size_t I = 0, typename... Te> std::enable_if_t<I == sizeof...(Te), void> writeAll(std::tuple<Te...>)
        {
            // Do nothing. Either tuple is empty or we are at the end.
        }

        /// Writes a tuple to the stream.
        /// @tparam I The index of the first element to marshal.
        /// @tparam Te The types of the values in the tuple, starting at index @p I.
        /// @param tuple The tuple to marshal.
        template<size_t I = 0, typename... Te>
            std::enable_if_t < I<sizeof...(Te), void> writeAll(std::tuple<Te...> tuple)
        {
            write(std::get<I>(tuple));
            writeAll<I + 1, Te...>(tuple);
        }

        /// Writes a byte to the stream.
        /// @param v The byte to marshal.
        void write(std::byte v) { b.push_back(v); }

        /// Writes a byte to the stream.
        /// @param v The byte to marshal.
        void write(std::uint8_t v) { b.push_back(std::byte{v}); }

        /// Writes a boolean to the stream.
        /// @param v The boolean to marshal.
        void write(bool v) { b.push_back(static_cast<std::byte>(v)); }

        /// Writes a boolean sequence to the stream.
        /// @param v The sequence to be written.
        void write(const std::vector<bool>& v);

        /// Writes an int16_t as a Slice short.
        /// @param v The int16_t to marshal.
        void write(std::int16_t v);

        /// Writes an int to the stream.
        /// @param v The int to marshal.
        void write(std::int32_t v) // Inlined for performance reasons.
        {
            Container::size_type position = b.size();
            resize(position + sizeof(std::int32_t));
            write(v, &b[position]);
        }

        /// Writes a long to the stream.
        /// @param v The long to marshal.
        void write(std::int64_t v);

        /// Writes a float as a Slice float.
        /// @param v The float to marshal.
        void write(float v);

        /// Writes a double as a Slice double.
        /// @param v The double to marshal.
        void write(double v);

        /// Writes a wide string to the stream.
        /// @param v The wide string to marshal.
        void write(const std::wstring& v) { write(std::wstring_view(v)); }

        /// Writes a wide string view to the stream.
        /// @param v The wide string view to marshal.
        void write(std::wstring_view v);

        /// @endcond

        /// Overwrites a 32-bit integer value at the given position in the stream.
        /// This function does not change the stream's current position.
        /// @param v The integer value to marshal.
        /// @param dest The buffer destination for the integer value.
        void write(std::int32_t v, Container::iterator dest);

        /// Writes a byte sequence to the stream.
        /// @param start The beginning of the sequence.
        /// @param end The end of the sequence.
        void write(const std::byte* start, const std::byte* end);

        /// Writes a byte sequence to the stream.
        /// @param start The beginning of the sequence.
        /// @param end The end of the sequence.
        void write(const std::uint8_t* start, const std::uint8_t* end);

        /// Writes a boolean sequence to the stream.
        /// @param begin The beginning of the sequence.
        /// @param end The end of the sequence.
        void write(const bool* begin, const bool* end);

        /// Writes an int16_t sequence as a Slice short sequence.
        /// @param begin The beginning of the sequence.
        /// @param end The end of the sequence.
        void write(const std::int16_t* begin, const std::int16_t* end);

        /// Writes an int sequence to the stream.
        /// @param begin The beginning of the sequence.
        /// @param end The end of the sequence.
        void write(const std::int32_t* begin, const std::int32_t* end);

        /// Writes a long sequence to the stream.
        /// @param begin The beginning of the sequence.
        /// @param end The end of the sequence.
        void write(const std::int64_t* begin, const std::int64_t* end);

        /// Writes a float sequence as a Slice float sequence.
        /// @param begin The beginning of the sequence.
        /// @param end The end of the sequence.
        void write(const float* begin, const float* end);

        /// Writes a double sequence as a Slice double sequence.
        /// @param begin The beginning of the sequence.
        /// @param end The end of the sequence.
        void write(const double* begin, const double* end);

        /// Writes a string to the stream.
        /// @param v The string to marshal.
        /// @param convert `true` to process @p v through the narrow string converter (if not null), `false` to write
        /// @p v as-is.
        void write(const std::string& v, bool convert = true) { write(std::string_view(v), convert); }

        /// Writes a string view to the stream.
        /// @param v The string view to marshal.
        /// @param convert `true` to process @p v through the narrow string converter (if not null), `false` to write
        /// @p v as-is.
        void write(std::string_view v, bool convert = true)
        {
            auto sz = static_cast<std::int32_t>(v.size());
            if (convert && sz > 0)
            {
                writeConverted(v.data(), static_cast<size_t>(sz));
            }
            else
            {
                writeSize(sz);
                if (sz > 0)
                {
                    Container::size_type position = b.size();
                    resize(position + static_cast<size_t>(sz));
                    memcpy(&b[position], v.data(), static_cast<size_t>(sz));
                }
            }
        }

        /// Writes a string to the stream.
        /// @param vdata The string to marshal.
        /// @param vsize The size of the string.
        /// @param convert `true` to process @p vdata through the narrow string converter (if not null), `false` to
        /// write @p vdata as-is.
        void write(const char* vdata, size_t vsize, bool convert = true)
        {
            auto sz = static_cast<std::int32_t>(vsize);
            if (convert && sz > 0)
            {
                writeConverted(vdata, vsize);
            }
            else
            {
                writeSize(sz);
                if (sz > 0)
                {
                    Container::size_type position = b.size();
                    resize(position + static_cast<size_t>(sz));
                    memcpy(&b[position], vdata, vsize);
                }
            }
        }

        /// Writes a string to the stream.
        /// @param vdata The null-terminated string to marshal.
        /// @param convert `true` to process @p vdata through the narrow string converter (if not null), `false` to
        /// write @p vdata as-is.
        void write(const char* vdata, bool convert = true) { write(vdata, strlen(vdata), convert); }

        /// Writes a string sequence to the stream.
        /// @param begin The beginning of the sequence.
        /// @param end The end of the sequence.
        /// @param convert `true` to process the strings through the narrow string converter (if not null), `false` to
        /// write the strings as-is.
        void write(const std::string* begin, const std::string* end, bool convert = true);

        /// Writes a wide string sequence to the stream.
        /// @param begin The beginning of the sequence.
        /// @param end The end of the sequence.
        void write(const std::wstring* begin, const std::wstring* end);

        /// Writes a proxy to the stream.
        /// @param v The proxy to be written.
        void writeProxy(const ObjectPrx& v);

        /// Writes a null proxy to the stream.
        void writeNullProxy();

        /// @cond INTERNAL

        /// Writes a proxy to the stream.
        /// @param v The proxy to marshal.
        template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        void write(const std::optional<Prx>& v)
        {
            if (v)
            {
                writeProxy(v.value());
            }
            else
            {
                writeNullProxy();
            }
        }

        /// Writes a value instance to the stream.
        /// @param v The value to marshal.
        template<typename T, std::enable_if_t<std::is_base_of_v<Value, T>>* = nullptr>
        void write(const std::shared_ptr<T>& v)
        {
            initEncaps();
            _currentEncaps->encoder->write(v);
        }

        /// @endcond

        /// Writes an enumerator to the stream.
        /// @param v The enumerator to marshal.
        /// @param maxValue The maximum value of all enumerators in this enumeration.
        void writeEnum(std::int32_t v, std::int32_t maxValue);

        /// Writes a user exception to the stream.
        /// @param v The exception to marshal.
        void writeException(const UserException& v);

        /// Gets the current position of the stream.
        /// @return The current position.
        size_type pos() { return b.size(); }

        /// Overwrites a 32-bit integer value at the specified position in the stream. This function does not change the
        /// stream's current position.
        /// @param v The value to marshal.
        /// @param pos The buffer position for the value.
        void rewrite(std::int32_t v, size_type pos) { write(v, b.begin() + pos); }

        /// Indicates that marshaling is complete. This function must only be called once.
        /// @param[out] v Filled with a copy of the encoded data.
        void finished(std::vector<std::byte>& v);

        /// Indicates that marshaling is complete. This function must only be called once.
        /// @return A pair of pointers into the internal marshaling buffer. These pointers are valid for the lifetime
        /// of the stream.
        std::pair<const std::byte*, const std::byte*> finished();

        /// @private
        OutputStream(IceInternal::Instance*, EncodingVersion encoding);

    private:
        // Optionals
        bool writeOptImpl(std::int32_t, OptionalFormat);

        //
        // String
        //
        void writeConverted(const char*, size_t);

        StringConverterPtr _stringConverter;
        WstringConverterPtr _wstringConverter;

        //
        // The public stream API needs to attach data to a stream.
        //
        void* _closure;

        class Encaps;
        enum SliceType
        {
            NoSlice,
            ValueSlice,
            ExceptionSlice
        };

        using ValueList = std::vector<ValuePtr>;

        class ICE_API EncapsEncoder
        {
        public:
            EncapsEncoder(const EncapsEncoder&) = delete;
            virtual ~EncapsEncoder();

            EncapsEncoder& operator=(const EncapsEncoder&) = delete;

            virtual void write(const ValuePtr&) = 0;
            virtual void write(const UserException&) = 0;

            virtual void startInstance(SliceType, const SlicedDataPtr&) = 0;
            virtual void endInstance() = 0;
            virtual void startSlice(std::string_view, int, bool) = 0;
            virtual void endSlice() = 0;

            virtual bool writeOptional(std::int32_t, OptionalFormat) { return false; }

            virtual void writePendingValues() {}

        protected:
            EncapsEncoder(OutputStream* stream, Encaps* encaps) : _stream(stream), _encaps(encaps) {}

            std::int32_t registerTypeId(std::string_view);

            OutputStream* _stream;
            Encaps* _encaps;

            using PtrToIndexMap = std::map<ValuePtr, std::int32_t>;
            using TypeIdMap = std::map<std::string, std::int32_t, std::less<>>;

            // Encapsulation attributes for value marshaling.
            PtrToIndexMap _marshaledMap;

        private:
            // Encapsulation attributes for value marshaling.
            TypeIdMap _typeIdMap;
            std::int32_t _typeIdIndex{0};
        };

        class ICE_API EncapsEncoder10 : public EncapsEncoder
        {
        public:
            EncapsEncoder10(OutputStream* stream, Encaps* encaps) : EncapsEncoder(stream, encaps) {}

            void write(const ValuePtr&) override;
            void write(const UserException&) override;

            void startInstance(SliceType, const SlicedDataPtr&) override;
            void endInstance() override;
            void startSlice(std::string_view, int, bool) override;
            void endSlice() override;

            void writePendingValues() override;

        private:
            std::int32_t registerValue(const ValuePtr&);

            // Instance attributes
            SliceType _sliceType{NoSlice};

            // Slice attributes
            Container::size_type _writeSlice{0}; // Position of the slice data members

            // Encapsulation attributes for value marshaling.
            std::int32_t _valueIdIndex{0};
            PtrToIndexMap _toBeMarshaledMap;
        };

        class ICE_API EncapsEncoder11 : public EncapsEncoder
        {
        public:
            EncapsEncoder11(OutputStream* stream, Encaps* encaps)
                : EncapsEncoder(stream, encaps),
                  _preAllocatedInstanceData(nullptr)
            {
            }

            void write(const ValuePtr&) override;
            void write(const UserException&) override;

            void startInstance(SliceType, const SlicedDataPtr&) override;
            void endInstance() override;
            void startSlice(std::string_view, int, bool) override;
            void endSlice() override;

            bool writeOptional(std::int32_t, OptionalFormat) override;

        private:
            void writeSlicedData(const SlicedDataPtr&);
            void writeInstance(const ValuePtr&);

            struct InstanceData
            {
                InstanceData(InstanceData* p) : previous(p)
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
                SliceType sliceType{NoSlice};
                bool firstSlice{false};

                // Slice attributes
                std::uint8_t sliceFlags{0};
                Container::size_type writeSlice{0};    // Position of the slice data members
                Container::size_type sliceFlagsPos{0}; // Position of the slice flags
                PtrToIndexMap indirectionMap;
                ValueList indirectionTable;

                InstanceData* previous{nullptr};
                InstanceData* next{nullptr};
            };
            InstanceData _preAllocatedInstanceData;
            InstanceData* _current{nullptr};

            std::int32_t _valueIdIndex{1}; // The ID of the next value to marshal
        };

        class Encaps
        {
        public:
            Encaps() = default;
            Encaps(const Encaps&) = delete;
            ~Encaps() { delete encoder; }
            Encaps& operator=(const Encaps&) = delete;

            void reset()
            {
                // Inlined for performance reasons.
                delete encoder;
                encoder = nullptr;

                previous = nullptr;
            }

            Container::size_type start;
            EncodingVersion encoding;
            FormatType format{FormatType::CompactFormat};

            EncapsEncoder* encoder{nullptr};

            Encaps* previous{nullptr};
        };

        //
        // The encoding version to use when there's no encapsulation to
        // read from or write to. This is for example used to read message
        // headers or when the user is using the streaming API with no
        // encapsulation.
        //
        EncodingVersion _encoding;

        FormatType _format; // TODO: make it const

        Encaps* _currentEncaps;

        void initEncaps();

        Encaps _preAllocatedEncaps;
    };

} // End namespace Ice

#endif

// Copyright (c) ZeroC, Inc.

#ifndef DATASTORM_TYPES_H
#define DATASTORM_TYPES_H

#include "Config.h"
#include "Ice/Ice.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace DataStorm
{
    /// The discard policy specifies how samples are discarded by readers upon receive.
    enum struct DiscardPolicy
    {
        /// Samples are never discarded.
        None,

        /// Samples are discared based on the sample timestamp. If the received sample timestamp is older than the
        /// last received sample, the sample is discarded. This ensures that readers will eventually always end up
        /// with the same view of the data if multiple writers are sending samples.
        SendTime,

        /// Samples are discarded based on the writer priority. Only samples from the highest priority connected
        /// writers are kept, others are discarded.
        Priority
    };

    /// The clear history policy specifies when the history is cleared. The history can be cleared based on the
    /// event of the received sample.
    enum struct ClearHistoryPolicy
    {
        /// Clear the sample history when a Add sample is received.
        OnAdd,

        /// Clear the sample history when a Remove sample is received.
        OnRemove,

        /// Clear the sample history when a new sample is received.
        OnAll,

        /// Clear the sample history when a new sample which is not a partial update is received.
        OnAllExceptPartialUpdate,

        /// Never clear the sample history.
        Never
    };

    /// The configuration base class holds configuration options common to readers and writers.
    /// @headerfile DataStorm/DataStorm.h
    class Config
    {
    public:
        /// Constructs a Config object.
        /// @param sampleCount The optional sample count.
        /// @param sampleLifetime The optional sample lifetime.
        /// @param clearHistory The optional clear history policy.
        Config(
            std::optional<int> sampleCount = std::nullopt,
            std::optional<int> sampleLifetime = std::nullopt,
            std::optional<ClearHistoryPolicy> clearHistory = std::nullopt) noexcept
            : sampleCount{sampleCount},
              sampleLifetime{sampleLifetime},
              clearHistory{clearHistory}
        {
        }

        /// The sampleCount configuration specifies how many samples are kept by the reader or writer in its sample
        /// history. By default, the sample count is unlimited.
        std::optional<int> sampleCount;

        /// The sampleLifetime configuration specifies samples to keep in the writer or reader history based on
        /// their age. Samples with a timestamp older than the sampleLifetime value (in milliseconds) are discarded
        /// from the history. By default, the samples are kept for an unlimited amount of time.
        std::optional<int> sampleLifetime;

        /// The clear history policy specifies when samples are removed from the sample history. By default,
        /// samples are removed when a new sample is is received which effectively disables the sample history.
        std::optional<ClearHistoryPolicy> clearHistory;
    };

    /// The ReaderConfig class specifies configuration options specific to readers.
    /// It extends the Config class and therefore inherits its configuration options.
    /// @headerfile DataStorm/DataStorm.h
    class ReaderConfig : public Config
    {
    public:
        /// Constructs a ReaderConfig object.
        /// @param sampleCount The optional sample count.
        /// @param sampleLifetime The optional sample lifetime.
        /// @param clearHistory The optional clear history policy.
        /// @param discardPolicy The discard policy.
        ReaderConfig(
            std::optional<int> sampleCount = std::nullopt,
            std::optional<int> sampleLifetime = std::nullopt,
            std::optional<ClearHistoryPolicy> clearHistory = std::nullopt,
            std::optional<DiscardPolicy> discardPolicy = std::nullopt) noexcept
            : Config{sampleCount, sampleLifetime, clearHistory},
              discardPolicy{discardPolicy}
        {
        }

        /// Specifies if and how samples are discarded after being received by a reader.
        std::optional<DiscardPolicy> discardPolicy;
    };

    /// The WriterConfig class specifies configuration options specific to writers.
    /// It extends the Config class and therefore inherits its configuration
    /// options.
    /// @headerfile DataStorm/DataStorm.h
    class WriterConfig : public Config
    {
    public:
        /// Constructs a WriterConfig object.
        /// @param sampleCount The optional sample count.
        /// @param sampleLifetime The optional sample lifetime.
        /// @param clearHistory The optional clear history policy.
        /// @param priority The writer priority.
        WriterConfig(
            std::optional<int> sampleCount = std::nullopt,
            std::optional<int> sampleLifetime = std::nullopt,
            std::optional<ClearHistoryPolicy> clearHistory = std::nullopt,
            std::optional<int> priority = std::nullopt) noexcept
            : Config{sampleCount, sampleLifetime, clearHistory},
              priority{priority}
        {
        }

        /// Specifies the writer priority. The priority is used by readers using the priority discard policy.
        std::optional<int> priority;
    };

    /// The callback action enumerator specifies the reason why a callback is called.
    enum struct CallbackReason
    {
        /// The callback is called because of connection.
        Connect,

        /// The callback is called because of a disconnection.
        Disconnect
    };

    /// The Encoder template provides a method to encode decode user types.
    /// The encoder template can be specialized to provide encoding for types that don't support being encoded with
    /// Ice. By default, the Ice encoding is used if no Encoder template specialization is provided for the type.
    /// @headerfile DataStorm/DataStorm.h
    template<typename T, typename Enabler = void> struct Encoder
    {
        /// Encodes the given value. This method encodes the given value and returns the resulting byte sequence.
        /// @param communicator The communicator associated with the node.
        /// @param value The value to encode.
        /// @return The resulting byte sequence.
        static Ice::ByteSeq encode(const Ice::CommunicatorPtr& communicator, const T& value) noexcept;
    };

    /// The Decoder template provides a method to decode user types.
    /// The decoder template can be specialized to provide decoding for types that don't support being decoded with
    /// Ice. By default, the Ice decoding is used if no Decoder template specialization is provided for the type.
    /// @headerfile DataStorm/DataStorm.h
    template<typename T, typename Enabler = void> struct Decoder
    {
        /// Decodes a value. This method decodes the given byte sequence and returns the resulting value.
        /// @param communicator The communicator associated with the node.
        /// @param value The byte sequence to decode.
        /// @return The resulting value.
        static T decode(const Ice::CommunicatorPtr& communicator, const Ice::ByteSeq& value) noexcept;
    };

    /// The Cloner template provides a method to clone user types.
    /// The cloner template can be specialized to provide cloning for types that require special cloning. By
    /// default, the template uses plain C++ copy.
    /// @headerfile DataStorm/DataStorm.h
    template<typename T, typename Enabler = void> struct Cloner
    {
        /// Clones the given value. This helper is used when processing partial update to clone the previous value
        /// and compute the new value with the partial update. The default implementation performs a plain C++ copy
        /// with the copy constructor.
        /// @param value The value to encode
        /// @return The cloned value
        static T clone(const T& value) noexcept { return value; }
    };

    /// Cloner template specialization to clone shared Ice values using ice_clone.
    template<typename T> struct Cloner<std::shared_ptr<T>, typename std::enable_if_t<std::is_base_of_v<Ice::Value, T>>>
    {
        static std::shared_ptr<T> clone(const std::shared_ptr<T>& value) noexcept { return value->ice_clone(); }
    };

    // Encoder template implementation.
    template<typename T, typename E>
    Ice::ByteSeq Encoder<T, E>::encode(const Ice::CommunicatorPtr& communicator, const T& value) noexcept
    {
        Ice::ByteSeq v;
        Ice::OutputStream stream(communicator);
        stream.write(value);
        stream.finished(v);
        return v;
    }

    // Decoder template implementation.
    template<typename T, typename E>
    T Decoder<T, E>::decode(const Ice::CommunicatorPtr& communicator, const Ice::ByteSeq& value) noexcept
    {
        T v;
        if (value.empty())
        {
            v = T();
        }
        else
        {
            Ice::InputStream(communicator, value).read(v);
        }
        return v;
    }
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif

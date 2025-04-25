// Copyright (c) ZeroC, Inc.

#ifndef ICE_SLICED_DATA_H
#define ICE_SLICED_DATA_H

#include "Config.h"
#include "SlicedDataF.h"
#include "Value.h"

#include <cassert>
#include <string>

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace Ice
{
    /// Encapsulates the details of a class or exception slice with an unknown type.
    /// @headerfile Ice/Ice.h
    struct SliceInfo
    {
        /// The Slice type ID for this slice. It's empty when the compact ID is set (not `-1`).
        const std::string typeId;

        /// The Slice compact type ID for this slice. No compact ID is encoded as `-1`.
        const int compactId;

        /// The encoded bytes for this slice, including the leading size integer.
        const std::vector<std::byte> bytes;

        /// The class instances referenced by this slice.
        std::vector<ValuePtr> instances;

        /// Whether or not the slice contains optional members.
        const bool hasOptionalMembers;

        /// Whether or not this is the last slice.
        const bool isLastSlice;

        /// Constructs a new SliceInfo instance.
        /// @param typeId The Slice type ID for this slice.
        /// @param compactId The Slice compact type ID for this slice.
        /// @param bytes The encoded bytes for this slice.
        /// @param hasOptionalMembers Whether or not the slice contains optional members.
        /// @param isLastSlice Whether or not this is the last slice.
        SliceInfo(
            std::string typeId,
            int compactId,
            std::vector<std::byte> bytes,
            bool hasOptionalMembers,
            bool isLastSlice) noexcept
            : typeId(std::move(typeId)),
              compactId(compactId),
              bytes(std::move(bytes)),
              hasOptionalMembers(hasOptionalMembers),
              isLastSlice(isLastSlice)
        {
            if (compactId == -1)
            {
                assert(!this->typeId.empty());
            }
            else
            {
                assert(this->typeId.empty());
            }
        }
    };

    /// Holds class slices that cannot be unmarshaled because their types are not known locally.
    /// @headerfile Ice/Ice.h
    class ICE_API SlicedData final
    {
    public:
        /// Constructs a SlicedData instance with the given slices.
        /// @param slices The slices of the unknown class.
        SlicedData(SliceInfoSeq slices) noexcept;

        /// The slices of the unknown class.
        const SliceInfoSeq slices;

        /// Clears the slices to break potential cyclic references.
        void clear();
    };

    /// Represents an instance of an unknown class.
    /// @headerfile Ice/Ice.h
    class ICE_API UnknownSlicedValue final : public Value
    {
    public:
        /// Constructs the placeholder instance.
        /// @param unknownTypeId The Slice type ID of the unknown value.
        UnknownSlicedValue(std::string unknownTypeId) noexcept;

        /// Determine the Slice type ID associated with this instance.
        /// @return The type ID supplied to the constructor.
        [[nodiscard]] const char* ice_id() const noexcept final;

        /// Clones this object.
        /// @return A new instance.
        [[nodiscard]] UnknownSlicedValuePtr ice_clone() const
        {
            return std::static_pointer_cast<UnknownSlicedValue>(_iceCloneImpl());
        }

        void ice_printFields(std::ostream& os) const final;

    private:
        [[nodiscard]] ValuePtr _iceCloneImpl() const final;

        std::string _unknownTypeId;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif

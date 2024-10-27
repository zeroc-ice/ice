//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SLICED_DATA_H
#define ICE_SLICED_DATA_H

#include "Config.h"
#include "SlicedDataF.h"
#include "Value.h"

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
    /**
     * Encapsulates the details of a slice with an unknown type.
     * \headerfile Ice/Ice.h
     */
    struct SliceInfo
    {
        /**
         * The Slice type ID for this slice.
         */
        const std::string typeId;

        /**
         * The Slice compact type ID for this slice.
         */
        const int compactId;

        /**
         * The encoded bytes for this slice, including the leading size integer.
         */
        const std::vector<std::byte> bytes;

        /**
         * The class instances referenced by this slice.
         */
        std::vector<ValuePtr> instances;

        /**
         * Whether or not the slice contains optional members.
         */
        const bool hasOptionalMembers;

        /**
         * Whether or not this is the last slice.
         */
        const bool isLastSlice;

        /**
         * Constructs a new SliceInfo instance.
         * @param typeId The Slice type ID for this slice.
         * @param compactId The Slice compact type ID for this slice.
         * @param bytes The encoded bytes for this slice.
         * @param hasOptionalMembers Whether or not the slice contains optional members.
         * @param isLastSlice Whether or not this is the last slice.
         */
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
        }
    };

    /**
     * Holds class slices that cannot be unmarshaled because their types are not known locally.
     * \headerfile Ice/Ice.h
     */
    class ICE_API SlicedData final
    {
    public:
        SlicedData(SliceInfoSeq slices) noexcept;

        /** The slices of the unknown class. */
        const SliceInfoSeq slices;

        /**
         * Clears the slices to break potential cyclic references.
         */
        void clear();
    };

    /**
     * Represents an instance of an unknown class.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnknownSlicedValue final : public Value
    {
    public:
        /**
         * Constructs the placeholder instance.
         * @param unknownTypeId The Slice type ID of the unknown value.
         */
        UnknownSlicedValue(std::string unknownTypeId) noexcept;

        /**
         * Determine the Slice type ID associated with this instance.
         * @return The type ID supplied to the constructor.
         */
        const char* ice_id() const noexcept final;

        /**
         * Clones this object.
         * @return A new instance.
         */
        UnknownSlicedValuePtr ice_clone() const
        {
            return std::static_pointer_cast<UnknownSlicedValue>(_iceCloneImpl());
        }

    protected:
        /// \cond INTERNAL
        ValuePtr _iceCloneImpl() const final;
        /// \endcond

    private:
        std::string _unknownTypeId;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif

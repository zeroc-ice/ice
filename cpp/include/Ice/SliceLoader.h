// Copyright (c) ZeroC, Inc.

#ifndef ICE_SLICE_LOADER_H
#define ICE_SLICE_LOADER_H

#include "Config.h"
#include "ValueF.h"

#include <exception>
#include <memory>
#include <string_view>
#include <vector>

namespace Ice
{
    /// Creates class and exception instances from Slice type IDs.
    /// @remark The implementation of newClassInstance and newExceptionInstance in the base class always returns
    /// nullptr.
    /// @headerfile Ice/Ice.h
    class ICE_API SliceLoader
    {
    public:
        virtual ~SliceLoader();

        /// Creates an instance of a class mapped from a Slice class based on a Slice type ID.
        /// @param typeId The Slice type ID or compact type ID.
        /// @return A new instance of the class identified by @p typeId, or nullptr if the implementation cannot find
        /// the corresponding class.
        /// @throws MarshalException Thrown when the corresponding class was found but its instantiation failed.
        virtual ValuePtr newClassInstance(std::string_view typeId) const;

        /// Creates an instance of a class mapped from a Slice exception based on a Slice type ID.
        /// @param typeId The Slice type ID.
        /// @return A new instance of the exception class identified by @p typeId, or nullptr if the implementation
        /// cannot find the corresponding class.
        /// @throws MarshalException Thrown when the corresponding class was found but its instantiation failed.
        virtual std::exception_ptr newExceptionInstance(std::string_view typeId) const;

    protected:
        SliceLoader() = default;
    };

    using SliceLoaderPtr = std::shared_ptr<SliceLoader>;

    /// Implements SliceLoader by combining multiple SliceLoader instances.
    class ICE_API CompositeSliceLoader final : public SliceLoader
    {
    public:
        virtual ~CompositeSliceLoader();

        /// Adds a SliceLoader instance to this CompositeSliceLoader.
        /// @param loader The SliceLoader to add.
        void add(SliceLoaderPtr loader) noexcept;

        ValuePtr newClassInstance(std::string_view typeId) const final;
        std::exception_ptr newExceptionInstance(std::string_view typeId) const final;

    private:
        std::vector<SliceLoaderPtr> _loaders;
    };

    using CompositeSliceLoaderPtr = std::shared_ptr<CompositeSliceLoader>;
}

#endif

// Copyright (c) ZeroC, Inc.

#ifndef ICE_RUBY_DEFAULT_SLICE_LOADER_H
#define ICE_RUBY_DEFAULT_SLICE_LOADER_H

#include "Config.h"
#include "Ice/Ice.h"

namespace IceRuby
{
    /// Instantiates generated Python classes/exceptions based on the Slice type ID.
    class DefaultSliceLoader final : public Ice::SliceLoader
    {
    public:
        static Ice::SliceLoaderPtr instance();

        [[nodiscard]] Ice::ValuePtr newClassInstance(std::string_view typeId) const final;
        [[nodiscard]] std::exception_ptr newExceptionInstance(std::string_view typeId) const final;

    private:
        DefaultSliceLoader() = default;
    };
}

#endif

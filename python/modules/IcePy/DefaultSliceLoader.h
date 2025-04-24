// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_DEFAULT_SLICE_LOADER_H
#define ICEPY_DEFAULT_SLICE_LOADER_H

#include "Config.h"
#include "Ice/Ice.h"

namespace IcePy
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

// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_DEFAULT_SLICE_LOADER_H
#define ICEPY_DEFAULT_SLICE_LOADER_H

#include "Config.h"
#include "Util.h"
#include "Ice/Ice.h"

#include <map>
#include <mutex>

namespace IcePy
{
    class DefaultSliceLoader final : public Ice::SliceLoader
    {
    public:
        [[nodiscard]] Ice::ValuePtr newClassInstance(std::string_view typeId) const final;
        // [[nodiscard]] std::exception_ptr newExceptionInstance(std::string_view typeId) const final;
    };
}

#endif

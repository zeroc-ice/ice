// Copyright (c) ZeroC, Inc.

#ifndef ICEPHP_DEFAULT_SLICE_LOADER_H
#define ICEPHP_DEFAULT_SLICE_LOADER_H

#include "Communicator.h"
#include "Config.h"
#include "Ice/Ice.h"

namespace IcePHP
{
    /// Instantiates generated PHP classes/exceptions based on the Slice type ID.
    class DefaultSliceLoader final : public Ice::SliceLoader
    {
    public:
        explicit DefaultSliceLoader(const CommunicatorInfoPtr& communicatorInfo);

        [[nodiscard]] Ice::ValuePtr newClassInstance(std::string_view typeId) const final;
        [[nodiscard]] std::exception_ptr newExceptionInstance(std::string_view typeId) const final;

    private:
        std::weak_ptr<CommunicatorInfo> _communicatorInfo;
    };
}

#endif

// Copyright (c) ZeroC, Inc.

#ifndef ICEPHP_PHP_SLICE_LOADER_H
#define ICEPHP_PHP_SLICE_LOADER_H

#include "Communicator.h"
#include "Config.h"
#include "Ice/Ice.h"

namespace IcePHP
{
    /// Implements SliceLoader using a PHP SliceLoader.
    class PHPSliceLoader final : public Ice::SliceLoader
    {
    public:
        PHPSliceLoader(zval* sliceLoader, const CommunicatorInfoPtr& communicatorInfo);
        ~PHPSliceLoader() final;

        [[nodiscard]] Ice::ValuePtr newClassInstance(std::string_view typeId) const final;

        // TODO: we currently don't support loading of custom Slice exceptions from PHP.

    private:
        zval _sliceLoader;
        std::weak_ptr<CommunicatorInfo> _communicatorInfo;
    };
}

#endif

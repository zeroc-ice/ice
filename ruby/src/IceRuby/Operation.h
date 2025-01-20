// Copyright (c) ZeroC, Inc.

#ifndef ICE_RUBY_OPERATION_H
#define ICE_RUBY_OPERATION_H

#include "Config.h"
#include "Ice/ProxyF.h"

#include <memory>

namespace IceRuby
{
    class Operation
    {
    public:
        virtual ~Operation();

        virtual VALUE invoke(const Ice::ObjectPrx&, VALUE, VALUE) = 0;
        virtual void deprecate(const std::string&) = 0;
    };
    using OperationPtr = std::shared_ptr<Operation>;

    bool initOperation(VALUE);

    OperationPtr getOperation(VALUE);
}

#endif

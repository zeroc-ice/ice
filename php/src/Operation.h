// Copyright (c) ZeroC, Inc.

#ifndef ICEPHP_OPERATION_H
#define ICEPHP_OPERATION_H

#include "Config.h"

extern "C"
{
    ZEND_FUNCTION(IcePHP_defineOperation);
}

namespace IcePHP
{
    class Operation
    {
    public:
        virtual zend_function* function() = 0;
    };
    using OperationPtr = std::shared_ptr<Operation>;
}

#endif

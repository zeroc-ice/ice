//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPHP_OPERATION_H
#define ICEPHP_OPERATION_H

#include <Config.h>

//
// Global functions.
//
extern "C"
{
ZEND_FUNCTION(IcePHP_defineOperation);
}

namespace IcePHP
{

class Operation : public IceUtil::Shared
{
public:

    virtual zend_function* function() = 0;

};
typedef IceUtil::Handle<Operation> OperationPtr;

}

#endif

// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_OPERATION_H
#define ICE_RUBY_OPERATION_H

#include <Config.h>
#include <Ice/CommunicatorF.h>

namespace IceRuby
{

class Operation : public IceUtil::Shared
{
public:

    virtual ~Operation();

    virtual VALUE invoke(const Ice::ObjectPrx&, VALUE, VALUE) = 0;
    virtual void deprecate(const std::string&) = 0;
};
typedef IceUtil::Handle<Operation> OperationPtr;

bool initOperation(VALUE);

OperationPtr getOperation(VALUE);

}

#endif

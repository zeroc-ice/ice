//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/IncomingAsync.h>
#include <Ice/ServantLocator.h>
#include <Ice/Object.h>
#include <Ice/ConnectionI.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/ReplyStatus.h>

#include <mutex>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::IncomingAsync::IncomingAsync(IncomingBase& in) :
    IncomingBase(in)
{
}

shared_ptr<IncomingAsync>
IceInternal::IncomingAsync::create(IncomingBase& in)
{
    auto async = make_shared<IncomingAsync>(in);
    in.setAsync(async);
    return async;
}

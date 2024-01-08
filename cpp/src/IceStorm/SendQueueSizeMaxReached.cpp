//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#include "SendQueueSizeMaxReached.h"

#ifdef ICE_CPP11_MAPPING // C++11 mapping

IceStorm::SendQueueSizeMaxReached::~SendQueueSizeMaxReached()
{
}

const ::std::string&
IceStorm::SendQueueSizeMaxReached::ice_staticId()
{
    static const ::std::string typeId = "::IceStorm::SendQueueSizeMaxReached";
    return typeId;
}

#else

IceStorm::SendQueueSizeMaxReached::SendQueueSizeMaxReached(const char* file, int line) :
    ::Ice::LocalException(file, line)
{
}

#ifdef ICE_CPP11_COMPILER
IceStorm::SendQueueSizeMaxReached::~SendQueueSizeMaxReached()
{
}
#else
IceStorm::SendQueueSizeMaxReached::~SendQueueSizeMaxReached() throw()
{
}
#endif

::std::string
IceStorm::SendQueueSizeMaxReached::ice_id() const
{
    return "::IceStorm::SendQueueSizeMaxReached";
}

IceStorm::SendQueueSizeMaxReached*
IceStorm::SendQueueSizeMaxReached::ice_clone() const
{
    return new SendQueueSizeMaxReached(*this);
}

void
IceStorm::SendQueueSizeMaxReached::ice_throw() const
{
    throw *this;
}

#endif

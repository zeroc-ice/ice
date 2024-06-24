//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "IceBox/Service.h"

using namespace std;

const char*
IceBox::FailureException::ice_id() const
{
    return ice_staticId();
}

const char*
IceBox::FailureException::ice_staticId() noexcept
{
    return "::IceBox::FailureException";
}

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "IceBox/Service.h"

using namespace std;

const char*
IceBox::FailureException::ice_id() const noexcept
{
    return "::IceBox::FailureException";
}

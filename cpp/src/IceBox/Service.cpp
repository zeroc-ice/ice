// Copyright (c) ZeroC, Inc.

#include "IceBox/Service.h"

using namespace std;

const char*
IceBox::FailureException::ice_id() const noexcept
{
    return "::IceBox::FailureException";
}

IceBox::Service::~Service() = default; // avoid weak vtable

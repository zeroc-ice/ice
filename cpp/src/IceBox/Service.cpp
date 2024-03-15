//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceBox/Service.h>

using namespace std;

string
IceBox::FailureException::ice_id() const
{
    return string{ice_staticId()};
}

std::string_view
IceBox::FailureException::ice_staticId() noexcept
{
    static constexpr std::string_view typeId = "::IceBox::FailureException";
    return typeId;
}

IceBox::Service::~Service() {}

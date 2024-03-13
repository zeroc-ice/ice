//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "IceBT/Types.h"

using namespace std;

string
IceBT::BluetoothException::ice_id() const
{
    return string{ice_staticId()};
}

void
IceBT::BluetoothException::ice_throw() const
{
    throw *this;
}

std::string_view
IceBT::BluetoothException::ice_staticId() noexcept
{
    static constexpr std::string_view typeId = "::IceBT::BluetoothException";
    return typeId;
}

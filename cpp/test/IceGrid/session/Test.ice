//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Glacier2/PermissionsVerifier.ice>

module Test
{

exception ExtendedPermissionDeniedException extends Glacier2::PermissionDeniedException
{
}

}

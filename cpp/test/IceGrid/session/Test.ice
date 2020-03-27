//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Glacier2/PermissionsVerifier.ice>

module Test
{

exception ExtendedPermissionDeniedException : Glacier2::PermissionDeniedException
{
}

}

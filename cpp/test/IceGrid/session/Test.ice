// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Glacier2/PermissionsVerifier.ice>

module Test
{

exception ExtendedPermissionDeniedException extends Glacier2::PermissionDeniedException
{
}

}

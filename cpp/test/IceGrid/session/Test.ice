// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

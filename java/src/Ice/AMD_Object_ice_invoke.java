// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package Ice;

interface AMD_Object_ice_invoke
{
    void ice_response(boolean ok, byte[] outParams);
    void ice_exception(java.lang.Exception ex);
};

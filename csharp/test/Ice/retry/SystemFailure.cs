// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class SystemFailure : Ice.SystemException
{
    [System.Obsolete("ice_name() is deprecated, use ice_id() instead.")]
    public override string
    ice_name()
    {
        return "SystemFailure";
    }
    
    public override string
    ice_id()
    {
        return "SystemFailure";
    }
};

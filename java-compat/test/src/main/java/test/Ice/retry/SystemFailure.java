// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.retry;

class SystemFailure extends Ice.SystemException
{
    @Override
    public String
    ice_id()
    {
        return "::SystemFailure";
    }
};

// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

[System.Serializable]
public class SystemFailure : Ice.SystemException
{
    public override string
    ice_id()
    {
        return "::SystemFailure";
    }
};

// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public sealed class EI : E_Disp
{
    public EI()
    {
    }
    
    public override string callE(Ice.Current current)
    {
        return "E";
    }
}

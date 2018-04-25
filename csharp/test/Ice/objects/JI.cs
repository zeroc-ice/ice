// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public sealed class JI : Ice.InterfaceByValue
{
    public JI() : base(Test.JDisp_.ice_staticId())
    {
    }
}

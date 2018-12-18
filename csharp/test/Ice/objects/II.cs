// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    namespace objects
    {
        public sealed class II : Ice.InterfaceByValue
        {
            public II() : base(Test.IDisp_.ice_staticId())
            {
            }
        }
    }
}

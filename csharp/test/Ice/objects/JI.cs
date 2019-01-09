// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace objects
    {
        public sealed class JI : Ice.InterfaceByValue
        {
            public JI() : base(Test.JDisp_.ice_staticId())
            {
            }
        }
    }
}

// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    namespace objects
    {
        public sealed class FI : Test.F
        {
            public FI()
            {
            }

            public FI(Test.E e) : base(e, e)
            {
            }

            public bool checkValues()
            {
                return e1 != null && e1 == e2;
            }
        }
    }
}

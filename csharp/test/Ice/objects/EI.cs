// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace objects
    {
        public sealed class EI : Test.E
        {
            public EI() : base(1, "hello")
            {
            }

            public bool checkValues()
            {
                return i == 1 && s.Equals("hello");
            }
        }
    }
}

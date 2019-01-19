//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

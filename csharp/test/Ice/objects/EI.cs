// Copyright (c) ZeroC, Inc.

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
                return i == 1 && s == "hello";
            }
        }
    }
}

// Copyright (c) ZeroC, Inc.

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

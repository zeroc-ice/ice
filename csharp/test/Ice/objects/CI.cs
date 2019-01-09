// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace objects
    {
        public sealed class CI : Test.C
        {
            public override void ice_preMarshal()
            {
                preMarshalInvoked = true;
            }

            public override void ice_postUnmarshal()
            {
                postUnmarshalInvoked = true;
            }
        }
    }
}

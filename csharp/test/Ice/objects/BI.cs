//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace objects
    {
        public sealed class BI : Test.B
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

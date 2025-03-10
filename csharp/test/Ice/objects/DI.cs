// Copyright (c) ZeroC, Inc.

namespace Ice.objects
{



        public sealed class DI : Test.D
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


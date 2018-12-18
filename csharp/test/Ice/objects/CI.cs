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

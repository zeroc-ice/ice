// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;

namespace Test
{
    public sealed class ChecksumI : ChecksumDisp_
    {
        public ChecksumI()
        {
        }

        public override Dictionary<string, string> getSliceChecksums(Ice.Current current)
        {
            return Ice.SliceChecksums.checksums;
        }

        public override void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }
}

// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Test
{
    public sealed class ChecksumI : ChecksumDisp_
    {
        public ChecksumI(Ice.ObjectAdapter adapter)
        {
            _adapter = adapter;
        }

        public override Ice.SliceChecksumDict getSliceChecksums(Ice.Current __current)
        {
            return Ice.SliceChecksums.checksums;
        }

        public override void shutdown(Ice.Current __current)
        {
            _adapter.getCommunicator().shutdown();
        }

        private Ice.ObjectAdapter _adapter;
    }
}

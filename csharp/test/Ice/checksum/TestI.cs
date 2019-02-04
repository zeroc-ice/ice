//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    namespace checksum
    {

        public sealed class ChecksumI : Test.ChecksumDisp_
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
}

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace objects
    {
        public sealed class UnexpectedObjectExceptionTestI : Blobject
        {
            public override bool ice_invoke(byte[] inParams, out byte[] outParams, Ice.Current current)
            {
                var communicator = current.adapter.GetCommunicator();
                var @out = new Ice.OutputStream(communicator);
                @out.startEncapsulation(current.encoding, FormatType.DefaultFormat);
                var ae = new Test.AlsoEmpty();
                @out.writeValue(ae);
                @out.writePendingValues();
                @out.endEncapsulation();
                outParams = @out.finished();
                return true;
            }
        }
    }
}

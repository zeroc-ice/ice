// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public sealed class UnexpectedObjectExceptionTestI : Ice.Blobject
{
    public override bool ice_invoke(byte[] inParams, out byte[] outParams, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();
        Ice.OutputStream @out = Ice.Util.createOutputStream(communicator);
        @out.startEncapsulation(current.encoding, Ice.FormatType.DefaultFormat);
        AlsoEmpty ae = new AlsoEmpty();
        Test.AlsoEmptyHelper.write(@out, ae);
        @out.writePendingObjects();
        @out.endEncapsulation();
        outParams = @out.finished();
        return true;
    }
}

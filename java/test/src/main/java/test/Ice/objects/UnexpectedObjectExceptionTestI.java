// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

import test.Ice.objects.Test.AlsoEmpty;
import test.Ice.objects.Test.AlsoEmptyHelper;


public final class UnexpectedObjectExceptionTestI extends Ice.Blobject
{
    @Override
    public boolean
    ice_invoke(byte[] inParams, Ice.ByteSeqHolder outParams, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();
        Ice.OutputStream out = Ice.Util.createOutputStream(communicator);
        out.startEncapsulation(current.encoding, Ice.FormatType.DefaultFormat);
        AlsoEmpty ae = new AlsoEmpty();
        AlsoEmptyHelper.write(out, ae);
        out.writePendingObjects();
        out.endEncapsulation();
        outParams.value = out.finished();
        return true;
    }
}

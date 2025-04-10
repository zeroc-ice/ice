// Copyright (c) ZeroC, Inc.

package test.Ice.objects;

import com.zeroc.Ice.Blobject;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.FormatType;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.OutputStream;

import test.Ice.objects.Test.AlsoEmpty;

public final class UnexpectedObjectExceptionTestI implements Blobject {
    @Override
    public Object.Ice_invokeResult ice_invoke(
            byte[] inParams, Current current) {
        Object.Ice_invokeResult r = new Object.Ice_invokeResult();
        Communicator communicator = current.adapter.getCommunicator();
        OutputStream out = new OutputStream(communicator);
        out.startEncapsulation(current.encoding, FormatType.SlicedFormat);
        out.writeValue(new AlsoEmpty());
        out.writePendingValues();
        out.endEncapsulation();
        r.outParams = out.finished();
        r.returnValue = true;
        return r;
    }
}

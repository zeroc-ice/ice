//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Diagnostics;
using Ice;

public class BlobjectI : Ice.BlobjectAsync
{
    public override Task<Ice.Object_Ice_invokeResult>
    ice_invokeAsync(byte[] inEncaps, Ice.Current current)
    {
        Debug.Assert(current.Connection != null);
        var prx = current.Connection.createProxy(current.Id).Clone(facet: current.Facet, oneway: current.RequestId == 0);
        return prx.InvokeAsync(current.Operation, current.Mode, inEncaps, current.Context);
    }
}

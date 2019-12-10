//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Ice;

public class BlobjectI : Ice.BlobjectAsync
{
    public override Task<Ice.Object_Ice_invokeResult>
    ice_invokeAsync(byte[] inEncaps, Ice.Current current)
    {
        var prx = current.con.createProxy(current.id).Clone(facet: current.facet, oneway: current.requestId == 0);
        return prx.InvokeAsync(current.operation, current.mode, inEncaps, current.ctx);
    }
}

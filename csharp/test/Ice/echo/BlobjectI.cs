//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

public class BlobjectI : Ice.BlobjectAsync
{
    public override Task<Ice.Object_Ice_invokeResult>
    ice_invokeAsync(byte[] inEncaps, Ice.Current current)
    {
        bool twoway = current.requestId > 0;
        Ice.ObjectPrx obj = current.con.createProxy(current.id);
        if (!twoway)
        {
            if (current.facet.Length != 0)
            {
                obj = obj.ice_facet(current.facet);
            }
            return obj.ice_oneway().ice_invokeAsync(current.operation,
                                                    current.mode,
                                                    inEncaps,
                                                    current.ctx);
        }
        else
        {
            if (current.facet.Length != 0)
            {
                obj = obj.ice_facet(current.facet);
            }
            return obj.ice_invokeAsync(current.operation,
                                       current.mode,
                                       inEncaps,
                                       current.ctx);
        }
    }
}

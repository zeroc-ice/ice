// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.oldevictor;
import test.Freeze.oldevictor.Test.*;

public class FacetI extends ServantI implements _FacetOperations
{
    FacetI(Facet tie)
    {
        super(tie);
    }

    FacetI(Servant tie, RemoteEvictorI remoteEvictor, Freeze.BackgroundSaveEvictor evictor, int value, String data)
    {
        super(tie, remoteEvictor, evictor, value);
        ((Facet)_tie).data = data;
    }
    
    public String
    getData(Ice.Current current)
    {
        synchronized(_tie)
        {
            return ((Facet)_tie).data;
        }
    }

    public void
    setData(String data, Ice.Current current)
    {
        synchronized(_tie)
        {
            ((Facet)_tie).data = data;
        }
    }
}

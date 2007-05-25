// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class FacetI extends ServantI implements Test._FacetOperations
{
    FacetI(Test.Facet tie)
    {
        super(tie);
    }

    FacetI(Test.Servant tie, RemoteEvictorI remoteEvictor, Freeze.BackgroundSaveEvictor evictor, int value, String data)
    {
        super(tie, remoteEvictor, evictor, value);
        ((Test.Facet)_tie).data = data;
    }
    
    public String
    getData(Ice.Current current)
    {
        synchronized(_tie)
        {
            return ((Test.Facet)_tie).data;
        }
    }

    public void
    setData(String data, Ice.Current current)
    {
        synchronized(_tie)
        {
            ((Test.Facet)_tie).data = data;
        }
    }
}

// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public class Incoming
{
    public
    Incoming(Instance instance, Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _is = new StreamI(instance);
        _os = new StreamI(instance);
    }

    public void
    invoke(Stream is)
    {
        _is.swap(is);
        String identity = _is.readString();
        String facet = _is.readString();
        String operation = _is.readString();

        // TODO
    }

    public Stream
    is()
    {
        return _is;
    }

    public Stream
    os()
    {
        return _os;
    }

    private Ice.ObjectAdapter _adapter;

    private Stream _is;
    private Stream _os;
}

// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public final class ServantI extends Test.Servant
{
    ServantI()
    {
    }

    ServantI(RemoteEvictorI remoteEvictor, Freeze.Evictor evictor, int value)
    {
        _remoteEvictor = remoteEvictor;
        _evictor = evictor;
        this.value = value;
    }

    void
    init(RemoteEvictorI remoteEvictor, Freeze.Evictor evictor)
    {
        _remoteEvictor = remoteEvictor;
        _evictor = evictor;
    }

    public void
    destroy(Ice.Current current)
    {
        _evictor.destroyObject(current.id);
    }

    public int
    getValue(Ice.Current current)
    {
        return value;
    }

    public void
    setValue(int value, Ice.Current current)
    {
        this.value = value;
    }

    public void
    __write(IceInternal.BasicStream os)
    {
        _remoteEvictor.setLastSavedValue(value);
        super.__write(os);
    }

    public void
    __marshal(Ice.Stream os)
    {
        _remoteEvictor.setLastSavedValue(value);
        super.__marshal(os);
    }

    private RemoteEvictorI _remoteEvictor;
    private Freeze.Evictor _evictor;
}

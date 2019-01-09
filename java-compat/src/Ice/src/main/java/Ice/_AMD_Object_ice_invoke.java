// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

final class _AMD_Object_ice_invoke extends IceInternal.IncomingAsync implements AMD_Object_ice_invoke
{
    public
    _AMD_Object_ice_invoke(IceInternal.Incoming in)
    {
        super(in);
    }

    @Override
    public void
    ice_response(boolean ok, byte[] outEncaps)
    {
        writeParamEncaps(outEncaps, ok);
        completed();
    }
}

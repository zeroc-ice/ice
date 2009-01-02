// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class SocketStatus
{
    private static SocketStatus[] _values = new SocketStatus[4];

    public static final int _Finished = 0;
    public static final SocketStatus Finished = new SocketStatus(_Finished);
    public static final int _NeedConnect = 1;
    public static final SocketStatus NeedConnect = new SocketStatus(_NeedConnect);
    public static final int _NeedRead = 2;
    public static final SocketStatus NeedRead = new SocketStatus(_NeedRead);
    public static final int _NeedWrite = 3;
    public static final SocketStatus NeedWrite = new SocketStatus(_NeedWrite);

    public int
    value()
    {
        return _value;
    }

    private
    SocketStatus(int val)
    {
        _value = val;
        _values[val] = this;
    }

    private int _value;
}

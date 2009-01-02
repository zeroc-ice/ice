// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class SignalPolicy
{
    private static SignalPolicy[] _values = new SignalPolicy[4];

    public static final int _HandleSignals = 0;
    public static final SignalPolicy HandleSignals = new SignalPolicy(_HandleSignals);
    public static final int _NoSignalHandling = 1;
    public static final SignalPolicy NoSignalHandling = new SignalPolicy(_NoSignalHandling);

    public int
    value()
    {
        return _value;
    }

    private
    SignalPolicy(int val)
    {
        _value = val;
        _values[val] = this;
    }

    private int _value;
}

// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;

public class Callback
{
    internal Callback()
    {
        _called = false;
    }

    public virtual bool check()
    {
        lock(this)
        {
            return _called;
        }
    }

    public virtual bool checkWait()
    {
        int cnt = 0;
        do
        {
            lock(this)
            {
                if(_called)
                {
                    return true;
                }
            }
            Thread.Sleep(100);
        }
        while(++cnt < 50);

        return false; // Must be timeout
    }

    public virtual void called()
    {
        lock(this)
        {
            Debug.Assert(!_called);
            _called = true;
        }
    }

    private bool _called;
}

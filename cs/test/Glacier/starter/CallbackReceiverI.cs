// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;
using System.Threading;

sealed class CallbackReceiverI:CallbackReceiver
{
    internal CallbackReceiverI()
    {
        _callback = false;
    }
    
    public override void callback(Ice.Current current)
    {
        lock(this)
        {
            Debug.Assert(!_callback);
            _callback = true;
            Monitor.Pulse(this);
        }
    }
    
    public override void callbackEx(Ice.Current current)
    {
        callback(current);
        CallbackException ex = new CallbackException();
        ex.someValue = 3.14;
        ex.someString = "3.14";
        throw ex;
    }
    
    internal bool callbackOK()
    {
        lock(this)
        {
            while(!_callback)
            {
		Monitor.Wait(this, 5000);
		if(!_callback)
		{
		    return false;
		}
            }
            
            _callback = false;
            return true;
        }
    }
    
    private bool _callback;
}

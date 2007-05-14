// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

//
// Ensures that only one IncomingAsync is active at a given time for a given request
//

class IncomingAsyncController
{    
    final void
    reset(Incoming in)
    {
        //
        // Always runs in the dispatch thread
        //
        if(_cb != null)
        {
            //
            // May raise ResponseSentException
            //
            _cb.__deactivate(in);
            _cb = null;
        }
    }

    final void
    setActive(IncomingAsync cb)
    {
        assert _cb == null;
        _cb = cb;
    }

    private IncomingAsync _cb;
}

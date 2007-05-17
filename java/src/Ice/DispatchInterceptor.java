// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class DispatchInterceptor extends ObjectImpl
{
    public abstract DispatchStatus
    dispatch(Request request);
   
    public DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        try
        {
            DispatchStatus status = dispatch(in);
            if(status != DispatchStatus.DispatchAsync)
            {
                //
                // Make sure 'in' owns the connection etc.
                //
                in.killAsync();
            }
            return status;
        }
        catch(ResponseSentException e)
        {
            return DispatchStatus.DispatchAsync;
        }
        catch(java.lang.RuntimeException e)
        {
            try
            {
                in.killAsync();
                throw e;
            }
            catch(ResponseSentException rse)
            {
                return DispatchStatus.DispatchAsync;
            }
        }
    }

    public DispatchStatus
    __collocDispatch(IceInternal.Direct request) 
    {
        //
        // Should only return DispatchOK or DispatchUserException
        //
        return dispatch(request);
    }
}

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
    public abstract IceInternal.DispatchStatus
    dispatch(Request request);
   
    public IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        try
        {
            IceInternal.DispatchStatus status = dispatch(in);
            if(status != IceInternal.DispatchStatus.DispatchAsync)
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
            return IceInternal.DispatchStatus.DispatchAsync;
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
                return IceInternal.DispatchStatus.DispatchAsync;
            }
        }
    }

    public IceInternal.DispatchStatus
    __collocDispatch(IceInternal.Direct request) 
    {
        //
        // Should only return DispatchOK or DispatchUserException
        //
        return dispatch(request);
    }
}

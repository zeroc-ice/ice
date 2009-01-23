// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

namespace Ice
{
    public abstract class DispatchInterceptor : Ice.ObjectImpl
    {
        public abstract DispatchStatus 
        dispatch(Request request);

        public override DispatchStatus
        dispatch__(IceInternal.Incoming inc, Current current)
        {
            try
            {
                DispatchStatus status = dispatch(inc);
                if(status != DispatchStatus.DispatchAsync)
                {
                    //
                    // Make sure 'inc' owns the connection etc.
                    //
                    inc.killAsync();
                }
                return status;
            }
            catch(ResponseSentException)
            {
                return DispatchStatus.DispatchAsync;
            }
            catch(System.Exception)
            {
                try
                {
                    inc.killAsync();
                    throw;
                }
                catch(ResponseSentException)
                {
                    return DispatchStatus.DispatchAsync;
                }
            }
        }
        
        public override DispatchStatus
        collocDispatch__(IceInternal.Direct request) 
        {
            //
            // Should only return DispatchOK or DispatchUserException
            //
            return dispatch(request);
        }
    }
}

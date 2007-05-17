// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Diagnostics;

    public class IncomingAsync : IncomingBase
    {
        public IncomingAsync(Incoming inc)
            : base(inc)
        {
        }

        protected void response__(bool ok)
        {
            try
            {
                if(!servantLocatorFinished__())
                {
                    return;
                }

                if(response_)
                {
                    os_.endWriteEncaps();
                
                    int save = os_.pos();
                    os_.pos(Protocol.headerSize + 4); // Reply status position.
                
                    if(ok)
                    {
                        os_.writeByte(ReplyStatus.replyOK);
                    }
                    else
                    {
                        os_.writeByte(ReplyStatus.replyUserException);
                    }
                
                    os_.pos(save);

                    connection_.sendResponse(os_, compress_);
                }
                else
                {
                    connection_.sendNoResponse();
                }
            }
            catch(Ice.LocalException ex)
            {
                connection_.invokeException(ex, 1);
            }
        }
        
        protected internal void exception__(System.Exception exc)
        {
            try
            {
                if(!servantLocatorFinished__())
                {
                    return;
                }

                handleException__(exc);
            }
            catch(Ice.LocalException ex)
            {
                connection_.invokeException(ex, 1);
            }
        }
        
        protected internal BasicStream os__()
        {
            return os_;
        }

        private bool servantLocatorFinished__()
        {
            try
            {
                if(locator_ != null && servant_ != null)
                {
                    locator_.finished(current_, servant_, cookie_);
                }
                return true;
            }
            catch(System.Exception ex)
            {
                handleException__(ex);
                return false;
            }
        }
    }
}

namespace Ice
{

    public interface AMD_Object_ice_invoke
    {
        void ice_response(bool ok, byte[] outParams);
        void ice_exception(System.Exception ex);
    }

    sealed class _AMD_Object_ice_invoke : IceInternal.IncomingAsync, AMD_Object_ice_invoke
    {
        public _AMD_Object_ice_invoke(IceInternal.Incoming inc)
            : base(inc)
        {
        }
        
        public void ice_response(bool ok, byte[] outParams)
        {
            try
            {
                os__().writeBlob(outParams);
            }
            catch(Ice.LocalException ex)
            {
                exception__(ex);
                return;
            }
            response__(ok);
        }
        
        public void ice_exception(System.Exception ex)
        {
            exception__(ex);
        }
    }
}

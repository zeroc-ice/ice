// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
		if(locator_ != null && servant_ != null)
		{
		    locator_.finished(current_, servant_, cookie_);
		}

		if(response_)
		{
		    os_.endWriteEncaps();
		
		    int save = os_.pos();
		    os_.pos(Protocol.headerSize + 4); // Dispatch status position.
		
		    if(ok)
		    {
			os_.writeByte((byte)DispatchStatus.DispatchOK);
		    }
		    else
		    {
			os_.writeByte((byte)DispatchStatus.DispatchUserException);
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
		connection_.exception(ex);
	    }
	    catch(System.Exception ex)
	    {
		Ice.UnknownException uex = new Ice.UnknownException();
		uex.unknown = ex.ToString();
		connection_.exception(uex);
	    }
        }
	
        protected internal void exception__(System.Exception exc)
        {
	    try
	    {
		if(locator_ != null && servant_ != null)
		{
		    locator_.finished(current_, servant_, cookie_);
		}

		try
		{
		    throw exc;
		}
		catch(Ice.RequestFailedException ex)
		{
		    if(ex.id.name == null)
		    {
			ex.id = current_.id;
		    }
		
		    if(ex.facet == null)
		    {
			ex.facet = current_.facet;
		    }
		
		    if(ex.operation == null || ex.operation.Length == 0)
		    {
			ex.operation = current_.operation;
		    }
		
		    if(os_.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
		    {
			warning__(ex);
		    }
		
		    if(response_)
		    {
			os_.endWriteEncaps();
			os_.resize(Protocol.headerSize + 4, false); // Dispatch status position.
			if(ex is Ice.ObjectNotExistException)
			{
			    os_.writeByte((byte)DispatchStatus.DispatchObjectNotExist);
			}
			else if(ex is Ice.FacetNotExistException)
			{
			    os_.writeByte((byte)DispatchStatus.DispatchFacetNotExist);
			}
			else if(ex is Ice.OperationNotExistException)
			{
			    os_.writeByte((byte)DispatchStatus.DispatchOperationNotExist);
			}
			else
			{
			    Debug.Assert(false);
			}
			ex.id.write__(os_);

			//
			// For compatibility with the old FacetPath.
			//
			if(ex.facet == null || ex.facet.Length == 0)
			{
			    os_.writeStringSeq(null);
			}
			else
			{
			    string[] facetPath = { ex.facet };
			    os_.writeStringSeq(facetPath);
			}

			os_.writeString(ex.operation);

			connection_.sendResponse(os_, compress_);
		    }
		    else
		    {
			connection_.sendNoResponse();
		    }
		}
		catch(Ice.UnknownLocalException ex)
		{
		    if(os_.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
		    {
			warning__(ex);
		    }
		
		    if(response_)
		    {
			os_.endWriteEncaps();
			os_.resize(Protocol.headerSize + 4, false); // Dispatch status position.
			os_.writeByte((byte)DispatchStatus.DispatchUnknownLocalException);
			os_.writeString(ex.unknown);
			connection_.sendResponse(os_, compress_);
		    }
		    else
		    {
			connection_.sendNoResponse();
		    }
		}
		catch(Ice.UnknownUserException ex)
		{
		    if(os_.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
		    {
			warning__(ex);
		    }
		
		    if(response_)
		    {
			os_.endWriteEncaps();
			os_.resize(Protocol.headerSize + 4, false); // Dispatch status position.
			os_.writeByte((byte)DispatchStatus.DispatchUnknownUserException);
			os_.writeString(ex.unknown);
			connection_.sendResponse(os_, compress_);
		    }
		    else
		    {
			connection_.sendNoResponse();
		    }
		}
		catch(Ice.UnknownException ex)
		{
		    if(os_.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
		    {
			warning__(ex);
		    }
		
		    if(response_)
		    {
			os_.endWriteEncaps();
			os_.resize(Protocol.headerSize + 4, false); // Dispatch status position.
			os_.writeByte((byte)DispatchStatus.DispatchUnknownException);
			os_.writeString(ex.unknown);
			connection_.sendResponse(os_, compress_);
		    }
		    else
		    {
			connection_.sendNoResponse();
		    }
		}
		catch(Ice.LocalException ex)
		{
		    if(os_.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
		    {
			warning__(ex);
		    }
		
		    if(response_)
		    {
			os_.endWriteEncaps();
			os_.resize(Protocol.headerSize + 4, false); // Dispatch status position.
			os_.writeByte((byte)DispatchStatus.DispatchUnknownLocalException);
			os_.writeString(ex.ToString());
			connection_.sendResponse(os_, compress_);
		    }
		    else
		    {
			connection_.sendNoResponse();
		    }
		}
		catch(Ice.UserException ex)
		{
		    if(os_.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
		    {
			warning__(ex);
		    }
		
		    if(response_)
		    {
			os_.endWriteEncaps();
			os_.resize(Protocol.headerSize + 4, false); // Dispatch status position.
			os_.writeByte((byte)DispatchStatus.DispatchUnknownUserException);
			os_.writeString(ex.ToString());
			connection_.sendResponse(os_, compress_);
		    }
		    else
		    {
			connection_.sendNoResponse();
		    }
		}
		catch(System.Exception ex)
		{
		    if(os_.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
		    {
			warning__(ex);
		    }
		
		    if(response_)
		    {
			os_.endWriteEncaps();
			os_.resize(Protocol.headerSize + 4, false); // Dispatch status position.
			os_.writeByte((byte)DispatchStatus.DispatchUnknownException);
			os_.writeString(ex.ToString());
			connection_.sendResponse(os_, compress_);
		    }
		    else
		    {
			connection_.sendNoResponse();
		    }
		}
	    }
	    catch(Ice.LocalException ex)
	    {
		connection_.exception(ex);
	    }
	    catch(System.Exception ex)
	    {
		Ice.UnknownException uex = new Ice.UnknownException();
		uex.unknown = ex.ToString();
		connection_.exception(uex);
	    }
        }
	
        protected internal BasicStream os__()
        {
            return os_;
        }
    }
}

namespace Ice
{

    public interface AMD_Object_ice_invoke
    {
        void iceresponse_(bool ok, byte[] outParams);
        void ice_exception(System.Exception ex);
    }

    sealed class _AMD_Object_ice_invoke : IceInternal.IncomingAsync, AMD_Object_ice_invoke
    {
        public _AMD_Object_ice_invoke(IceInternal.Incoming inc)
            : base(inc)
        {
        }
	
        public void iceresponse_(bool ok, byte[] outParams)
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

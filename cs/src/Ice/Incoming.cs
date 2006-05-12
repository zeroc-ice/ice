// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;
    using System.Diagnostics;
    using System.IO;

    public class IncomingBase
    {
	protected internal IncomingBase(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter,
					bool response, byte compress, int requestId)
	{
	    response_ = response;
	    compress_ = compress;
	    os_ = new BasicStream(instance);
	    connection_ = connection;

	    current_ = new Ice.Current();
	    current_.id = new Ice.Identity();
	    current_.adapter = adapter;
	    current_.con = connection;
	    current_.requestId = requestId;
	    
	    cookie_ = null;
	}
	
	protected internal IncomingBase(IncomingBase inc) // Adopts the argument. It must not be used afterwards.
	{
	    current_ = inc.current_;
	    
	    servant_ = inc.servant_;
	    inc.servant_ = null;
	    
	    locator_ = inc.locator_;
	    inc.locator_ = null;
	    
	    cookie_ = inc.cookie_;
	    inc.cookie_ = null;
	    
	    response_ = inc.response_;
	    inc.response_ = false;
	    
	    compress_ = inc.compress_;
	    inc.compress_ = 0;
	    
	    os_ = inc.os_;
	    inc.os_ = null;
	    
	    connection_ = inc.connection_;
	    inc.connection_ = null;
	}

	//
	// These functions allow this object to be reused, rather than reallocated.
	//
	public virtual void reset(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter, 
				  bool response, byte compress, int requestId)
	{
            //
            // Don't recycle the Current object, because servants may keep a reference to it.
            //
	    current_ = new Ice.Current();
	    current_.id = new Ice.Identity();
	    current_.adapter = adapter;
	    current_.con = connection;
	    current_.requestId = requestId;
	    
	    Debug.Assert(cookie_ == null);
	    
	    response_ = response;
	    
	    compress_ = compress;
	    
	    if(os_ == null)
	    {
		os_ = new BasicStream(instance);
	    }

	    connection_ = connection;
	}
	
	public virtual void reclaim()
	{
	    servant_ = null;
	    
	    locator_ = null;

	    cookie_ = null;

	    if(os_ != null)
	    {
		os_.reset();
	    }
	}

	protected internal void warning__(System.Exception ex)
	{
	    Debug.Assert(os_ != null);
	    
	    using(StringWriter sw = new StringWriter())
	    {
		IceUtil.OutputBase output = new IceUtil.OutputBase(sw);
		output.setUseTab(false);
		output.print("dispatch exception:");
		output.print("\nidentity: " + os_.instance().identityToString(current_.id));
		output.print("\nfacet: " + IceUtil.StringUtil.escapeString(current_.facet, ""));
		output.print("\noperation: " + current_.operation);
		output.print("\n");
		output.print(ex.ToString());
		os_.instance().initializationData().logger.warning(sw.ToString());
	    }
	}
	
	protected internal Ice.Current current_;
	protected internal Ice.Object servant_;
	protected internal Ice.ServantLocator locator_;
	protected internal Ice.LocalObject cookie_;
	
	protected internal bool response_;
	protected internal byte compress_;
	
	protected internal BasicStream os_;
	
	protected Ice.ConnectionI connection_;
    }
	
    sealed public class Incoming : IncomingBase
    {
	public Incoming(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter,
		        bool response, byte compress, int requestId)
	     : base(instance, connection, adapter, response, compress, requestId)
	{
	    _is = new BasicStream(instance);
	}

	//
	// These functions allow this object to be reused, rather than reallocated.
	//
	public override void reset(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter,
				   bool response, byte compress, int requestId)
	{
	    if(_is == null)
	    {
		_is = new BasicStream(instance);
	    }

	    base.reset(instance, connection, adapter, response, compress, requestId);
	}
	
	public override void reclaim()
	{
	    if(_is != null)
	    {
		_is.reset();
	    }

	    base.reclaim();
	}

	public void invoke(ServantManager servantManager)
	{
	    //
	    // Read the current.
	    //
	    current_.id.read__(_is);

            //
            // For compatibility with the old FacetPath.
            //
            string[] facetPath = _is.readStringSeq();
            if(facetPath.Length > 0)
            {
                if(facetPath.Length > 1)
                {
                    throw new Ice.MarshalException();
                }
                current_.facet = facetPath[0];
            }
            else
            {
                current_.facet = "";
            }

	    current_.operation = _is.readString();
	    current_.mode = (Ice.OperationMode)(int)_is.readByte();
	    int sz = _is.readSize();
	    while(sz-- > 0)
	    {
		string first = _is.readString();
		string second = _is.readString();
		if(current_.ctx == null)
		{
		    current_.ctx = new Ice.Context();
		}
		current_.ctx[first] = second;
	    }
	    
	    _is.startReadEncaps();
	    
	    if(response_)
	    {
		Debug.Assert(os_.size() == Protocol.headerSize + 4); // Dispatch status position.
		os_.writeByte((byte)0);
		os_.startWriteEncaps();
	    }
	    
	    // Initialize status to some value, to keep the compiler happy.
	    DispatchStatus status = DispatchStatus.DispatchOK;
	    
	    //
	    // Don't put the code above into the try block below. Exceptions
	    // in the code above are considered fatal, and must propagate to
	    // the caller of this operation.
	    //
	    
	    try
	    {
		try
		{
		    if(servantManager != null)
		    {
			servant_ = servantManager.findServant(current_.id, current_.facet);
			if(servant_ == null)
			{
			    locator_ = servantManager.findServantLocator(current_.id.category);
			    if(locator_ == null && current_.id.category.Length > 0)
			    {
				locator_ = servantManager.findServantLocator("");
			    }
			    if(locator_ != null)
			    {
				servant_ = locator_.locate(current_, out cookie_);
			    }
			}
		    }
		    if(servant_ == null)
		    {
			if(servantManager != null && servantManager.hasServant(current_.id))
			{
			    status = DispatchStatus.DispatchFacetNotExist;
			}
			else
			{
			    status = DispatchStatus.DispatchObjectNotExist;
			}
		    }
		    else
		    {
			status = servant_.dispatch__(this, current_);
		    }		
		}
		finally
		{
		    if(locator_ != null && servant_ != null && status != DispatchStatus.DispatchAsync)
		    {
			locator_.finished(current_, servant_, cookie_);
		    }
		}
	    }
	    catch(Ice.RequestFailedException ex)
	    {
	        _is.endReadEncaps();

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
		
		if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
										"Ice.Warn.Dispatch", 1) > 1)
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
                        string[] facetPath2 = { ex.facet };
                        os_.writeStringSeq(facetPath2);
                    }

		    os_.writeString(ex.operation);

		    connection_.sendResponse(os_, compress_);
		}
		else
		{
		    connection_.sendNoResponse();
		}

		return;
	    }
	    catch(Ice.UnknownLocalException ex)
	    {
	        _is.endReadEncaps();
		
		if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
										"Ice.Warn.Dispatch", 1) > 0)
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

		return;
	    }
	    catch(Ice.UnknownUserException ex)
	    {
	        _is.endReadEncaps();
		
		if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
										"Ice.Warn.Dispatch", 1) > 0)
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

		return;
	    }
	    catch(Ice.UnknownException ex)
	    {
	        _is.endReadEncaps();
		
		if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
										"Ice.Warn.Dispatch", 1) > 0)
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

		return;
	    }
	    catch(Ice.LocalException ex)
	    {
	        _is.endReadEncaps();
		
		if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
										"Ice.Warn.Dispatch", 1) > 0)
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

		return;
	    }

	    catch(Ice.UserException ex)
	    {
	        _is.endReadEncaps();
		
		if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
										"Ice.Warn.Dispatch", 1) > 0)
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

		return;
	    }

	    catch(System.Exception ex)
	    {
		_is.endReadEncaps();

		if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
										"Ice.Warn.Dispatch", 1) > 0)
		{
		    warning__(ex);
		}
		
		if(response_)
		{
		    os_.endWriteEncaps();
		    os_.resize(Protocol.headerSize + 4, false); // Dispatch status position.
		    os_.writeByte((byte) DispatchStatus.DispatchUnknownException);
		    os_.writeString(ex.ToString());
		    connection_.sendResponse(os_, compress_);
		}
		else
		{
		    connection_.sendNoResponse();
		}

		return;
	    }
	    
	    //
	    // Don't put the code below into the try block above. Exceptions
	    // in the code below are considered fatal, and must propagate to
	    // the caller of this operation.
	    //

	    _is.endReadEncaps();
	    
	    //
	    // DispatchAsync is "pseudo dispatch status", used internally
	    // only to indicate async dispatch.
	    //
	    if(status == DispatchStatus.DispatchAsync)
	    {
		//
		// If this was an asynchronous dispatch, we're done
		// here.
		//
		return;
	    }
	    
	    if(response_)
	    {
		os_.endWriteEncaps();
		
		if(status != DispatchStatus.DispatchOK && status != DispatchStatus.DispatchUserException)
		{
		    Debug.Assert(status == DispatchStatus.DispatchObjectNotExist ||
				 status == DispatchStatus.DispatchFacetNotExist ||
				 status == DispatchStatus.DispatchOperationNotExist);
		    
		    os_.resize(Protocol.headerSize + 4, false); // Dispatch status position.
		    os_.writeByte((byte)status);
		    
		    current_.id.write__(os_);

                    //
                    // For compatibility with the old FacetPath.
                    //
                    if(current_.facet == null || current_.facet.Length == 0)
                    {
                        os_.writeStringSeq(null);
                    }
                    else
                    {
                        string[] facetPath2 = { current_.facet };
                        os_.writeStringSeq(facetPath2);
                    }

		    os_.writeString(current_.operation);
		}
		else
		{
		    int save = os_.pos();
		    os_.pos(Protocol.headerSize + 4); // Dispatch status position.
		    os_.writeByte((byte)status);
		    os_.pos(save);
		}

		connection_.sendResponse(os_, compress_);
	    }
	    else
	    {
		connection_.sendNoResponse();
	    }
	}
	
	public BasicStream istr()
	{
	    return _is;
	}
	
	public BasicStream ostr()
	{
	    return os_;
	}
	
	public Incoming next; // For use by Connection.
	
	private BasicStream _is;
    }

}

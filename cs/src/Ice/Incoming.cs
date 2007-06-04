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

        protected internal void handleException__(System.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.RequestFailedException ex)
            {
                if(ex.id == null)
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
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    if(ex is Ice.ObjectNotExistException)
                    {
                        os_.writeByte(ReplyStatus.replyObjectNotExist);
                    }
                    else if(ex is Ice.FacetNotExistException)
                    {
                        os_.writeByte(ReplyStatus.replyFacetNotExist);
                    }
                    else if(ex is Ice.OperationNotExistException)
                    {
                        os_.writeByte(ReplyStatus.replyOperationNotExist);
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
                if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
                                                                                "Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }
                
                if(response_)
                {
                    os_.endWriteEncaps();
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownLocalException);
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
                if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
                                                                                "Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }
                
                if(response_)
                {
                    os_.endWriteEncaps();
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownUserException);
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
                if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
                                                                                "Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }
                
                if(response_)
                {
                    os_.endWriteEncaps();
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownException);
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
                if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
                                                                                "Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }
                
                if(response_)
                {
                    os_.endWriteEncaps();
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownLocalException);
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
                if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
                                                                                "Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }
                
                if(response_)
                {
                    os_.endWriteEncaps();
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownUserException);
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
                if(os_.instance().initializationData().properties.getPropertyAsIntWithDefault(
                                                                                "Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }
                
                if(response_)
                {
                    os_.endWriteEncaps();
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownException);
                    os_.writeString(ex.ToString());
                    connection_.sendResponse(os_, compress_);
                }
                else
                {
                    connection_.sendNoResponse();
                }

                return;
            }
        }
        
        protected internal Ice.Current current_;
        protected internal Ice.Object servant_;
        protected internal Ice.ServantLocator locator_;
        protected internal System.Object cookie_;
        
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
                Debug.Assert(os_.size() == Protocol.headerSize + 4); // Reply status position.
                os_.writeByte((byte)0);
                os_.startWriteEncaps();
            }
            
            byte replyStatus = ReplyStatus.replyOK;
            Ice.DispatchStatus dispatchStatus = Ice.DispatchStatus.DispatchOK;
            
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
                            replyStatus = ReplyStatus.replyFacetNotExist;
                        }
                        else
                        {
                            replyStatus = ReplyStatus.replyObjectNotExist;
                        }
                    }
                    else
                    {
                        dispatchStatus = servant_.dispatch__(this, current_);
                        if(dispatchStatus == Ice.DispatchStatus.DispatchUserException)
                        {
                            replyStatus = ReplyStatus.replyUserException;
                        }
                    }           
                }
                finally
                {
                    if(locator_ != null && servant_ != null && dispatchStatus != Ice.DispatchStatus.DispatchAsync)
                    {
                        locator_.finished(current_, servant_, cookie_);
                    }
                }
            }
            catch(System.Exception ex)
            {
                _is.endReadEncaps();
                handleException__(ex);
                return;
            }
            
            //
            // Don't put the code below into the try block above. Exceptions
            // in the code below are considered fatal, and must propagate to
            // the caller of this operation.
            //

            _is.endReadEncaps();
            
            //
            // Async dispatch
            //
            if(dispatchStatus == Ice.DispatchStatus.DispatchAsync)
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
                
                if(replyStatus != ReplyStatus.replyOK && replyStatus != ReplyStatus.replyUserException)
                {
                    Debug.Assert(replyStatus == ReplyStatus.replyObjectNotExist ||
                                 replyStatus == ReplyStatus.replyFacetNotExist);
                    
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(replyStatus);
                    
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
                    os_.pos(Protocol.headerSize + 4); // Reply status position.
                    os_.writeByte(replyStatus);
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

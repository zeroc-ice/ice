// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Globalization;

    public class IncomingBase
    {
        protected internal IncomingBase(Instance instance, ResponseHandler handler, Ice.ConnectionI connection,
                                        Ice.ObjectAdapter adapter, bool response, byte compress, int requestId)
        {
            instance_ = instance;
            responseHandler_ = handler;
            response_ = response;
            compress_ = compress;
            if(response_)
            {
                os_ = new BasicStream(instance, Ice.Util.currentProtocolEncoding);
            }

            current_ = new Ice.Current();
            current_.id = new Ice.Identity();
            current_.adapter = adapter;
            current_.con = connection;
            current_.requestId = requestId;

            cookie_ = null;
        }

        protected internal IncomingBase(IncomingBase inc) // Adopts the argument. It must not be used afterwards.
        {
            //
            // We don't change current_ as it's exposed by Ice::Request.
            //
            current_ = inc.current_;

            //
            // Deep copy
            //
            if(inc.interceptorAsyncCallbackList_ != null)
            {
                //
                // Copy, not just reference
                //
                interceptorAsyncCallbackList_ =
                    new List<Ice.DispatchInterceptorAsyncCallback>(inc.interceptorAsyncCallbackList_);
            }

            adopt(inc);
        }

        internal void
        adopt(IncomingBase inc)
        {
            instance_ = inc.instance_;
            //inc.instance_ = null; // Don't reset instance_.

            observer_ = inc.observer_;
            inc.observer_ = null;

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

            //
            // Adopt the stream - it creates less garbage.
            //
            os_ = inc.os_;
            inc.os_ = null;

            responseHandler_ = inc.responseHandler_;
            inc.responseHandler_ = null;
        }

        public BasicStream startWriteParams__(Ice.FormatType format)
        {
            if(!response_)
            {
                throw new Ice.MarshalException("can't marshal out parameters for oneway dispatch");
            }

            Debug.Assert(os_.size() == Protocol.headerSize + 4); // Reply status position.
            os_.writeByte((byte)0);
            os_.startWriteEncaps(current_.encoding, format);
            return os_;
        }

        public void endWriteParams__(bool ok)
        {
            if(!ok && observer_ != null)
            {
                observer_.userException();
            }

            if(response_)
            {
                int save = os_.pos();
                os_.pos(Protocol.headerSize + 4); // Reply status position.
                os_.writeByte(ok ? ReplyStatus.replyOK : ReplyStatus.replyUserException);
                os_.pos(save);
                os_.endWriteEncaps();
            }
        }

        public void writeEmptyParams__()
        {
            if(response_)
            {
                Debug.Assert(os_.size() == Protocol.headerSize + 4); // Reply status position.
                os_.writeByte(ReplyStatus.replyOK);
                os_.writeEmptyEncaps(current_.encoding);
            }
        }

        public void writeParamEncaps__(byte[] v, bool ok)
        {
            if(!ok && observer_ != null)
            {
                observer_.userException();
            }

            if(response_)
            {
                Debug.Assert(os_.size() == Protocol.headerSize + 4); // Reply status position.
                os_.writeByte(ok ? ReplyStatus.replyOK : ReplyStatus.replyUserException);
                if(v == null || v.Length == 0)
                {
                    os_.writeEmptyEncaps(current_.encoding);
                }
                else
                {
                    os_.writeEncaps(v);
                }
            }
        }

        public void writeUserException__(Ice.UserException ex, Ice.FormatType format)
        {
            BasicStream os__ = startWriteParams__(format);
            os__.writeUserException(ex);
            endWriteParams__(false);
        }

        //
        // These functions allow this object to be reused, rather than reallocated.
        //
        public virtual void reset(Instance instance, ResponseHandler handler, Ice.ConnectionI connection,
                                  Ice.ObjectAdapter adapter, bool response, byte compress, int requestId)
        {
            instance_ = instance;

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

            if(response_ && os_ == null)
            {
                os_ = new BasicStream(instance, Ice.Util.currentProtocolEncoding);
            }

            responseHandler_ = handler;
            interceptorAsyncCallbackList_ = null;
        }

        public virtual void reclaim()
        {
            servant_ = null;

            locator_ = null;

            cookie_ = null;

            observer_ = null;

            if(os_ != null)
            {
                os_.reset();
            }

            interceptorAsyncCallbackList_ = null;
        }

        protected internal void warning__(System.Exception ex)
        {
            Debug.Assert(instance_ != null);

            using(StringWriter sw = new StringWriter(CultureInfo.CurrentCulture))
            {
                IceUtilInternal.OutputBase output = new IceUtilInternal.OutputBase(sw);
                output.setUseTab(false);
                output.print("dispatch exception:");
                output.print("\nidentity: " + instance_.identityToString(current_.id));
                output.print("\nfacet: " + IceUtilInternal.StringUtil.escapeString(current_.facet, ""));
                output.print("\noperation: " + current_.operation);
                if(current_.con != null)
                {
                    try
                    {
                        Ice.ConnectionInfo connInfo = current_.con.getInfo();
                        if(connInfo is Ice.IPConnectionInfo)
                        {
                            Ice.IPConnectionInfo ipConnInfo = (Ice.IPConnectionInfo)connInfo;
                            output.print("\nremote host: " + ipConnInfo.remoteAddress + " remote port: " +
                                         ipConnInfo.remotePort.ToString());
                        }
                    }
                    catch(Ice.LocalException)
                    {
                        // Ignore.
                    }
                }
                output.print("\n");
                output.print(ex.ToString());
                instance_.initializationData().logger.warning(sw.ToString());
            }
        }

        protected bool servantLocatorFinished__(bool amd)
        {
            Debug.Assert(locator_ != null && servant_ != null);
            try
            {
                locator_.finished(current_, servant_, cookie_);
                return true;
            }
            catch(Ice.UserException ex)
            {
                Debug.Assert(responseHandler_ != null);

                if(observer_ != null)
                {
                    observer_.userException();
                }

                //
                // The operation may have already marshaled a reply; we must overwrite that reply.
                //
                if(response_)
                {
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUserException);
                    os_.startWriteEncaps(current_.encoding, Ice.FormatType.DefaultFormat);
                    os_.writeUserException(ex);
                    os_.endWriteEncaps();
                    if(observer_ != null)
                    {
                        observer_.reply(os_.size() - Protocol.headerSize - 4);
                    }
                    responseHandler_.sendResponse(current_.requestId, os_, compress_, amd);
                }
                else
                {
                    responseHandler_.sendNoResponse();
                }

                if(observer_ != null)
                {
                    observer_.detach();
                    observer_ = null;
                }
                responseHandler_ = null;
            }
            catch(System.Exception ex)
            {
                handleException__(ex, amd);
            }
            return false;
        }

        protected internal void handleException__(System.Exception exc, bool amd)
        {
            Debug.Assert(responseHandler_ != null);

            try
            {
                throw exc;
            }
            catch(Ice.RequestFailedException ex)
            {
                if(ex.id == null || ex.id.name == null || ex.id.name.Length == 0)
                {
                    ex.id = current_.id;
                }

                if(ex.facet == null || ex.facet.Length == 0)
                {
                    ex.facet = current_.facet;
                }

                if(ex.operation == null || ex.operation.Length == 0)
                {
                    ex.operation = current_.operation;
                }

                if(instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                {
                    warning__(ex);
                }

                if(observer_ != null)
                {
                    observer_.failed(ex.ice_name());
                }

                if(response_)
                {
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

                    if(observer_ != null)
                    {
                        observer_.reply(os_.size() - Protocol.headerSize - 4);
                    }
                    responseHandler_.sendResponse(current_.requestId, os_, compress_, amd);
                }
                else
                {
                    responseHandler_.sendNoResponse();
                }
            }
            catch(Ice.UnknownLocalException ex)
            {
                if(instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }

                if(observer_ != null)
                {
                    observer_.failed(ex.ice_name());
                }

                if(response_)
                {
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownLocalException);
                    os_.writeString(ex.unknown);
                    if(observer_ != null)
                    {
                        observer_.reply(os_.size() - Protocol.headerSize - 4);
                    }
                    responseHandler_.sendResponse(current_.requestId, os_, compress_, amd);
                }
                else
                {
                    responseHandler_.sendNoResponse();
                }
            }
            catch(Ice.UnknownUserException ex)
            {
                if(instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }

                if(observer_ != null)
                {
                    observer_.failed(ex.ice_name());
                }

                if(response_)
                {
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownUserException);
                    os_.writeString(ex.unknown);
                    if(observer_ != null)
                    {
                        observer_.reply(os_.size() - Protocol.headerSize - 4);
                    }
                    responseHandler_.sendResponse(current_.requestId, os_, compress_, amd);
                }
                else
                {
                    responseHandler_.sendNoResponse();
                }
            }
            catch(Ice.UnknownException ex)
            {
                if(instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }

                if(observer_ != null)
                {
                    observer_.failed(ex.ice_name());
                }

                if(response_)
                {
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownException);
                    os_.writeString(ex.unknown);
                    if(observer_ != null)
                    {
                        observer_.reply(os_.size() - Protocol.headerSize - 4);
                    }
                    responseHandler_.sendResponse(current_.requestId, os_, compress_, amd);
                }
                else
                {
                    responseHandler_.sendNoResponse();
                }
            }
            catch(Ice.UserException ex)
            {
                if(instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }

                if(observer_ != null)
                {
                    observer_.failed(ex.ice_name());
                }

                if(response_)
                {
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownUserException);
                    os_.writeString(ex.ice_name() + "\n" + ex.StackTrace);
                    if(observer_ != null)
                    {
                        observer_.reply(os_.size() - Protocol.headerSize - 4);
                    }
                    responseHandler_.sendResponse(current_.requestId, os_, compress_, amd);
                }
                else
                {
                    responseHandler_.sendNoResponse();
                }
            }
            catch(Ice.Exception ex)
            {
                if(ex is Ice.SystemException)
                {
                    if(responseHandler_.systemException(current_.requestId, (Ice.SystemException)ex, amd))
                    {
                        return;
                    }
                }

                if(instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }

                if(observer_ != null)
                {
                    observer_.failed(ex.ice_name());
                }

                if(response_)
                {
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownLocalException);
                    os_.writeString(ex.ice_name() + "\n" + ex.StackTrace);
                    if(observer_ != null)
                    {
                        observer_.reply(os_.size() - Protocol.headerSize - 4);
                    }
                    responseHandler_.sendResponse(current_.requestId, os_, compress_, amd);
                }
                else
                {
                    responseHandler_.sendNoResponse();
                }
            }
            catch(System.Exception ex)
            {
                if(instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }

                if(observer_ != null)
                {
                    observer_.failed(ex.GetType().FullName);
                }

                if(response_)
                {
                    os_.resize(Protocol.headerSize + 4, false); // Reply status position.
                    os_.writeByte(ReplyStatus.replyUnknownException);
                    os_.writeString(ex.ToString());
                    if(observer_ != null)
                    {
                        observer_.reply(os_.size() - Protocol.headerSize - 4);
                    }
                    responseHandler_.sendResponse(current_.requestId, os_, compress_, amd);
                }
                else
                {
                    responseHandler_.sendNoResponse();
                }
            }

            if(observer_ != null)
            {
                observer_.detach();
                observer_ = null;
            }
            responseHandler_ = null;
        }

        protected internal Instance instance_;
        protected internal Ice.Current current_;
        protected internal Ice.Object servant_;
        protected internal Ice.ServantLocator locator_;
        protected internal System.Object cookie_;
        protected internal Ice.Instrumentation.DispatchObserver observer_;

        protected internal bool response_;
        protected internal byte compress_;

        protected internal BasicStream os_;

        protected ResponseHandler responseHandler_;

        protected List<Ice.DispatchInterceptorAsyncCallback> interceptorAsyncCallbackList_;
    }

    sealed public class Incoming : IncomingBase, Ice.Request
    {
        public Incoming(Instance instance, ResponseHandler handler, Ice.ConnectionI connection,
                        Ice.ObjectAdapter adapter, bool response, byte compress, int requestId)
            : base(instance, handler, connection, adapter, response, compress, requestId)
        {
            //
            // Prepare the response if necessary.
            //
            if(response)
            {
                os_.writeBlob(IceInternal.Protocol.replyHdr);

                //
                // Add the request ID.
                //
                os_.writeInt(requestId);
            }
        }

        public Ice.Current
        getCurrent()
        {
            return current_;
        }

        //
        // These functions allow this object to be reused, rather than reallocated.
        //
        public override void reset(Instance instance, ResponseHandler handler, Ice.ConnectionI connection,
                                   Ice.ObjectAdapter adapter, bool response, byte compress, int requestId)
        {
            _cb = null;
            _inParamPos = -1;

            base.reset(instance, handler, connection, adapter, response, compress, requestId);

            //
            // Prepare the response if necessary.
            //
            if(response)
            {
                os_.writeBlob(IceInternal.Protocol.replyHdr);

                //
                // Add the request ID.
                //
                os_.writeInt(requestId);
            }
        }

        public override void reclaim()
        {
            _cb = null;
            _inParamPos = -1;

            base.reclaim();
        }

        public void invoke(ServantManager servantManager, BasicStream stream)
        {
            _is = stream;

            int start = _is.pos();

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
            current_.ctx = new Dictionary<string, string>();
            int sz = _is.readSize();
            while(sz-- > 0)
            {
                string first = _is.readString();
                string second = _is.readString();
                current_.ctx[first] = second;
            }

            Ice.Instrumentation.CommunicatorObserver obsv = instance_.initializationData().observer;
            if(obsv != null)
            {
                // Read the encapsulation size.
                int size = _is.readInt();
                _is.pos(_is.pos() - 4);

                observer_ = obsv.getDispatchObserver(current_, _is.pos() - start + size);
                if(observer_ != null)
                {
                    observer_.attach();
                }
            }

            //
            // Don't put the code above into the try block below. Exceptions
            // in the code above are considered fatal, and must propagate to
            // the caller of this operation.
            //

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
                        try
                        {
                            servant_ = locator_.locate(current_, out cookie_);
                        }
                        catch(Ice.UserException ex)
                        {
                            Ice.EncodingVersion encoding = _is.skipEncaps(); // Required for batch requests.

                            if(observer_ != null)
                            {
                                observer_.userException();
                            }

                            if(response_)
                            {
                                os_.writeByte(ReplyStatus.replyUserException);
                                os_.startWriteEncaps(encoding, Ice.FormatType.DefaultFormat);
                                os_.writeUserException(ex);
                                os_.endWriteEncaps();
                                if(observer_ != null)
                                {
                                    observer_.reply(os_.size() - Protocol.headerSize - 4);
                                }
                                responseHandler_.sendResponse(current_.requestId, os_, compress_, false);
                            }
                            else
                            {
                                responseHandler_.sendNoResponse();
                            }

                            if(observer_ != null)
                            {
                                observer_.detach();
                                observer_ = null;
                            }
                            responseHandler_ = null;
                            return;
                        }
                        catch(System.Exception ex)
                        {
                            _is.skipEncaps(); // Required for batch requests.
                            handleException__(ex, false);
                            return;
                        }
                    }
                }
            }

            try
            {
                if(servant_ != null)
                {
                    //
                    // DispatchAsync is a "pseudo dispatch status", used internally only
                    // to indicate async dispatch.
                    //
                    if(servant_.dispatch__(this, current_) == Ice.DispatchStatus.DispatchAsync)
                    {
                        //
                        // If this was an asynchronous dispatch, we're done here.
                        //
                        return;
                    }

                    if(locator_ != null && !servantLocatorFinished__(false))
                    {
                        return;
                    }
                }
                else
                {
                    //
                    // Skip the input parameters, this is required for reading
                    // the next batch request if dispatching batch requests.
                    //
                    _is.skipEncaps();

                    if(servantManager != null && servantManager.hasServant(current_.id))
                    {
                        throw new Ice.FacetNotExistException(current_.id, current_.facet, current_.operation);
                    }
                    else
                    {
                        throw new Ice.ObjectNotExistException(current_.id, current_.facet, current_.operation);
                    }
                }
            }
            catch(System.Exception ex)
            {
                if(servant_ != null && locator_ != null && !servantLocatorFinished__(false))
                {
                    return;
                }
                handleException__(ex, false);
                return;
            }

            //
            // Don't put the code below into the try block above. Exceptions
            // in the code below are considered fatal, and must propagate to
            // the caller of this operation.
            //

            Debug.Assert(responseHandler_ != null);

            if(response_)
            {
                if(observer_ != null)
                {
                    observer_.reply(os_.size() - Protocol.headerSize - 4);
                }
                responseHandler_.sendResponse(current_.requestId, os_, compress_, false);
            }
            else
            {
                responseHandler_.sendNoResponse();
            }

            if(observer_ != null)
            {
                observer_.detach();
                observer_ = null;
            }
            responseHandler_ = null;
        }

        public void push(Ice.DispatchInterceptorAsyncCallback cb)
        {
            if(interceptorAsyncCallbackList_ == null)
            {
                interceptorAsyncCallbackList_ = new List<Ice.DispatchInterceptorAsyncCallback>();
            }

            interceptorAsyncCallbackList_.Insert(0, cb);
        }

        public void pop()
        {
            Debug.Assert(interceptorAsyncCallbackList_ != null);
            interceptorAsyncCallbackList_.RemoveAt(0);
        }

        public void startOver()
        {
            if(_inParamPos == -1)
            {
                //
                // That's the first startOver, so almost nothing to do
                //
                _inParamPos = _is.pos();
            }
            else
            {
                killAsync();

                //
                // Let's rewind _is and clean-up os_
                //
                _is.pos(_inParamPos);
                if(response_)
                {
                    os_.resize(Protocol.headerSize + 4, false);
                }
            }
        }

        public void killAsync()
        {
            //
            // Always runs in the dispatch thread
            //
            if(_cb != null)
            {
                //
                // May raise ResponseSentException
                //
                _cb.deactivate__(this);
                _cb = null;
            }
        }

        public BasicStream startReadParams()
        {
            //
            // Remember the encoding used by the input parameters, we'll
            // encode the response parameters with the same encoding.
            //
            current_.encoding = _is.startReadEncaps();
            return _is;
        }

        public void endReadParams()
        {
            _is.endReadEncaps();
        }

        public void readEmptyParams()
        {
            current_.encoding = _is.skipEmptyEncaps();
        }

        public byte[] readParamEncaps()
        {
            return _is.readEncaps(out current_.encoding);
        }

        internal void setActive(IncomingAsync cb)
        {
            Debug.Assert(_cb == null);
            _cb = cb;
        }

        internal bool isRetriable()
        {
            return _inParamPos != -1;
        }

        public Incoming next; // For use by Connection.

        private BasicStream _is;

        private IncomingAsync _cb;
        private int _inParamPos = -1;
    }

}

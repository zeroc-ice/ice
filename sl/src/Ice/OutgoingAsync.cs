// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;
    using System.IO;
    using System.Net;

#if SILVERLIGHT
    using System.Windows.Browser.Net;
#else
    using System.Collections.Specialized;
#endif

    public abstract class OutgoingAsync : OutgoingBase
    {
        public void responseReady__(IAsyncResult iar)
        {
            lock(this)
            {
                try
                {
                    Debug.Assert(_response == null);
                    _response = (HttpWebResponse)((HttpWebRequest)iar.AsyncState).EndGetResponse(iar);
                }
                catch(WebException ex)
                {
                    // Error occurred in processing the response.
                    finished__(new Ice.SocketException(ex));
                    return;
                }

                if(_response.StatusCode != HttpStatusCode.OK)
                {
                    finished__(new Ice.ProtocolException("invalid http response code: " + _response.StatusCode));
                    return;
                }

                int sz = 12; // Reply header size
                is__.resize(sz, true);
                ByteBuffer buf = is__.prepareRead();
                int position = 0;
                is__.pos(position);

                try
                {
                    //
                    // Read the reply header.
                    //
                    Stream stream = _response.GetResponseStream();
                    readResponse__(stream, buf, sz, ref position);

                    //
                    // Determine size and read the rest of the reply.
                    //
                    int remaining = BitConverter.ToInt32(buf.toArray(8, 4), 0);
                    is__.resize(sz + remaining, true);
                    buf = is__.prepareRead();
                    readResponse__(stream, buf, remaining, ref position);

                    IceInternal.TraceUtil.traceReply("received asynchronous reply", is__, logger__, traceLevels__);
		    bool ok = handleResponse__();

		    try
		    {
			response__(ok);
		    }
		    catch(System.Exception sysex)
		    {
			warning(sysex);
		    }
		    finally
		    {
			cleanup();
		    }
		}
		catch(Ice.LocalException ex)
		{
		    finished__(ex);
		    return;
		}
                catch(Exception ex)
                {
                    finished__(new Ice.SocketException(ex));
                    return;
                }
            }
        }

        public void finished__(Ice.LocalException exc)
        {
            lock(this)
            {
                if(_response != null)
                {
                    _response.Close();
                    _response = null;
                }

                if(os__ != null) // Don't retry if cleanup() was already called.
                {
                    //
                    // An ObjectNotExistException can always be
                    // retried as well without violating
                    // "at-most-once".
                    //  
                    if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent ||
                       exc is Ice.ObjectNotExistException)
                    {
                        try
                        {
                            _cnt = ((Ice.ObjectPrxHelperBase)_proxy).handleException__(exc, _cnt);
                            send__();
                            return;
                        }
                        catch(Ice.LocalException)
                        {
                        }
                    }
                }

                try
                {
                    exception__(exc);
                }
                catch(System.Exception ex)
                {
                    warning(ex);
                }
                finally
                {
                    cleanup();
                }
            }
        }
        
        protected void prepare__(Ice.ObjectPrx prx, string operation, Ice.OperationMode mode,
                                 Dictionary<string, string> context)
        {
            lock(this)
            {
                //
                // Can't call sync via a oneway proxy.
                //
                ((Ice.ObjectPrxHelperBase)prx).checkTwowayOnly__(operation);

                Reference rf = ((Ice.ObjectPrxHelperBase)prx).reference__();
                logger__ = rf.getInstance().initializationData().logger;
                traceLevels__ = rf.getInstance().traceLevels();
                bridgeUri__ = rf.getInstance().bridgeUri();

                Debug.Assert(is__ == null);
                is__ = new BasicStream(rf.getInstance());
                Debug.Assert(os__ == null);
                os__ = new BasicStream(rf.getInstance());

                writeHeader__(rf, operation, mode, context);

                _proxy = prx;
                _cnt = 0;
                _mode = mode;

            }
        }
        
        protected void send__()
        {
            lock(this)
            {
                try
                {
                    while(true)
                    {
                        try
                        {
                            ByteBuffer buf = os__.prepareWrite();
#if SILVERLIGHT
                            BrowserHttpWebRequest req = new BrowserHttpWebRequest(bridgeUri__);
#else
                            HttpWebRequest req = (HttpWebRequest)WebRequest.Create(bridgeUri__);
#endif
                            req.Method = "PUT";
                            req.ContentLength = os__.prepareWrite().limit();
                            req.ContentType = "application/binary";

                            if(traceLevels__.network >= 2)
                            {
                                string str = "trying to bridge request with " + bridgeUri__.ToString();
                                logger__.trace(traceLevels__.networkCat, str);
                            }

                            IceInternal.TraceUtil.traceRequest("sending asynchronous request", os__, logger__,
                                                               traceLevels__);

                            Stream s = null;
                            try
                            {
                                s = req.GetRequestStream();
                                s.Write(buf.rawBytes(), 0, buf.limit());
                            }
                            catch(WebException ex)
                            {
                                finished__(new Ice.ConnectFailedException(ex));
                                return;
                            }
                            catch(IOException ex)
                            {
                                try
                                {
                                    _cnt = ((Ice.ObjectPrxHelperBase)_proxy).handleException__(
                                        new Ice.SocketException(ex), _cnt);
                                    //
                                    // If we reach here we should retry the send.
                                    //
                                    send__();
                                    return;
                                }
                                catch(Ice.LocalException e)
                                {
                                    finished__(e);
                                    return;
                                }
                            }
                            finally
                            {
#if !SILVERLIGHT
                                // The stream must be closed.
                                if(s != null)
                                {
                                    s.Close();
                                    s = null;
                                }
#endif
                            }

                            if(traceLevels__.network >= 1)
                            {
                                string str = "bridging request with " + bridgeUri__.ToString();
                                logger__.trace(traceLevels__.networkCat, str);
                                if(traceLevels__.network >= 3)
                                {
                                    str = "sent " + buf.limit() + " of " + buf.limit() + " bytes";
                                    logger__.trace(traceLevels__.networkCat, str);
                                }
                            }

                            try
                            {
                                req.BeginGetResponse(new AsyncCallback(responseReady__), req);
                            }
                            catch(WebException ex)
                            {
                                //
                                // Error occurred in processing the response. The
                                // request cannot be retried.
                                //
                                finished__(new Ice.SocketException(ex));
                            }
                            return;
                        }
                        catch(Ice.LocalException ex)
                        {
                            _cnt = ((Ice.ObjectPrxHelperBase)_proxy).handleException__(ex, _cnt);
                        }
                    }
                }
                catch(Ice.LocalException ex)
                {
                    finished__(ex);
                }
            }
        }

        protected abstract void response__(bool ok);
        protected abstract void exception__(Ice.Exception ex);

        private void warning(System.Exception ex)
        {
            if(os__ != null) // Don't print anything if cleanup() was already called.
            {
                Reference rf = ((Ice.ObjectPrxHelperBase)_proxy).reference__();
                if(rf.getInstance().initializationData().properties.getPropertyAsIntWithDefault(
                                                                                "Ice.Warn.AMICallback", 1) > 0)
                {
                    rf.getInstance().initializationData().logger.warning("exception raised by AMI callback:\n" + ex);
                }
            }
        }

        private void cleanup()
        {
            if(_response != null)
            {
                _response.Close();
                _response = null;
            }

            is__ = null;
            os__ = null;
        }
        
        private Ice.ObjectPrx _proxy;
        private int _cnt;
        private Ice.OperationMode _mode;
        private HttpWebResponse _response = null;
    }
}

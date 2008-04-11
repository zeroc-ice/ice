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
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Net;
    using System;

#if SILVERLIGHT
    using System.Windows.Browser.Net;
#else
    using System.Collections.Specialized;
#endif

    public class Outgoing : OutgoingBase
    {
        public Outgoing(Reference r, string operation, Ice.OperationMode mode,
                        Dictionary<string, string> context)
        {
            _oneway = r.getMode() == Reference.Mode.ModeOneway;

            bridgeUri__ = r.getInstance().bridgeUri();
            logger__ = r.getInstance().initializationData().logger;
            traceLevels__ = r.getInstance().traceLevels();
            is__ = new BasicStream(r.getInstance());
            os__ = new BasicStream(r.getInstance());
            writeHeader__(r, operation, mode, context);
        }

        // Returns true if ok, false if user exception.
        public bool invoke()
        {
            os__.endWriteEncaps();

            ByteBuffer buf = os__.prepareWrite();

#if SILVERLIGHT
            BrowserHttpWebRequest req = new BrowserHttpWebRequest(bridgeUri__);
#else
            HttpWebRequest req = (HttpWebRequest)WebRequest.Create(bridgeUri__);
#endif
            req.Method = "PUT";
            req.ContentLength = buf.limit();
            req.ContentType = "application/binary";

            if(traceLevels__.network >= 2)
            {
                string str = "trying to bridge request with " + bridgeUri__.ToString();
                logger__.trace(traceLevels__.networkCat, str);
            }

            IceInternal.TraceUtil.traceRequest("sending request", os__, logger__, traceLevels__);
            Stream s = null;
            // XXX: Either get rid of Stats or fix this.
            try
            {
                s = req.GetRequestStream();
                s.Write(buf.rawBytes(), 0, buf.limit());
		//IceInternal.TraceUtil.dumpStream(os__);
            }
            catch(WebException ex)
            {
                throw new Ice.ConnectFailedException(ex);
            }
            catch(IOException ex)
            {
                throw new Ice.SocketException(ex);
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

            // If the request is oneway then read the http reply
            // asynchronously.
            if(_oneway)
            {
                try
                {
                    req.BeginGetResponse(new AsyncCallback(responseReady), req);
                }
                catch(WebException ex)
                {
                    // Error occurred in processing the response. The
                    // request cannot be retried.
                    throw new LocalExceptionWrapper(new Ice.SocketException(ex), false);
                }
                return true;
            }

            HttpWebResponse response;
            try
            {
                response = (HttpWebResponse)req.GetResponse();
            }
            catch(WebException ex)
            {
                // Error occurred in processing the response. The
                // request cannot be retried.
                throw new LocalExceptionWrapper(new Ice.SocketException(ex), false);
            }

            try
            {
                if(response.StatusCode != HttpStatusCode.OK)
                {
                    throw new Ice.ProtocolException("invalid http response code: " + response.StatusCode);
                }

                int sz = 12; // Reply header size
                is__.resize(sz, true);
                buf = is__.prepareRead();    
                int position = 0;
                is__.pos(position);

                try
                {
                    //
                    // Read the reply header.
                    //
                    s = response.GetResponseStream();
                    readResponse__(s, buf, sz, ref position);

                    //
                    // Determine size and read the rest of the reply.
                    //
                    int remaining = BitConverter.ToInt32(buf.toArray(8, 4), 0);
                    is__.resize(sz + remaining, true);
                    buf = is__.prepareRead();
                    readResponse__(s, buf, remaining, ref position);
                }
                catch(IOException ex)
                {
                    throw new Ice.SocketException(ex);
                }

                IceInternal.TraceUtil.traceReply("received reply", is__, logger__, traceLevels__);
		//IceInternal.TraceUtil.dumpStream(is__);
                return handleResponse__();
            }
            //      
            // A CloseConnectionException indicates graceful
            // server shutdown, and is therefore always repeatable
            // without violating "at-most-once". That's because by
            // sending a close connection message, the server
            // guarantees that all outstanding requests can safely
            // be repeated.
            //
            // This cannot occur with Ice for SL.
            //
            //catch(Ice.CloseConnectionException ex)
            //{
            //    throw;
            //}
            //
            // An ObjectNotExistException can always be retried as
            // well without violating "at-most-once".
            //
            catch(Ice.ObjectNotExistException)
            {
                throw;
            }
            //
            // Throw the exception wrapped in a LocalExceptionWrapper, to
            // indicate that the request cannot be resent without
            // potentially violating the "at-most-once" principle.
            //
            catch(Ice.LocalException ex)
            {
                throw new LocalExceptionWrapper(ex, false);
            }
            finally
            {
                // Its not necessary to close both the stream and the response.
                //s.Close();
                response.Close();
            }
        }

        // Process and ignore the response to a oneway invocation.
        public void responseReady(IAsyncResult iar)
        {
            try
            {
                HttpWebResponse response = (HttpWebResponse)((HttpWebRequest)iar.AsyncState).EndGetResponse(iar);
                response.Close();
            }
            catch(WebException)
            {
            }
        }
        
        public BasicStream istr()
        {
            return is__;
        }
        
        public BasicStream ostr()
        {
            return os__;
        }

        private bool _oneway;
    }
}

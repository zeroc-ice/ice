// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

namespace IceInternal
{

    public abstract class EventHandler
    {
        //
        // Return true if the handler is for a datagram transport, false otherwise.
        //
        abstract public bool datagram();
        
        //
        // Return true if read() must be called before calling message().
        //
        abstract public bool readable();
        
        //
        // Read data via the event handler. May only be called if
        // readable() returns true.
        //
        abstract public void read(BasicStream istr);
        
        //
        // A complete message has been received.
        //
        abstract public void message(BasicStream stream, ThreadPool threadPool);
        
        //
        // Will be called if the event handler is finally
        // unregistered. (Calling unregister() does not unregister
        // immediately.)
        //
        abstract public void finished(ThreadPool threadPool);
        
        //
        // Propagate an exception to the event handler.
        //
        abstract public void exception(Ice.LocalException ex);
        
        //
        // Get a textual representation of the event handler.
        //
        public abstract override string ToString();
        
        public Instance instance()
        {
            return instance_;
        }

        protected internal EventHandler(Instance instance)
        {
            instance_ = instance;
            stream_ = new BasicStream(instance);
        }

        protected internal Instance instance_;
        
        //
        // The stream_ data member is for use by ThreadPool only.
        //
        internal BasicStream stream_;
    }

}

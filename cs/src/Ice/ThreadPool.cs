// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// Uncomment these definitions as needed. (We use #define here instead
// of constants to stop the compiler from complaining about unreachable code.)
//
//#define TRACE_REGISTRATION
//#define TRACE_INTERRUPT
//#define TRACE_SHUTDOWN
//#define TRACE_SELECT
//#define TRACE_EXCEPTION
//#define TRACE_THREAD
//#define TRACE_STACK_TRACE

namespace IceInternal
{

    using System;
    using System.Collections;
    using System.Diagnostics;
    using System.Net.Sockets;
    using System.Threading;
    using IceUtil;

    public sealed class ThreadPool
    {  
        public ThreadPool(Instance instance, string prefix, int timeout)
        {
            instance_ = instance;
            _destroyed = false;
            _prefix = prefix;
            _timeout = timeout;
            size_ = 0;
            sizeMax_ = 0;
            sizeWarn_ = 0;
            threadIndex_ = 0;
            running_ = 0;
            inUse_ = 0;
            load_ = 1.0;
            promote_ = true;
            warnUdp_ = instance_.initializationData().properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;

            string programName = instance_.initializationData().properties.getProperty("Ice.ProgramName");
            if(programName.Length > 0)
            {
                _programNamePrefix = programName + "-";
            }
            else
            {
                _programNamePrefix = "";
            }
            
            Network.SocketPair pair = Network.createPipe();
            _fdIntrRead = pair.source;
            _fdIntrWrite = pair.sink;
            Network.setBlock(_fdIntrRead, false);
            
            //
            // We use just one thread as the default. This is the fastest
            // possible setting, still allows one level of nesting, and
            // doesn't require to make the servants thread safe.
            //
            int size = instance_.initializationData().properties.getPropertyAsIntWithDefault(_prefix + ".Size", 1);
            if(size < 1)
            {
                string s = _prefix + ".Size < 1; Size adjusted to 1";
                instance_.initializationData().logger.warning(s);
                size = 1;
            }
            
            int sizeMax = 
                instance_.initializationData().properties.getPropertyAsIntWithDefault(_prefix + ".SizeMax", size);
            if(sizeMax < size)
            {
                string s = _prefix + ".SizeMax < " + _prefix + ".Size; SizeMax adjusted to Size (" + size + ")";
                instance_.initializationData().logger.warning(s);
                sizeMax = size;
            }
            
            int sizeWarn = instance_.initializationData().properties.getPropertyAsIntWithDefault(_prefix + ".SizeWarn",
                                                                              sizeMax * 80 / 100);
            if(sizeWarn > sizeMax)
            {
                string s = _prefix + ".SizeWarn > " + _prefix + ".SizeMax; adjusted SizeWarn to SizeMax ("
                    + sizeMax + ")";
                instance_.initializationData().logger.warning(s);
                sizeWarn = sizeMax;
            }

            size_ = size;
            sizeMax_ = sizeMax;
            sizeWarn_ = sizeWarn;
            
            try
            {
                threads_ = new ArrayList();
                for(int i = 0; i < size_; ++i)
                {
                    EventHandlerThread thread = new EventHandlerThread(this, _programNamePrefix + _prefix + "-" +
                                                                       threadIndex_++);
                    threads_.Add(thread);
                    thread.Start();
                    ++running_;
                }
            }
            catch(System.Exception ex)
            {
                string s = "cannot create thread for `" + _prefix + "':\n" + ex;
                instance_.initializationData().logger.error(s);
                
                destroy();
                joinWithAllThreads();
                throw;
            }
        }
        
        public void destroy()
        {
            lock(this)
            {
                #if TRACE_SHUTDOWN
                    trace("destroy");
                #endif
                
                Debug.Assert(!_destroyed);
                Debug.Assert(_handlerMap.Count == 0);
                Debug.Assert(_changes.Count == 0);
                _destroyed = true;
                setInterrupt();
            }
        }
        
        public void register(Socket fd, EventHandler handler)
        {
            lock(this)
            {
                #if TRACE_REGISTRATION
                    trace("adding handler of type " + handler.GetType().FullName + " for channel " + fd.Handle);
                #endif
                Debug.Assert(!_destroyed);
                _changes.Add(new FdHandlerPair(fd, handler));
                setInterrupt();
            }
        }
        
        public void unregister(Socket fd)
        {
            lock(this)
            {
                #if TRACE_REGISTRATION
                    #if TRACE_STACK_TRACE
                        try
                        {
                            throw new System.Exception();
                        }
                        catch(System.Exception ex)
                        {
                            trace("removing handler for channel " + fd.Handle + "\n" + ex);
                        }
                    #else
                        trace("removing handler for channel " + fd.Handle);
                    #endif
                #endif
                
                Debug.Assert(!_destroyed);
                _changes.Add(new FdHandlerPair(fd, null));
                setInterrupt();
            }
        }
        
        public void promoteFollower()
        {
            if(sizeMax_ > 1)
            {
                lock(this)
                {
                    Debug.Assert(!promote_);
                    promote_ = true;
                    System.Threading.Monitor.Pulse(this);
                    
                    if(!_destroyed)
                    {
                        Debug.Assert(inUse_ >= 0);
                        ++inUse_;
                        
                        if(inUse_ == sizeWarn_)
                        {
                            string s = "thread pool `" + _prefix + "' is running low on threads\n"
                                       + "Size=" + size_ + ", " + "SizeMax=" + sizeMax_ + ", "
                                       + "SizeWarn=" + sizeWarn_;
                            instance_.initializationData().logger.warning(s);
                        }
                        
                        Debug.Assert(inUse_ <= running_);
                        if(inUse_ < sizeMax_ && inUse_ == running_)
                        {
                            try
                            {
                                EventHandlerThread thread = new EventHandlerThread(this, _programNamePrefix +
                                                                                   _prefix + "-" + threadIndex_++);
                                threads_.Add(thread);
                                thread.Start();
                                ++running_;
                            }
                            catch(System.Exception ex)
                            {
                                string s = "cannot create thread for `" + _prefix + "':\n" + ex;
                                instance_.initializationData().logger.error(s);
                            }
                        }
                    }
                }
            }
        }
        
        public void joinWithAllThreads()
        {
            //
            // threads_ is immutable after destroy() has been called,
            // therefore no synchronization is needed. (Synchronization
            // wouldn't be possible here anyway, because otherwise the
            // other threads would never terminate.)
            //
            Debug.Assert(_destroyed);
            foreach(EventHandlerThread thread in threads_)
            {
                while(true)
                {
                    thread.Join();
                    break;
                }
            }
            //
            // Close the socket pair.
            //
            try
            {
                Network.closeSocket(_fdIntrWrite);
                Network.closeSocket(_fdIntrRead);
            }
            catch(System.Exception)
            {
            }
        }
        
        public string prefix()
        {
            return _prefix;
        }

        private void clearInterrupt()
        {
            #if TRACE_INTERRUPT
                trace("clearInterrupt");
                #if TRACE_STACK_TRACE
                    try
                    {
                        throw new System.Exception();
                    }
                    catch(System.Exception ex)
                    {
                        Console.Error.WriteLine(ex);
                    }
                #endif
            #endif

        repeat:
            try
            {
                _fdIntrRead.Receive(_intrBuf);
            }
            catch(SocketException ex)
            {
                #if TRACE_INTERRUPT
                    trace("clearInterrupt, handling exception");
                #endif
                if(Network.interrupted(ex))
                {
                    goto repeat;
                }
                throw new Ice.SocketException(ex);
            }
        }
        
        private void setInterrupt()
        {
            #if TRACE_INTERRUPT
                trace("setInterrupt()");
                #if TRACE_STACK_TRACE
                    try
                    {
                        throw new System.Exception();
                    }
                    catch(System.Exception ex)
                    {
                        Console.Error.WriteLine(ex);
                    }
                #endif
            #endif

        repeat:
            try
            {
                _fdIntrWrite.Send(_intrBuf);
            }
            catch(SocketException ex)
            {
                #if TRACE_INTERRUPT
                    trace("setInterrupt, handling exception");
                #endif
                if(Network.interrupted(ex))
                {
                    goto repeat;
                }
                throw new Ice.SocketException(ex);
            }
        }

        private static byte[] _intrBuf = new byte[1];

        //
        // Each thread supplies a BasicStream, to avoid creating excessive
        // garbage.
        //
        private bool run(BasicStream stream)
        {
            if(sizeMax_ > 1)
            {
                lock(this)
                {
                    while(!promote_)
                    {
                        System.Threading.Monitor.Wait(this);
                    }

                    promote_ = false;
                }
                
                #if TRACE_THREAD
                    trace("thread " + System.Threading.Thread.CurrentThread.Name + " has the lock");
                #endif
            }
            
            while(true)
            {
                #if TRACE_REGISTRATION
                    trace("selecting on " + (_handlerMap.Count + 1) + " channels: ");
                    trace(_fdIntrRead.Handle.ToString());
                    foreach(Socket socket in _handlerMap.Keys)
                    {
                        trace(", " + socket.Handle);
                    }
                #endif
                
                ArrayList readList = new ArrayList(_handlerMap.Count + 1);
                readList.Add(_fdIntrRead);
                readList.AddRange(_handlerMap.Keys);

                Network.doSelect(readList, null, null, _timeout > 0 ? _timeout * 1000 : -1);

                EventHandler handler = null;
                bool finished = false;
                bool shutdown = false;

                lock(this)
                {
                    if(readList.Count == 0) // We initiate a shutdown if there is a thread pool timeout.
                    {
                        #if TRACE_SELECT
                            trace("timeout");
                        #endif
                        
                        Debug.Assert(_timeout > 0);
                        _timeout = 0;
                        shutdown = true;
                    }
                    else
                    {
                        if(readList.Contains(_fdIntrRead))
                        {
                            #if TRACE_SELECT || TRACE_INTERRUPT
                                trace("detected interrupt");
                            #endif
                            
                            //
                            // There are two possibilities for an interrupt:
                            //
                            // 1. The thread pool has been destroyed.
                            //
                            // 2. An event handler was registered or unregistered.
                            //
                            
                            //
                            // Thread pool destroyed?
                            //
                            if(_destroyed)
                            {
                                #if TRACE_SHUTDOWN
                                    trace("destroyed, thread id = " + System.Threading.Thread.CurrentThread.Name);
                                #endif
                                
                                //
                                // Don't clear the interrupt fd if
                                // destroyed, so that the other threads
                                // exit as well.
                                //
                                return true;
                            }
                            
                            //
                            // Remove the interrupt channel from the
                            // readList.
                            //
                            readList.Remove(_fdIntrRead);

                            clearInterrupt();
                            
                            //
                            // An event handler must have been registered
                            // or unregistered.
                            //
                            Debug.Assert(_changes.Count != 0);
                            LinkedList.Enumerator first = (LinkedList.Enumerator)_changes.GetEnumerator();
                            first.MoveNext();
                            FdHandlerPair change = (FdHandlerPair)first.Current;
                            first.Remove();
                            if(change.handler != null) // Addition if handler is set.
                            {
                                _handlerMap[change.fd] = change.handler;
                    
                                #if TRACE_REGISTRATION
                                    trace("added handler (" + change.handler.GetType().FullName + ") for fd "
                                          + change.fd.Handle);
                                #endif
                    
                                continue;
                            }
                            else // Removal if handler is not set.
                            {
                                handler = (EventHandler)_handlerMap[change.fd];
                                _handlerMap.Remove(change.fd);
                                finished = true;
                    
                                #if TRACE_REGISTRATION
                                    trace("removed handler (" + handler.GetType().FullName + ") for fd "
                                          + change.fd.Handle);
                                #endif
                    
                                // Don't continue; we have to call
                                // finished() on the event handler below,
                                // outside the thread synchronization.
                            }
                        }
                        else
                        {
                            Socket fd = (Socket)readList[0];
                            #if TRACE_SELECT
                                trace("found a readable socket: " + fd.Handle);
                            #endif
                            handler = (EventHandler)_handlerMap[fd];
                                    
                            if(handler == null)
                            {
                                #if TRACE_SELECT
                                    trace("socket " + fd.Handle + " not registered with " + _prefix);
                                #endif
                                
                                continue;
                            }
                        }
                    }
                }

                //
                // Now we are outside the thread synchronization.
                //
                
                if(shutdown)
                {
                    #if TRACE_SHUTDOWN
                        trace("shutdown detected");
                    #endif
                    
                    //
                    // Initiate server shutdown.
                    //
                    ObjectAdapterFactory factory;
                    try
                    {
                        factory = instance_.objectAdapterFactory();
                    }
                    catch(Ice.CommunicatorDestroyedException)
                    {
                        continue;
                    }
                    
                    promoteFollower();
                    factory.shutdown();

                    //
                    // No "continue", because we want shutdown to be done in
                    // its own thread from this pool. Therefore we called
                    // promoteFollower();
                    //
                }
                else
                {
                    Debug.Assert(handler != null);

                    if(finished)
                    {
                        //
                        // Notify a handler about it's removal from
                        // the thread pool.
                        //
                        try
                        {
                            handler.finished(this);
                        }
                        catch(Ice.LocalException ex)
                        {
                            string s = "exception in `" + _prefix + "' while calling finished():\n"
                                       + ex + "\n" + handler.ToString();
                            instance_.initializationData().logger.error(s);
                        }

                        //
                        // No "continue", because we want finished() to be
                        // called in its own thread from this pool. Note
                        // that this means that finished() must call
                        // promoteFollower().
                        //
                    }
                    else
                    {
                        //
                        // If the handler is "readable", try to read a
                        // message.
                        //
                        try
                        {
                            if(handler.readable())
                            {
                                try
                                {
                                    read(handler);
                                }
                                catch(Ice.TimeoutException) // Expected.
                                {
                                    continue;
                                }
                                catch(Ice.DatagramLimitException) // Expected.
                                {
                                    continue;
                                }
                                catch(Ice.SocketException ex)
                                {
                                    #if TRACE_EXCEPTION
                                        trace("informing handler (" + handler.GetType().FullName + ") about "
                                              + ex.GetType().FullName + " exception " + ex);
                                    #endif
                                    
                                    handler.exception(ex);
                                    continue;
                                }
                                catch(Ice.LocalException ex)
                                {
                                    if(handler.datagram())
                                    {
                                        if(instance_.initializationData().properties.getPropertyAsInt(
                                                                                        "Ice.Warn.Connections") > 0)
                                        {
                                            instance_.initializationData().logger.warning(
                                                                       "datagram connection exception:\n" + ex + 
                                                                       handler.ToString());
                                        }
                                    }
                                    else
                                    {
                                        #if TRACE_EXCEPTION
                                            trace("informing handler (" + handler.GetType().FullName + ") about "
                                                  + ex.GetType().FullName + " exception " + ex);
                                        #endif
                                    
                                        handler.exception(ex);
                                    }
                                    continue;
                                }
                                
                                stream.swap(handler.stream_);
                                Debug.Assert(stream.pos() == stream.size());
                            }
                            
                            //
                            // Provide a new message to the handler.
                            //
                            try
                            {
                                handler.message(stream, this);
                            }
                            catch(Ice.LocalException ex)
                            {
                                string s = "exception in `" + _prefix + "' while calling message():\n" + ex;
                                instance_.initializationData().logger.error(s);
                            }

                            //
                            // No "continue", because we want message() to
                            // be called in its own thread from this
                            // pool. Note that this means that message()
                            // must call promoteFollower().
                            //
                        }
                        finally
                        {
                            stream.reset();
                        }
                    }
                }
                
                if(sizeMax_ > 1)
                {
                    lock(this)
                    {
                        if(!_destroyed)
                        {
                            //
                            // First we reap threads that have been
                            // destroyed before.
                            //
                            int sz = threads_.Count;
                            Debug.Assert(running_ <= sz);
                            if(running_ < sz)
                            {
                                ArrayList liveThreads = new ArrayList();
                                foreach(EventHandlerThread thread in threads_)
                                {
                                    if(!thread.IsAlive())
                                    {
                                        thread.Join();
                                    }
                                    else
                                    {
                                        liveThreads.Add(thread);
                                    }
                                }
                                threads_ = liveThreads;
                            }
                            
                            //
                            // Now we check if this thread can be destroyed, based
                            // on a load factor.
                            //

                            //
                            // The load factor jumps immediately to the number of
                            // threads that are currently in use, but decays
                            // exponentially if the number of threads in use is
                            // smaller than the load factor. This reflects that we
                            // create threads immediately when they are needed,
                            // but want the number of threads to slowly decline to
                            // the configured minimum.
                            //
                            double inUse = (double)inUse_;
                            if(load_ < inUse)
                            {
                                load_ = inUse;
                            }
                            else
                            {
                                double loadFactor = 0.05; // TODO: Configurable?
                                double oneMinusLoadFactor = 1 - loadFactor;
                                load_ = load_ * oneMinusLoadFactor + inUse * loadFactor;
                            }

                            if(running_ > size_)
                            {
                                int load = (int)(load_ + 0.5);

                                //
                                // We add one to the load factor because on
                                // additional thread is needed for select().
                                //
                                if(load  + 1 < running_)
                                {
                                    Debug.Assert(inUse_ > 0);
                                    --inUse_;
                                    
                                    Debug.Assert(running_ > 0);
                                    --running_;
                                    
                                    return false;
                                }
                            }
                            
                            Debug.Assert(inUse_ > 0);
                            --inUse_;
                        }
                        
                        while(!promote_)
                        {
                            System.Threading.Monitor.Wait(this);
                        }
                        
                        promote_ = false;
                    }
                    
                    #if TRACE_THREAD
                        trace("thread " + System.Threading.Thread.CurrentThread.Name + " has the lock");
                    #endif
                }
            }
        }
        
        private void read(EventHandler handler)
        {
            BasicStream stream = handler.stream_;
            
            if(stream.size() == 0)
            {
                stream.resize(Protocol.headerSize, true);
                stream.pos(0);
            }
            
            if(stream.pos() != stream.size())
            {
                handler.read(stream);
                Debug.Assert(stream.pos() == stream.size());
            }
            
            int pos = stream.pos();
            if(pos < Protocol.headerSize)
            {
                //
                // This situation is possible for small UDP packets.
                //
                throw new Ice.IllegalMessageSizeException();
            }
            stream.pos(0);
            byte[] m = new byte[4];
            m[0] = stream.readByte();
            m[1] = stream.readByte();
            m[2] = stream.readByte();
            m[3] = stream.readByte();
            if(m[0] != Protocol.magic[0] || m[1] != Protocol.magic[1] ||
               m[2] != Protocol.magic[2] || m[3] != Protocol.magic[3])
            {
                Ice.BadMagicException ex = new Ice.BadMagicException();
                ex.badMagic = m;
                throw ex;
            }
            
            byte pMajor = stream.readByte();
            byte pMinor = stream.readByte();
            if(pMajor != Protocol.protocolMajor || pMinor > Protocol.protocolMinor)
            {
                Ice.UnsupportedProtocolException e = new Ice.UnsupportedProtocolException();
                e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
                e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
                e.major = Protocol.protocolMajor;
                e.minor = Protocol.protocolMinor;
                throw e;
            }
            
            byte eMajor = stream.readByte();
            byte eMinor = stream.readByte();
            if(eMajor != Protocol.encodingMajor || eMinor > Protocol.encodingMinor)
            {
                Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
                e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
                e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
                e.major = Protocol.encodingMajor;
                e.minor = Protocol.encodingMinor;
                throw e;
            }
            
            stream.readByte(); // Message type.
            stream.readByte(); // Compression status.
            int size = stream.readInt();
            if(size < Protocol.headerSize)
            {
                throw new Ice.IllegalMessageSizeException();
            }
            if(size > instance_.messageSizeMax())
            {
                throw new Ice.MemoryLimitException();
            }
            if(size > stream.size())
            {
                stream.resize(size, true);
            }
            stream.pos(pos);
            
            if(stream.pos() != stream.size())
            {
                if(handler.datagram())
                {
                    if(warnUdp_)
                    {
                        instance_.initializationData().logger.warning("DatagramLimitException: maximum size of " +
                                                                      stream.pos() + " exceeded");
                    }
                    stream.pos(0);
                    stream.resize(0, true);
                    throw new Ice.DatagramLimitException();
                }
                else
                {
                    handler.read(stream);
                    Debug.Assert(stream.pos() == stream.size());
                }
            }
        }
        
/*
 * Commented out because it is unused.
 *
        private void selectNonBlocking()
        {
            while(true)
            {
                #if TRACE_SELECT
                    trace("non-blocking select on " + _handlerMap.Count + " sockets, thread id = "
                          + System.Threading.Thread.CurrentThread.Name);
                #endif
                
                ArrayList readList = new ArrayList(_handlerMap.Count + 1);
                readList.Add(_fdIntrRead);
                readList.AddRange(_handlerMap.Keys);
                Network.doSelect(readList, null, null, 0);
                
                #if TRACE_SELECT
                    if(readList.Count > 0)
                    {
                        trace("after selectNow, there are " + readList.Count + " sockets:");
                        foreach(Socket socket in readList)
                        {
                            trace("  " + socket);
                        }
                    }
                #endif
                
                break;
            }
        }
*/
        
/*
 * Commented out because it is unused.
 *
        private void trace(string msg)
        {
            System.Console.Error.WriteLine(_prefix + "(" + System.Threading.Thread.CurrentThread.Name + "): " + msg);
        }
 */
        
        private sealed class FdHandlerPair
        {
            internal Socket fd;
            internal EventHandler handler;
            
            internal FdHandlerPair(Socket fd, EventHandler handler)
            {
                this.fd = fd;
                this.handler = handler;
            }
        }
        
        private Instance instance_;
        private bool _destroyed;
        private readonly string _prefix;
        private readonly string _programNamePrefix;
        
        private Socket _fdIntrRead;
        private Socket _fdIntrWrite;
        
        private IceUtil.LinkedList _changes = new IceUtil.LinkedList();
        
        private Hashtable _handlerMap = new Hashtable();
        
        private int _timeout;
        
        private sealed class EventHandlerThread
        {
            private ThreadPool thread_Pool;

            internal EventHandlerThread(ThreadPool threadPool, string name)
                : base()
            {
                thread_Pool = threadPool;
                name_ = name;
            }

            public bool IsAlive()
            {
                return thread_.IsAlive;
            }

            public void Join()
            {
                thread_.Join();
            }

            public void Start()
            {
                thread_ = new Thread(new ThreadStart(Run));
                thread_.IsBackground = true;
                thread_.Name = name_;
                thread_.Start();
            }

            public void Run()
            {
                if(thread_Pool.instance_.initializationData().threadHook != null)
                {
                    thread_Pool.instance_.initializationData().threadHook.start();
                }

                BasicStream stream = new BasicStream(thread_Pool.instance_);
                
                bool promote;
                
                try
                {
                    promote = thread_Pool.run(stream);
                }
                catch(Ice.LocalException ex)
                {
                    string s = "exception in `" + thread_Pool._prefix + "' thread " + thread_.Name + ":\n" + ex;
                    thread_Pool.instance_.initializationData().logger.error(s);
                    promote = true;
                }
                catch(System.Exception ex)
                {
                    string s = "unknown exception in `" + thread_Pool._prefix + "' thread " + thread_.Name + ":\n" + ex;
                    thread_Pool.instance_.initializationData().logger.error(s);
                    promote = true;
                }
                
                if(promote && thread_Pool.sizeMax_ > 1)
                {
                    //
                    // Promote a follower, but w/o modifying inUse_ or
                    // creating new threads.
                    //
                    lock(thread_Pool)
                    {
                        Debug.Assert(!thread_Pool.promote_);
                        thread_Pool.promote_ = true;
                        System.Threading.Monitor.Pulse(thread_Pool);
                    }
                }

                if(thread_Pool.instance_.initializationData().threadHook != null)
                {
                    thread_Pool.instance_.initializationData().threadHook.stop();
                }
                
                #if TRACE_THREAD
                    thread_Pool.trace("run() terminated");
                #endif
            }

            private string name_;
            private Thread thread_;
        }
        
        private readonly int size_; // Number of threads that are pre-created.
        private readonly int sizeMax_; // Maximum number of threads.
        private readonly int sizeWarn_; // If inUse_ reaches sizeWarn_, a "low on threads" warning will be printed.
        
        private ArrayList threads_; // All threads, running or not.
        private int threadIndex_; // For assigning thread names.
        private int running_; // Number of running threads.
        private int inUse_; // Number of threads that are currently in use.
        private double load_; // Current load in number of threads.
        
        private bool promote_;
        
        private readonly bool warnUdp_;
    }

}

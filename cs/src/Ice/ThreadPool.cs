// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
    using IceUtil;

    public sealed class ThreadPool
    {  
	public ThreadPool(Instance instance, string prefix, int timeout)
	{
	    _instance = instance;
	    _destroyed = false;
	    _prefix = prefix;
	    _handlerMap = new Hashtable();
	    _timeout = timeout > 0 ? timeout * 1000 : -1;
	    _threadIndex = 0;
	    _running = 0;
	    _inUse = 0;
	    _load = 0;
	    _promote = true;
	    _warnUdp = _instance.properties().getPropertyAsInt("Ice.Warn.Datagrams") > 0;
	    
	    string programName = _instance.properties().getProperty("Ice.ProgramName");
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
	    
	    _changes = new IceUtil.LinkedList();

	    //
	    // We use just one thread as the default. This is the fastest
	    // possible setting, still allows one level of nesting, and
	    // doesn't require to make the servants thread safe.
	    //
	    int size = _instance.properties().getPropertyAsIntWithDefault(_prefix + ".Size", 1);
	    if(size < 1)
	    {
		size = 1;
	    }
	    
	    int sizeMax = _instance.properties().getPropertyAsIntWithDefault(_prefix + ".SizeMax", _size * 10);
	    if(sizeMax < _size)
	    {
		sizeMax = _size;
	    }
	    
	    int sizeWarn = _instance.properties().getPropertyAsIntWithDefault(_prefix + ".SizeWarn", _sizeMax * 80 / 100);
	    _size = size;
	    _sizeMax = sizeMax;
	    _sizeWarn = sizeWarn;
	    
	    _messageSizeMax = _instance.messageSizeMax();
	    
	    try
	    {
		_threads = new ArrayList();
		for(int i = 0; i < _size; ++i)
		{
		    EventHandlerThread thread = new EventHandlerThread(this,
								       _programNamePrefix + _prefix + "-" + _threadIndex++);
		    _threads.Add(thread);
		    thread.Start();
		    ++_running;
		}
	    }
	    catch(System.Exception ex)
	    {
		string s = "cannot create thread for `" + _prefix + "':\n" + ex;
		_instance.logger().error(s);
		
		destroy();
		joinWithAllThreads();
		throw ex;
	    }
	}
	
	~ThreadPool()
	{
	    Network.closeSocket(_fdIntrWrite);
	    Network.closeSocket(_fdIntrRead);
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
	
	public void _register(Socket fd, EventHandler handler)
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
	    if(_sizeMax > 1)
	    {
		lock(this)
		{
		    Debug.Assert(!_promote);
		    _promote = true;
		    System.Threading.Monitor.Pulse(this);
		    
		    if(!_destroyed)
		    {
			Debug.Assert(_inUse >= 0);
			++_inUse;
			
			if(_inUse == _sizeWarn)
			{
			    string s = "thread pool `" + _prefix + "' is running low on threads\n"
					      + "Size=" + _size + ", " + "SizeMax=" + _sizeMax + ", "
					      + "SizeWarn=" + _sizeWarn;
			    _instance.logger().warning(s);
			}
			
			Debug.Assert(_inUse <= _running);
			if(_inUse < _sizeMax && _inUse == _running)
			{
			    try
			    {
				EventHandlerThread thread = new EventHandlerThread(this,
										   _programNamePrefix
										   + _prefix + "-" + _threadIndex++);
				_threads.Add(thread);
				thread.Start();
				++_running;
			    }
			    catch(System.Exception ex)
			    {
				string s = "cannot create thread for `" + _prefix + "':\n" + ex;
				_instance.logger().error(s);
			    }
			}
		    }
		}
	    }
	}
	
	public void joinWithAllThreads()
	{
	    //
	    // _threads is immutable after destroy() has been called,
	    // therefore no synchronization is needed. (Synchronization
	    // wouldn't be possible here anyway, because otherwise the
	    // other threads would never terminate.)
	    //
	    Debug.Assert(_destroyed);
	    foreach(EventHandlerThread thread in _threads)
	    {
		while(true)
		{
		    try
		    {
			thread.Join();
			break;
		    }
		    catch(System.Threading.ThreadInterruptedException)
		    {
		    }
		}
	    }
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
			SupportClass.WriteStackTrace(ex, Console.Error);
		    }
		#endif
	    #endif
	    
	    byte[] buf = new byte[1];

	repeat:
	    try
	    {
		_fdIntrRead.Receive(buf);
	    }
	    catch(SocketException ex)
	    {
		if(Network.interrupted(ex))
		{
		    goto repeat;
		}
		throw new Ice.SocketException("Could not read from interrupt socket", ex);
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
		throw new Ice.SocketException("Could not write to interrupt socket", ex);
	    }
	}

	private static byte[] _intrBuf = new byte[1];

	//
	// Each thread supplies a BasicStream, to avoid creating excessive
	// garbage.
	//
	private bool run(BasicStream stream)
	{
	    if(_sizeMax > 1)
	    {
		lock(this)
		{
		    while(!_promote)
		    {
			try
			{
			    System.Threading.Monitor.Wait(this);
			}
			catch(System.Threading.ThreadInterruptedException)
			{
			}
		    }
		    
		    _promote = false;
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


		Network.doSelect(readList, null, null, _timeout);

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
			#if TRACE_SELECT || TRACE_INTERRUPT
			    System.Text.StringBuilder sb = new System.Text.StringBuilder();
			    sb.Append("readable sockets:");
			    foreach(Socket s in readList)
			    {
				sb.Append(" " + s.Handle);
				    if(_handlerMap[s] != null)
				    {
					sb.Append(" (" + _handlerMap[s].GetType().FullName + ") ");
				    }
			    }
			    trace("readable sockets: " + sb.ToString());
			#endif

			if(readList.Contains(_fdIntrRead))
			{
			    #if TRACE_SELECT || TRACE_INTERRUPT
				trace("detected interrupt");
			    #endif
			    
			    //
			    // There are three possibilities for an interrupt:
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
				// Don't clear the interrupt fd if destroyed,
				// so that the other threads exit as well.
				//
				return true;
			    }
			    
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
			factory = _instance.objectAdapterFactory();
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
			    _instance.logger().error(s);
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
				catch(Ice.LocalException ex)
				{
				    #if TRACE_EXCEPTION
					trace("informing handler (" + handler.GetType().FullName + ") about exception " + ex);
				    #endif
				    
				    handler.exception(ex);
				    continue;
				}
				
				stream.swap(handler._stream);
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
			        string s = "exception in `" + _prefix + "' while calling finished():\n" + ex;
			    	_instance.logger().error(s);
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
		
		if(_sizeMax > 1)
		{
		    lock(this)
		    {
			if(!_destroyed)
			{
			    //
			    // First we reap threads that have been
			    // destroyed before.
			    //
			    int sz = _threads.Count;
			    Debug.Assert(_running <= sz);
			    if(_running < sz)
			    {
				ArrayList newThreads = new ArrayList();
				foreach(EventHandlerThread thread in _threads)
				{
				    if(!thread.IsAlive)
				    {
					try
					{
					    thread.Join();
					    newThreads.Add(thread);
					}
					catch(System.Threading.ThreadInterruptedException)
					{
					}
				    }
				}
				_threads = newThreads;
			    }
			    
			    //
			    // Now we check if this thread can be destroyed, based
			    // on a load factor.
			    //
			    double loadFactor = 0.05; // TODO: Configurable?
			    double oneMinusLoadFactor = 1 - loadFactor;
			    _load = _load * oneMinusLoadFactor + _inUse * loadFactor;
			    
			    if(_running > _size)
			    {
				int load = (int)(_load + 1);
				if(load < _running)
				{
				    Debug.Assert(_inUse > 0);
				    --_inUse;
				    
				    Debug.Assert(_running > 0);
				    --_running;
				    
				    return false;
				}
			    }
			    
			    Debug.Assert(_inUse > 0);
			    --_inUse;
			}
			
			while(!_promote)
			{
			    try
			    {
				System.Threading.Monitor.Wait(this);
			    }
			    catch(System.Threading.ThreadInterruptedException)
			    {
			    }
			}
			
			_promote = false;
		    }
		    
		    #if TRACE_THREAD
			trace("thread " + System.Threading.Thread.CurrentThread.Name + " has the lock");
		    #endif
		}
	    }
	}
	
	private void read(EventHandler handler)
	{
	    BasicStream stream = handler._stream;
	    
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
	    Debug.Assert(pos >= Protocol.headerSize);
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
		ex.badMagic = new Ice.ByteSeq(m);
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
	    
	    byte messageType = stream.readByte();
	    byte compress = stream.readByte();
	    int size = stream.readInt();
	    if(size < Protocol.headerSize)
	    {
		throw new Ice.IllegalMessageSizeException();
	    }
	    if(size > _messageSizeMax)
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
		    if(_warnUdp)
		    {
			_instance.logger().warning("DatagramLimitException: maximum size of " + stream.pos() + " exceeded");
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
	
	private void selectNonBlocking()
	{
	    while(true)
	    {
		try
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
		catch(System.Threading.ThreadInterruptedException)
		{
		    continue;
		}
	    }
	}
	
	private void trace(string msg)
	{
	    System.Console.Error.WriteLine(_prefix + ": " + msg);
	}
	
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
	
	private Instance _instance;
	private bool _destroyed;
	private readonly string _prefix;
	private readonly string _programNamePrefix;
	
	private Socket _fdIntrRead;
	private Socket _fdIntrWrite;
	
	private IceUtil.LinkedList _changes;
	
	private Hashtable _handlerMap;
	
	private int _timeout;
	
	private sealed class EventHandlerThread : SupportClass.ThreadClass
	{
	    private void InitBlock(ThreadPool enclosingInstance)
	    {
		this.enclosingInstance = enclosingInstance;
	    }

	    private ThreadPool enclosingInstance;
	    public ThreadPool Enclosing_Instance
	    {
		get
		{
		    return enclosingInstance;
		}
	    }

	    internal EventHandlerThread(ThreadPool enclosingInstance, string name) : base()
	    {
		InitBlock(enclosingInstance);
		Name = name;
	    }
	    
	    override public void Run()
	    {
		BasicStream stream = new BasicStream(Enclosing_Instance._instance);
		
		bool promote;
		
		try
		{
		    promote = Enclosing_Instance.run(stream);
		}
		catch(Ice.LocalException ex)
		{
		    string s = "exception in `" + Enclosing_Instance._prefix + "' thread " + Name + ":\n" + ex;
		    Enclosing_Instance._instance.logger().error(s);
		    promote = true;
		}
		catch(System.Exception ex)
		{
		    string s = "unknown exception in `" + Enclosing_Instance._prefix + "' thread " + Name + ":\n" + ex;
		    Enclosing_Instance._instance.logger().error(s);
		    promote = true;
		}
		
		if(promote && Enclosing_Instance._sizeMax > 1)
		{
		    //
		    // Promote a follower, but w/o modifying _inUse or
		    // creating new threads.
		    //
		    lock(Enclosing_Instance)
		    {
			Debug.Assert(!Enclosing_Instance._promote);
			Enclosing_Instance._promote = true;
			System.Threading.Monitor.Pulse(Enclosing_Instance);
		    }
		}
		
		stream.destroy();
       
		#if TRACE_THREAD
		    Enclosing_Instance.trace("run() terminated");
		#endif
	    }
	}
	
	private readonly int _size; // Number of threads that are pre-created.
	private readonly int _sizeMax; // Maximum number of threads.
	private readonly int _sizeWarn; // If _inUse reaches _sizeWarn, a "low on threads" warning will be printed.
	
	private readonly int _messageSizeMax;
	
	private ArrayList _threads; // All threads, running or not.
	private int _threadIndex; // For assigning thread names.
	private int _running; // Number of running threads.
	private int _inUse; // Number of threads that are currently in use.
	private double _load; // Current load in number of threads.
	
	private bool _promote;
	
	private readonly bool _warnUdp;
    }

}

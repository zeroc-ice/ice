// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// Work-around for bug in .NET Socket class implementation.
//

namespace IceInternal
{

    using System;
    using System.Collections;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;
    using System.Runtime.InteropServices;

    public sealed class Network
    {
	//
	// Magic numbers taken from winsock2.h
	//
	const int WSAEINTR = 10004;
	const int WSAEFAULT = 10014;
	const int WSAEINVAL = 10022;
	const int WSAEWOULDBLOCK = 10035;
	const int WSAEINPROGRESS = 10036; // Deprecated in winsock2, but still used by Mono Beta 1
	const int WSAEMSGSIZE = 10040;
	const int WSAENETDOWN = 10050;
	const int WSAENETUNREACH = 10051;
	const int WSAENETRESET = 10052;
	const int WSAECONNABORTED = 10053;
	const int WSAECONNRESET = 10054;
	const int WSAENOBUFS = 10055;
	const int WSAENOTCONN = 10057;
	const int WSAESHUTDOWN = 10058;
	const int WSAETIMEDOUT = 10060;
	const int WSAECONNREFUSED = 100061;
	const int WSAEHOSTUNREACH = 100065;
	const int WSATRY_AGAIN = 11002;

	public static bool interrupted(Win32Exception ex)
	{
	    return ex.NativeErrorCode == WSAEINTR;
	}

	public static bool acceptInterrupted(Win32Exception ex)
	{
	    if(interrupted(ex))
	    {
		return true;
	    }
	    int error = ex.NativeErrorCode;
	    return error == WSAECONNABORTED ||
		   error == WSAECONNRESET ||
		   error == WSAETIMEDOUT;
	}

	public static bool noBuffers(Win32Exception ex)
	{
	    int error = ex.NativeErrorCode;
	    return error == WSAENOBUFS ||
		   error == WSAEFAULT;
	}

	public static bool wouldBlock(Win32Exception ex)
	{
	    return ex.NativeErrorCode == WSAEWOULDBLOCK;
	}

	public static bool connectFailed(Win32Exception ex)
	{
	    int error = ex.NativeErrorCode;
	    return error == WSAETIMEDOUT ||
		   error == WSAENETUNREACH ||
		   error == WSAEHOSTUNREACH ||
		   error == WSAECONNRESET ||
		   error == WSAESHUTDOWN ||
		   error == WSAECONNABORTED ||
		   error == WSAENETDOWN;
	}

	public static bool connectInProgress(Win32Exception ex)
	{
	    int error = ex.NativeErrorCode;
	    return error == WSAEWOULDBLOCK ||
	           error == WSAEINPROGRESS;
	}

	public static bool connectionLost(Win32Exception ex)
	{
	    int error = ex.NativeErrorCode;
	    return error == WSAECONNRESET ||
		   error == WSAESHUTDOWN ||
		   error == WSAECONNABORTED ||
		   error == WSAENETDOWN ||
		   error == WSAENETRESET;
	}

	public static bool connectionRefused(Win32Exception ex)
	{
	    return ex.NativeErrorCode == WSAECONNREFUSED;
	}
	
	public static bool notConnected(Win32Exception ex)
	{
	    // BUGFIX: WSAEINVAL because shutdown() under MacOS returns EINVAL if the server side is gone.
	    return ex.NativeErrorCode == WSAENOTCONN || ex.NativeErrorCode == WSAEINVAL;
	}

	public static bool recvTruncated(Win32Exception ex)
	{
	    return ex.NativeErrorCode == WSAEMSGSIZE;
	}

	public static Socket createSocket(bool udp)
	{
	    Socket socket;

	    try
	    {
		if(udp)
		{
		    socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
		}
		else
		{
		    socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
		}
	    }
	    catch(SocketException ex)
	    {
		throw new Ice.SocketException("Cannot create socket", ex);
	    }

	    if(!udp)
	    {
		try
		{
		    setTcpNoDelay(socket);
		    socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, 1);
		}
		catch(SocketException ex)
		{
		    closeSocketNoThrow(socket);
		    throw new Ice.SocketException("Cannot set socket options", ex);
		}
	    }
	    return socket;
	}

	private static void closeSocketNoThrow(Socket socket)
	{
	    if(socket == null)
	    {
		return;
	    }
	    try
	    {
		socket.Close();
	    }
	    catch(SocketException)
	    {
	        // Ignore
	    }
	}

	public static void closeSocket(Socket socket)
	{
	    if(socket == null)
	    {
		return;
	    }
	    try
	    {
		socket.Close();
	    }
	    catch(SocketException ex)
	    {
		throw new Ice.SocketException("Cannot close socket", ex);
	    }
	}

	public static void setBlock(Socket socket, bool block)
	{
	    try
	    {
		socket.Blocking = block;
	    }
	    catch(SocketException ex)
	    {
	        closeSocketNoThrow(socket);
		throw new Ice.SocketException(ex);
	    }
	}

	public static void setTcpNoDelay(Socket socket)
	{
	    try
	    {
		socket.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.NoDelay, 1);
	    }
	    catch(System.Exception ex)
	    {
	        closeSocketNoThrow(socket);
		throw new Ice.SocketException("Cannot set NoDelay option", ex);
	    }
	}

	public static void setKeepAlive(Socket socket)
	{
	    try
	    {
		socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, 1);
	    }
	    catch(System.Exception ex)
	    {
	        closeSocketNoThrow(socket);
		throw new Ice.SocketException("Cannot set KeepAlive option", ex);
	    }
	}

	public static void setSendBufferSize(Socket socket, int sz)
	{
	    try
	    {
		socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer, sz);
	    }
	    catch(SocketException ex)
	    {
	        closeSocketNoThrow(socket);
		throw new Ice.SocketException("Cannot set send buffer size", ex);
	    }
	}

	public static int getSendBufferSize(Socket socket)
	{
	    int sz;
	    try
	    {
		sz = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer);
	    }
	    catch(SocketException ex)
	    {
	        closeSocketNoThrow(socket);
		throw new Ice.SocketException("Cannot read send buffer size", ex);
	    }
	    return sz;
	}
	
	public static void setRecvBufferSize(Socket socket, int sz)
	{
	    try
	    {
		socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer, sz);
	    }
	    catch(SocketException ex)
	    {
	        closeSocketNoThrow(socket);
		throw new Ice.SocketException("Cannot set receive buffer size", ex);
	    }
	}

	public static int getRecvBufferSize(Socket socket)
	{
	    int sz = 0;
	    try
	    {
		sz = (int)socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer);
	    }
	    catch(SocketException ex)
	    {
	        closeSocketNoThrow(socket);
		throw new Ice.SocketException("Cannot read receive buffer size", ex);
	    }
	    return sz;
	}
	
	public static IPEndPoint doBind(Socket socket, EndPoint addr)
	{
	    try
	    {
		//
		// Don't set ReuseAddress on Win32 -- it allows two processes to bind to the same port!
		//
		//socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, 1);
		socket.Bind(addr);
		return (IPEndPoint)socket.LocalEndPoint;
	    }
	    catch(SocketException ex)
	    {
	        closeSocketNoThrow(socket);
		throw new Ice.SocketException("Cannot bind", ex);
	    }
	}
	
	public static void doListen(Socket socket, int backlog)
	{
	repeatListen:

	    try
	    {
		socket.Listen(backlog);
	    }
	    catch(SocketException ex)
	    {
		if(interrupted(ex))
		{
		    goto repeatListen;
		}

		closeSocketNoThrow(socket);
		throw new Ice.SocketException("Cannot listen", ex);
	    }
	}

	public static void doConnect(Socket socket, EndPoint addr, int timeout)
	{
            //
            // Set larger send buffer size to avoid performance problems on
            // WIN32.
            //
	    if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
	    {
		setSendBufferSize(socket, 64 * 1024);
	    }

	repeatConnect:
	    try
	    {

		socket.Connect(addr);
	    }
	    catch(SocketException ex)
	    {
		if(interrupted(ex))
		{
		    goto repeatConnect;
		}
		if(!connectInProgress(ex))
		{
		    closeSocketNoThrow(socket);
		    throw new Ice.ConnectFailedException("Connect failed", ex);
		}

	    repeatSelect:
		bool ready;
		bool error;
		try
		{
		    ArrayList writeList = new ArrayList();
		    writeList.Add(socket);
		    ArrayList errorList = new ArrayList();
		    errorList.Add(socket);
		    doSelect(null, writeList, errorList, timeout);
		    ready = writeList.Count != 0;
		    error = errorList.Count != 0;

		    //
		    // TODO: Mono 1.0 bug: Under Linux/Mono, dual-CPU machine, we see Select() return
		    // a writable socket after a non-blocking connect has thrown a SocketException with
		    // NativeErrorCode 10036 (WSAEWOULDBLOCK). The call to Select() that follows returns
		    // the socket as writable, indicating that the socket is now connected. In addition,
		    // retrieving the error status with GetSocketOption returns zero. But calling
		    // Connected() at that point returns false, and any subsequent call to read()
		    // on that socket fails with WSAECONNREFUSED.
		    //
		    // The only fix appears to be to restart the call to Connect() -- the second attempt
		    // works and establishes the connection.
		    //
		    if(ready && AssemblyUtil.runtime_ == AssemblyUtil.Runtime.Mono && !socket.Connected)
		    {
			goto repeatConnect;
		    }
		    Debug.Assert(!(ready && error));
		}
		catch(SocketException e)
		{
		    if(interrupted(e))
		    {
			goto repeatSelect;
		    }

		    closeSocketNoThrow(socket);
		    throw new Ice.SocketException(e);
		}
		if(error || !ready)
		{
		    closeSocketNoThrow(socket);
		    if(error)
		    {
		        throw new Ice.ConnectFailedException("Connect failed: connection refused");
		    }
		    else
		    {
		        throw new Ice.ConnectTimeoutException("Connect timed out after " + timeout + " msec");
		    }
		}
	    }
	}
	
	public static Socket doAccept(Socket socket, int timeout)
	{
	    Socket ret = null;

	repeatAccept:
	    try
	    {
		ret = socket.Accept();
	    }
	    catch(SocketException ex)
	    {
		if(acceptInterrupted(ex))
		{
		    goto repeatAccept;
		}
		if(wouldBlock(ex))
		{
		repeatSelect:
		    ArrayList readList = new ArrayList();
		    readList.Add(socket);
		    try
		    {
			doSelect(readList, null, null, timeout);
		    }
		    catch(System.Exception se)
		    {
			if(interrupted(ex))
			{
			    goto repeatSelect;
			}
			throw new Ice.SocketException("select failed", se);
		    }

		    if(readList.Count == 0)
		    {
			throw new Ice.TimeoutException();
		    }

		    goto repeatAccept;
		}
	    }

	    setTcpNoDelay(ret);
	    setKeepAlive(ret);
	    setSendBufferSize(ret, 64 * 1024);
	    
	    return ret;
	}
	
	private static ArrayList copyList(IList list)
	{
	    if(list == null)
	    {
		return null;
	    }
	    return new ArrayList(list);
	}

	private static void overwriteList(ArrayList from, IList to)
	{
	    if(from != null && to != null)
	    {
		to.Clear();
		foreach(object o in from)
		{
		    to.Add(o);
		}
	    }
	}

	public enum PollMode { Read, Write, Error };

	public static bool doPoll(Socket s, int timeout, PollMode mode)
	{
	    //
	    // Poll() wants microseconds, so we need to deal with overflow.
	    //
	    while((timeout > System.Int32.MaxValue / 1000))
	    {
		if(s.Poll((System.Int32.MaxValue / 1000) * 1000, (SelectMode)mode))
		{
		    return true;
		}
		timeout -= System.Int32.MaxValue / 1000;
	    }
	    return s.Poll(timeout * 1000, (SelectMode)mode);
	}

	public static void doSelect(IList checkRead, IList checkWrite, IList checkError, int milliSeconds)
	{
	    ArrayList cr = null;
	    ArrayList cw = null;
	    ArrayList ce = null;

	    if(milliSeconds < 0)
	    {
		//
		// Socket.Select() returns immediately if the timeout is < 0 (instead
		// of blocking indefinitely), so we have to emulate a blocking select here.
		// (Using Int32.MaxValue isn't good enough because that's only about 35 minutes.)
		//
		do {
		    cr = copyList(checkRead);
		    cw = copyList(checkWrite);
		    ce = copyList(checkError);
		    try
		    {
			Socket.Select(cr, cw, ce, System.Int32.MaxValue);
		    }
		    catch(SocketException e)
		    {
			if(interrupted(e))
			{
				continue;
			}
			throw new Ice.SocketException(e);
		    }
		}
		while((cr == null || cr.Count == 0) &&
		      (cw == null || cw.Count == 0) &&
		      (ce == null || ce.Count == 0));
		overwriteList(cr, checkRead);
		overwriteList(cw, checkWrite);
		overwriteList(ce, checkError);
	    }
	    else
	    {
		//
		// Select() wants microseconds, so we need to deal with overflow.
		//
		while((milliSeconds > System.Int32.MaxValue / 1000) &&
		      ((cr == null) || cr.Count == 0) &&
		      ((cw == null) || cw.Count == 0) &&
		      ((ce == null) || ce.Count == 0))
		{
		    cr = copyList(checkRead);
		    cw = copyList(checkWrite);
		    ce = copyList(checkError);
		    try
		    {
			Socket.Select(cr, cw, ce, (System.Int32.MaxValue / 1000) * 1000);
		    }
		    catch(SocketException e)
		    {
			if(interrupted(e))
			{
			    continue;
			}
			throw new Ice.SocketException(e);
		    }
		    milliSeconds -= System.Int32.MaxValue / 1000;
		}
		if(cr == null && cw == null && ce == null)
		{
		    Socket.Select(checkRead, checkWrite, checkError, milliSeconds * 1000);
		}
		overwriteList(cr, checkRead);
		overwriteList(cw, checkWrite);
		overwriteList(ce, checkError);
	    }
	}

	public static IPEndPoint getAddress(string host, int port)
	{
	    int retry = 5;

	repeatGetHostByName:
	    try
	    {
		IPHostEntry e = Dns.GetHostByName(host);
		Debug.Assert(e.AddressList.Length != 0);
		return new IPEndPoint(e.AddressList[0], port);
	    }
	    catch(Win32Exception ex)
	    {
		if(ex.NativeErrorCode == WSATRY_AGAIN && --retry >= 0)
		{
		    goto repeatGetHostByName;
		}
		Ice.DNSException e = new Ice.DNSException("GetHostByName failed", ex);
		e.host = host;
		throw e;
	    }
	    catch(System.Exception ex)
	    {
		Ice.DNSException e = new Ice.DNSException("GetHostByName failed", ex);
		e.host = host;
		throw e;
	    }
	}
	
	public static string getLocalHost(bool numeric)
	{
	    string hostname;

	    int retry = 5;

	repeatGetHostName:
	    try
	    {
		hostname = Dns.GetHostName();
	    }
	    catch(Win32Exception ex)
	    {
		if(ex.NativeErrorCode == WSATRY_AGAIN && --retry >= 0)
		{
		    goto repeatGetHostName;
		}
		Ice.DNSException e = new Ice.DNSException("GetHostName failed", ex);
		throw e;
	    }
	    catch(System.Exception ex)
	    {
		Ice.DNSException e = new Ice.DNSException("GetHostName failed", ex);
		throw e;
	    }
	    
	    if(numeric)
	    {
		retry = 5;

	    repeatGetHostByName:
		string numericHost;
		try
		{
		    numericHost = Dns.GetHostByName(hostname).AddressList[0].ToString();
		}
		catch(Win32Exception ex)
		{
		    if(ex.NativeErrorCode == WSATRY_AGAIN && --retry >= 0)
		    {
			goto repeatGetHostByName;
		    }
		    Ice.DNSException e = new Ice.DNSException("GetHostByName failed", ex);
		    e.host = hostname;
		    throw e;
		}
		catch(System.Exception ex)
		{
		    Ice.DNSException e = new Ice.DNSException("GetHostByName failed", ex);
		    e.host = hostname;
		    throw e;
		}
		hostname = numericHost;
	    }

	    return hostname;
	}
	 
	public sealed class SocketPair
	{
	    public Socket source;
	    public Socket sink;

	    public SocketPair()
	    {
		sink = createSocket(false);
		Socket listener = createSocket(false);


		doBind(listener, new IPEndPoint(IPAddress.Parse("127.0.0.1"), 0));
		doListen(listener, 1);
		doConnect(sink, listener.LocalEndPoint, 1000);
		try
		{
		    source = doAccept(listener, -1);
		}
		catch(Ice.SocketException ex)
		{
		    try
		    {
			sink.Close();
		    }
		    catch(System.Exception)
		    {
			// ignore
		    }
		    throw ex;
		}
		finally
		{
		    try
		    {
			listener.Close();
		    }
		    catch(System.Exception)
		    {
		    }
		}
	    }
	}

	public static SocketPair createPipe()
	{
	    return new SocketPair();
	}

	[StructLayout(LayoutKind.Sequential)]
	private struct in_addr
	{
	    [MarshalAs(UnmanagedType.ByValArray, SizeConst=4)]
	    public byte[] sin_addr;
	}

	[StructLayout(LayoutKind.Sequential)]
	private struct sockaddr
	{
	    public short sin_family;
	    public ushort sin_port;
	    public in_addr sin_addr;
	    [MarshalAs(UnmanagedType.ByValArray, SizeConst=8)]
	    public byte[] sin_zero;
	}

	[DllImport("wsock32.dll")]
	private static extern int getsockname(IntPtr s, ref sockaddr name, ref int namelen);

	[DllImport("wsock32.dll")]
	private static extern int getpeername(IntPtr s, ref sockaddr name, ref int namelen);

	[DllImport("ws2_32.dll")]
	private static extern IntPtr inet_ntoa(in_addr a);

	[DllImport("ws2_32.dll")]
	private static extern ushort ntohs(ushort netshort);

	public static string fdToString(Socket socket)
	{
	    if(socket == null)
	    {
		return "<closed>";
	    }

	    //
	    // .Net BUG: The LocalEndPoint and RemoteEndPoint properties
	    // are null for a socket that was connected in non-blocking
	    // mode. The only way to make this work is to step down to
	    // the native API and use platform invoke :-(
	    //
	    IPEndPoint localEndpoint;
	    IPEndPoint remoteEndpoint;
    	    if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
	    {
		sockaddr addr = new sockaddr();
		int addrLen = 16;

		if(getsockname(socket.Handle, ref addr, ref addrLen) != 0)
		{
		    throw new Ice.SyscallException("getsockname call failed");
		}
		string ip = Marshal.PtrToStringAnsi(inet_ntoa(addr.sin_addr));
		int port = ntohs(addr.sin_port);
		localEndpoint = new IPEndPoint(IPAddress.Parse(ip), port);

		remoteEndpoint = null;
		if(getpeername(socket.Handle, ref addr, ref addrLen) == 0)
		{ 
		    ip = Marshal.PtrToStringAnsi(inet_ntoa(addr.sin_addr));
		    port = ntohs(addr.sin_port);
		    remoteEndpoint = new IPEndPoint(IPAddress.Parse(ip), port);
		}
	    }
	    else
	    {
		try
		{
		    localEndpoint = (IPEndPoint)socket.LocalEndPoint;
		}
		catch(SocketException ex)
		{
		    throw new Ice.SocketException(ex);
		}

		try
		{
		    remoteEndpoint = (IPEndPoint)socket.RemoteEndPoint;
		}
		catch(SocketException)
		{
		    remoteEndpoint = null;
		}
	    }

	    System.Text.StringBuilder s = new System.Text.StringBuilder();
	    s.Append("local address = " + localEndpoint.Address);
	    s.Append(":" + localEndpoint.Port);

	    if(remoteEndpoint == null)
	    {
		s.Append("\nremote address = <not connected>");
	    }
	    else
	    {
		s.Append("\nremote address = " + IPAddress.Parse(remoteEndpoint.Address.ToString()));
		s.Append(":" + remoteEndpoint.Port.ToString());
	    }
	    
	    return s.ToString();
	}
	
	public static string
	addrToString(EndPoint addr)
	{
	    return addr.ToString();
	}
    }

}

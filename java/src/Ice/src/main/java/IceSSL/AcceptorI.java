// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class AcceptorI implements IceInternal.Acceptor
{
    @Override
    public java.nio.channels.ServerSocketChannel fd()
    {
        return _fd;
    }

    @Override
    public void close()
    {
        if(_fd != null)
        {
            IceInternal.Network.closeSocketNoThrow(_fd);
            _fd = null;
        }
    }

    @Override
    public IceInternal.EndpointI listen()
    {
        try
        {
            _addr = IceInternal.Network.doBind(_fd, _addr, _backlog);
        }
        catch(Ice.Exception ex)
        {
            _fd = null;
            throw ex;
        }
        _endpoint = _endpoint.endpoint(this);
        return _endpoint;
    }

    @Override
    public IceInternal.Transceiver accept()
    {
        //
        // The plug-in may not be fully initialized.
        //
        if(!_instance.initialized())
        {
            Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
            ex.reason = "IceSSL: plug-in is not initialized";
            throw ex;
        }

        IceInternal.StreamSocket stream = new IceInternal.StreamSocket(_instance, IceInternal.Network.doAccept(_fd));
        try
        {
            java.net.InetSocketAddress peerAddr =
                (java.net.InetSocketAddress)stream.fd().socket().getRemoteSocketAddress();
            return new TransceiverI(_instance, _instance.createSSLEngine(true, peerAddr), stream, _adapterName, true);
        }
        catch(RuntimeException ex)
        {
            stream.close();
            throw ex;
        }
    }

    @Override
    public String protocol()
    {
        return _instance.protocol();
    }

    @Override
    public String toString()
    {
        return IceInternal.Network.addrToString(_addr);
    }

    @Override
    public String toDetailedString()
    {
        StringBuffer s = new StringBuffer("local address = ");
        s.append(toString());

        java.util.List<String> intfs =
            IceInternal.Network.getHostsForEndpointExpand(_addr.getAddress().getHostAddress(),
                                                          _instance.protocolSupport(), true);
        if(!intfs.isEmpty())
        {
            s.append("\nlocal interfaces = ");
            s.append(IceUtilInternal.StringUtil.joinString(intfs, ", "));
        }
        return s.toString();
    }

    int effectivePort()
    {
        return _addr.getPort();
    }

    AcceptorI(EndpointI endpoint, Instance instance, String adapterName, String host, int port)
    {
        _endpoint = endpoint;
        _instance = instance;
        _adapterName = adapterName;
        _backlog = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.Backlog", 511);

        try
        {
            _fd = IceInternal.Network.createTcpServerSocket();
            IceInternal.Network.setBlock(_fd, false);
            IceInternal.Network.setTcpBufSize(_fd, _instance);
            if(!System.getProperty("os.name").startsWith("Windows"))
            {
                //
                // Enable SO_REUSEADDR on Unix platforms to allow
                // re-using the socket even if it's in the TIME_WAIT
                // state. On Windows, this doesn't appear to be
                // necessary and enabling SO_REUSEADDR would actually
                // not be a good thing since it allows a second
                // process to bind to an address even it's already
                // bound by another process.
                //
                // TODO: using SO_EXCLUSIVEADDRUSE on Windows would
                // probably be better but it's only supported by recent
                // Windows versions (XP SP2, Windows Server 2003).
                //
                IceInternal.Network.setReuseAddress(_fd, true);
            }
            _addr = IceInternal.Network.getAddressForServer(host, port, _instance.protocolSupport(),
                                                            _instance.preferIPv6());
        }
        catch(RuntimeException ex)
        {
            _fd = null;
            throw ex;
        }
    }

    @Override
    protected synchronized void finalize()
        throws Throwable
    {
        try
        {
            IceUtilInternal.Assert.FinalizerAssert(_fd == null);
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    private EndpointI _endpoint;
    private Instance _instance;
    private String _adapterName;
    private java.nio.channels.ServerSocketChannel _fd;
    private int _backlog;
    private java.net.InetSocketAddress _addr;
}

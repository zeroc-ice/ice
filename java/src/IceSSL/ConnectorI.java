// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class ConnectorI implements IceInternal.Connector
{
    public IceInternal.Transceiver
    connect()
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

        if(_instance.networkTraceLevel() >= 2)
        {
            String s = "trying to establish ssl connection to " + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        try
        {
            java.nio.channels.SocketChannel fd = IceInternal.Network.createTcpSocket();
            IceInternal.Network.setBlock(fd, false);
            IceInternal.Network.setTcpBufSize(fd, _instance.communicator().getProperties(), _logger);
            final java.net.InetSocketAddress addr = _proxy != null ? _proxy.getAddress() : _addr;
            IceInternal.Network.doConnect(fd, addr);
            try
            {
                javax.net.ssl.SSLEngine engine = _instance.createSSLEngine(false, _addr);
                return new TransceiverI(_instance, engine, fd, _proxy, _host, _addr);
            }
            catch(RuntimeException ex)
            {
                IceInternal.Network.closeSocketNoThrow(fd);
                throw ex;
            }
        }
        catch(Ice.LocalException ex)
        {
            if(_instance.networkTraceLevel() >= 2)
            {
                String s = "failed to establish ssl connection to " + toString() + "\n" + ex;
                _logger.trace(_instance.networkTraceCategory(), s);
            }
            throw ex;
        }
    }

    public short
    type()
    {
        return EndpointType.value;
    }

    public String
    toString()
    {
        return IceInternal.Network.addrToString(_proxy == null ? _addr : _proxy.getAddress());
    }

    public int
    hashCode()
    {
        return _hashCode;
    }

    //
    // Only for use by EndpointI.
    //
    ConnectorI(Instance instance, String host, java.net.InetSocketAddress addr, IceInternal.NetworkProxy proxy,
               int timeout, String connectionId)
    {
        _instance = instance;
        _logger = instance.communicator().getLogger();
        _host = host;
        _addr = addr;
        _proxy = proxy;
        _timeout = timeout;
        _connectionId = connectionId;

        _hashCode = 5381;
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _addr.getAddress().getHostAddress());
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _addr.getPort());
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _timeout);
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _connectionId);
    }

    public boolean
    equals(java.lang.Object obj)
    {
        if(!(obj instanceof ConnectorI))
        {
            return false;
        }

        if(this == obj)
        {
            return true;
        }

        ConnectorI p = (ConnectorI)obj;
        if(_timeout != p._timeout)
        {
            return false;
        }

        if(!_connectionId.equals(p._connectionId))
        {
            return false;
        }

        return IceInternal.Network.compareAddress(_addr, p._addr) == 0;
    }

    private Instance _instance;
    private Ice.Logger _logger;
    private String _host;
    private java.net.InetSocketAddress _addr;
    private IceInternal.NetworkProxy _proxy;
    private int _timeout;
    private String _connectionId;
    private int _hashCode;
}

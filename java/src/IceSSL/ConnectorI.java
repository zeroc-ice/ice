// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
            boolean connected = IceInternal.Network.doConnect(fd, _addr);
            try
            {
                javax.net.ssl.SSLEngine engine = _instance.createSSLEngine(false, _addr);
                return new TransceiverI(_instance, engine, fd, _host, connected, false, "", _addr);
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
        return IceInternal.Network.addrToString(_addr);
    }

    public int
    hashCode()
    {
        return _hashCode;
    }

    //
    // Only for use by EndpointI.
    //
    ConnectorI(Instance instance, String host, java.net.InetSocketAddress addr, int timeout, 
               Ice.ProtocolVersion protocol, Ice.EncodingVersion encoding, String connectionId)
    {
        _instance = instance;
        _logger = instance.communicator().getLogger();
        _host = host;
        _addr = addr;
        _timeout = timeout;
        _protocol = protocol;
        _encoding = encoding;
        _connectionId = connectionId;

        _hashCode = _addr.getAddress().getHostAddress().hashCode();
        _hashCode = 5 * _hashCode + _addr.getPort();
        _hashCode = 5 * _hashCode + _timeout;
        _hashCode = 5 * _hashCode + _protocol.hashCode();
        _hashCode = 5 * _hashCode + _encoding.hashCode();
        _hashCode = 5 * _hashCode + _connectionId.hashCode();
    }

    public boolean
    equals(java.lang.Object obj)
    {
        ConnectorI p = null;

        try
        {
            p = (ConnectorI)obj;
        }
        catch(ClassCastException ex)
        {
            return false;
        }

        if(this == p)
        {
            return true;
        }

        if(_timeout != p._timeout)
        {
            return false;
        }

        if(!_protocol.equals(p._protocol))
        {
            return false;
        }

        if(!_encoding.equals(p._encoding))
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
    private int _timeout;
    private Ice.ProtocolVersion _protocol;
    private Ice.EncodingVersion _encoding;
    private String _connectionId;
    private int _hashCode;
}

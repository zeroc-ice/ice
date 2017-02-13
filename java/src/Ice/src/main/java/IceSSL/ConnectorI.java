// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class ConnectorI implements IceInternal.Connector
{
    @Override
    public IceInternal.Transceiver connect()
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

        IceInternal.StreamSocket stream = new IceInternal.StreamSocket(_instance, _proxy, _addr, _sourceAddr);
        try
        {
            javax.net.ssl.SSLEngine engine = _instance.createSSLEngine(false, _addr);
            return new TransceiverI(_instance, engine, stream, _host, false);
        }
        catch(RuntimeException ex)
        {
            stream.close();
            throw ex;
        }
    }

    @Override
    public short type()
    {
        return _instance.type();
    }

    @Override
    public String toString()
    {
        return IceInternal.Network.addrToString(_proxy == null ? _addr : _proxy.getAddress());
    }

    @Override
    public int hashCode()
    {
        return _hashCode;
    }

    //
    // Only for use by EndpointI.
    //
    ConnectorI(Instance instance, String host, java.net.InetSocketAddress addr, IceInternal.NetworkProxy proxy,
               java.net.InetSocketAddress sourceAddr, int timeout, String connectionId)
    {
        _instance = instance;
        _host = host;
        _addr = addr;
        _sourceAddr = sourceAddr;
        _proxy = proxy;
        _timeout = timeout;
        _connectionId = connectionId;

        _hashCode = 5381;
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _addr.getAddress().getHostAddress());
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _addr.getPort());
        if(_sourceAddr != null)
        {
            _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _sourceAddr.getAddress().getHostAddress());
        }
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _timeout);
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _connectionId);
    }

    @Override
    public boolean equals(java.lang.Object obj)
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

        if(IceInternal.Network.compareAddress(_sourceAddr, p._sourceAddr) != 0)
        {
            return false;
        }

        return IceInternal.Network.compareAddress(_addr, p._addr) == 0;
    }

    private Instance _instance;
    private String _host;
    private java.net.InetSocketAddress _addr;
    private IceInternal.NetworkProxy _proxy;
    private java.net.InetSocketAddress _sourceAddr;
    private int _timeout;
    private String _connectionId;
    private int _hashCode;
}

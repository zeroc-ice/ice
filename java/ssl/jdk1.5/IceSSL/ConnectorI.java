// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class ConnectorI implements IceInternal.Connector, java.lang.Comparable
{
    final static short TYPE = 2;

    public IceInternal.Transceiver
    connect(int timeout)
    {
        //
        // The plugin may not be fully initialized.
        //
        if(!_instance.initialized())
        {
            Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
            ex.reason = "IceSSL: plugin is not initialized";
            throw ex;
        }

        if(_instance.networkTraceLevel() >= 2)
        {
            String s = "trying to establish ssl connection to " + toString();
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        java.nio.channels.SocketChannel fd = IceInternal.Network.createTcpSocket();
        IceInternal.Network.setBlock(fd, false);
        IceInternal.Network.setTcpBufSize(fd, _instance.communicator().getProperties(), _logger);
        IceInternal.Network.doConnect(fd, _addr, timeout);

        TransceiverI transceiver = null;
        try
        {
            javax.net.ssl.SSLEngine engine = _instance.createSSLEngine(false);

            transceiver = new TransceiverI(_instance, engine, fd, _host, false, "");
/*
            transceiver.waitForHandshake(timeout);

            //
            // Check IceSSL.VerifyPeer.
            //
            int verifyPeer =
                _instance.communicator().getProperties().getPropertyAsIntWithDefault("IceSSL.VerifyPeer", 2);
            if(verifyPeer > 0)
            {
                try
                {
                    engine.getSession().getPeerCertificates();
                }
                catch(javax.net.ssl.SSLPeerUnverifiedException ex)
                {
                    Ice.SecurityException e = new Ice.SecurityException();
                    e.reason = "IceSSL: server did not supply a certificate";
                    e.initCause(ex);
                    throw e;
                }
            }
*/

/*
            if(!ctx.verifyPeer(fd, _host, false))
            {
                Ice.SecurityException ex = new Ice.SecurityException();
                ex.reason = "IceSSL: outgoing connection rejected by certificate verifier";
                throw ex;
            }
*/
        }
        catch(RuntimeException ex)
        {
            try
            {
                fd.close();
            }
            catch(java.io.IOException e)
            {
                // Ignore.
            }
            throw ex;
        }

        if(_instance.networkTraceLevel() >= 1)
        {
            String s = "ssl connection established\n" + IceInternal.Network.fdToString(fd);
            _logger.trace(_instance.networkTraceCategory(), s);
        }

        return transceiver;
    }

    public short
    type()
    {
        return TYPE;
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
    ConnectorI(Instance instance, java.net.InetSocketAddress addr, int timeout, String connectionId)
    {
        _instance = instance;
        _logger = instance.communicator().getLogger();
        _host = addr.getHostName();
        _addr = addr;
        _timeout = timeout;
        _connectionId = connectionId;

        _hashCode = _addr.getAddress().getHostAddress().hashCode();
        _hashCode = 5 * _hashCode + _addr.getPort();
        _hashCode = 5 * _hashCode + _timeout;
        _hashCode = 5 * _hashCode + _connectionId.hashCode();
    }

    //
    // Compare connectors for sorting purposes
    //
    public boolean
    equals(java.lang.Object obj)
    {
        return compareTo(obj) == 0;
    }

    public int
    compareTo(java.lang.Object obj) // From java.lang.Comparable
    {
        ConnectorI p = null;

        try
        {
            p = (ConnectorI)obj;
        }
        catch(ClassCastException ex)
        {
            try
            {
                IceInternal.Connector c = (IceInternal.Connector)obj;
                return type() < c.type() ? -1 : 1;
            }
            catch(ClassCastException ee)
            {
                assert(false);
            }
        }

        if(this == p)
        {
            return 0;
        }

        if(_timeout < p._timeout)
        {
            return -1;
        }
        else if(p._timeout < _timeout)
        {
            return 1;
        }

        if(!_connectionId.equals(p._connectionId))
        {
            return _connectionId.compareTo(p._connectionId);
        }

        if(_timeout < p._timeout)
        {
            return -1;
        }
        else if(p._timeout < _timeout)
        {
            return 1;
        }

        return IceInternal.Network.compareAddress(_addr, p._addr);
    }

    private Instance _instance;
    private Ice.Logger _logger;
    private String _host;
    private java.net.InetSocketAddress _addr;
    private int _timeout;
    private String _connectionId;
    private int _hashCode;
}

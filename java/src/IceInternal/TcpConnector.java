// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class TcpConnector implements Connector
{
    public Transceiver
    connect(int timeout)
    {
        if(_traceLevels.network >= 2)
        {
            String s = "trying to establish tcp connection to " + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        java.nio.channels.SocketChannel fd = Network.createTcpSocket();
	Network.setBlock(fd, false);
        Network.doConnect(fd, _addr, timeout);

        if(_traceLevels.network >= 1)
        {
            String s = "tcp connection established\n" + Network.fdToString(fd);
            _logger.trace(_traceLevels.networkCat, s);
        }

        return new TcpTransceiver(_instance, fd);
    }

    public String
    toString()
    {
        return Network.addrToString(_addr);
    }

    //
    // Only for use by TcpEndpoint
    //
    TcpConnector(Instance instance, String host, int port)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();

        _addr = Network.getAddress(host, port);
    }

    private Instance _instance;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private java.net.InetSocketAddress _addr;
}

// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public class OutgoingConnectionFactory
{
    //
    // Only for use by Instance
    //
    OutgoingConnectionFactory(Instance instance)
    {
        _instance = instance;
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_instance == null);

        super.finalize();
    }

    public synchronized void
    destroy()
    {
        if (_instance == null)
        {
            return;
        }

        java.util.Iterator p = _connections.values().iterator();
        while (p.hasNext())
        {
            Connection connection = (Connection)p.next();
            connection.destroy(Connection.CommunicatorDestroyed);
        }
        _connections.clear();
        _instance = null;
    }

    public synchronized Connection
    create(Endpoint[] endpoints)
    {
        if (_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(endpoints.length > 0);

        //
        // First reap destroyed connections
        //
        java.util.Iterator p = _connections.values().iterator();
        while (p.hasNext())
        {
            Connection connection = (Connection)p.next();
            if (connection.destroyed())
            {
                p.remove();
            }
        }

        //
        // Search for existing connections
        //
        for (int i = 0; i < endpoints.length; i++)
        {
            Connection connection = (Connection)_connections.get(endpoints[i]);
            if (connection != null)
            {
                return connection;
            }
        }

        //
        // No connections exist, try to create one
        //
        TraceLevels traceLevels = _instance.traceLevels();
        Ice.Logger logger = _instance.logger();

        Connection connection = null;
        Ice.LocalException exception = null;
        for (int i = 0; i < endpoints.length; i++)
        {
            try
            {
                Transceiver transceiver = endpoints[i].clientTransceiver();
                if (transceiver == null)
                {
                    Connector connector = endpoints[i].connector();
                    assert(connector != null);
                    transceiver = connector.connect(endpoints[i].timeout());
                    assert(transceiver != null);
                }
                connection = new Connection(_instance, transceiver,
                                            endpoints[i], null);
                connection.activate();
                _connections.put(endpoints[i], connection);
                break;
            }
            catch (Ice.SocketException ex)
            {
                exception = ex;
            }
            /* TODO: SSL
            catch (IceSecurity.SecurityException ex)
            {
                exception = ex;
            }
            */
            catch (Ice.DNSException ex)
            {
                exception = ex;
            }
            catch (Ice.TimeoutException ex)
            {
                exception = ex;
            }

            if (traceLevels.retry >= 2)
            {
                StringBuffer s = new StringBuffer();
                s.append("connection to endpoint failed");
                if (i < endpoints.length - 1)
                {
                    s.append(", trying next endpoint\n");
                }
                else
                {
                    s.append(" and no more endpoints to try\n");
                }
                s.append(exception.toString());
                logger.trace(traceLevels.retryCat, s.toString());
            }
        }

        if (connection == null)
        {
            assert(exception != null);
            throw exception;
        }

        return connection;
    }

    private Instance _instance;
    private java.util.HashMap _connections = new java.util.HashMap();
}

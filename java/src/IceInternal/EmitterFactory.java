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

public final class EmitterFactory
{
    public synchronized Emitter
    create(Endpoint[] endpoints)
    {
        if (_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(endpoints.length > 0);

        //
        // First reap destroyed emitters
        //
        java.util.Iterator p = _emitters.values().iterator();
        while (p.hasNext())
        {
            Emitter emitter = (Emitter)p.next();
            if (emitter.destroyed())
            {
                p.remove();
            }
        }

        for (int i = 0; i < endpoints.length; i++)
        {
            Emitter emitter = (Emitter)_emitters.get(endpoints[i]);
            if (emitter != null)
            {
                return emitter;
            }
        }

        //
        // No emitters exist, try to create one
        //
        TraceLevels traceLevels = _instance.traceLevels();
        Ice.Logger logger = _instance.logger();

        Emitter emitter = null;
        Ice.LocalException exception = null;
        for (int i = 0; i < endpoints.length; i++)
        {
            try
            {
                Transceiver transceiver =
                    endpoints[i].clientTransceiver(_instance);
                if (transceiver == null)
                {
                    Connector connector = endpoints[i].connector(_instance);
                    assert(connector != null);
                    transceiver = connector.connect(endpoints[i].timeout());
                    assert(transceiver != null);
                }
                emitter = new Emitter(_instance, transceiver, endpoints[i]);
                _emitters.put(endpoints[i], emitter);
                break;
            }
            catch (Ice.SocketException ex)
            {
                exception = ex;
            }
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

        if (emitter == null)
        {
            assert(exception != null);
            throw exception;
        }

        return emitter;
    }

    //
    // For use by Instance
    //
    EmitterFactory(Instance instance)
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

    synchronized void
    destroy()
    {
        if (_instance == null)
        {
            return;
        }

        java.util.Iterator p = _emitters.values().iterator();
        while (p.hasNext())
        {
            Emitter emitter = (Emitter)p.next();
            emitter.destroy();
        }
        _emitters.clear();
    }

    private Instance _instance;
    private java.util.HashMap _emitters = new java.util.HashMap();
}

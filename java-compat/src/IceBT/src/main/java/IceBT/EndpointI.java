// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceBT;

import android.bluetooth.BluetoothAdapter;
import java.util.UUID;

final class EndpointI extends IceInternal.EndpointI
{
    public EndpointI(Instance instance, String addr, String uuid, String name, int channel, int timeout,
                     String connectionId, boolean compress)
    {
        _instance = instance;
        _addr = addr;
        _uuid = uuid;
        _name = name;
        _channel = channel;
        _timeout = timeout;
        _connectionId = connectionId;
        _compress = compress;
        hashInit();
    }

    public EndpointI(Instance instance)
    {
        _instance = instance;
        _addr = "";
        _uuid = "";
        _name = "";
        _channel = 0;
        _timeout = instance.defaultTimeout();
        _connectionId = "";
        _compress = false;
    }

    public EndpointI(Instance instance, Ice.InputStream s)
    {
        _instance = instance;

        //
        // _name and _channel are not marshaled.
        //
        _name = "";
        _channel = 0;
        _connectionId = "";
        _addr = s.readString().toUpperCase();
        _uuid = s.readString();
        _timeout = s.readInt();
        _compress = s.readBool();
        hashInit();
    }

    @Override
    public void streamWriteImpl(Ice.OutputStream s)
    {
        //
        // _name and _channel are not marshaled.
        //
        s.writeString(_addr);
        s.writeString(_uuid);
        s.writeInt(_timeout);
        s.writeBool(_compress);
    }

    @Override
    public short type()
    {
        return _instance.type();
    }

    @Override
    public String protocol()
    {
        return _instance.protocol();
    }

    @Override
    public int timeout()
    {
        return _timeout;
    }

    @Override
    public IceInternal.EndpointI timeout(int timeout)
    {
        if(timeout == _timeout)
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _addr, _uuid, _name, _channel, timeout, _connectionId, _compress);
        }
    }

    @Override
    public String connectionId()
    {
        return _connectionId;
    }

    @Override
    public IceInternal.EndpointI connectionId(String connectionId)
    {
        if(connectionId.equals(_connectionId))
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _addr, _uuid, _name, _channel, _timeout, connectionId, _compress);
        }
    }

    @Override
    public boolean compress()
    {
        return _compress;
    }

    @Override
    public IceInternal.EndpointI compress(boolean compress)
    {
        if(compress == _compress)
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _addr, _uuid, _name, _channel, _timeout, _connectionId, compress);
        }
    }

    @Override
    public boolean datagram()
    {
        return false;
    }

    @Override
    public boolean secure()
    {
        return _instance.secure();
    }

    @Override
    public IceInternal.Transceiver transceiver()
    {
        return null;
    }

    @Override
    public void connectors_async(Ice.EndpointSelectionType selType, IceInternal.EndpointI_connectors callback)
    {
        java.util.List<IceInternal.Connector> conns = new java.util.ArrayList<IceInternal.Connector>();
        conns.add(new ConnectorI(_instance, _addr, _uuid, _timeout, _connectionId));
        callback.connectors(conns);
    }

    @Override
    public IceInternal.Acceptor acceptor(String adapterName)
    {
        return new AcceptorI(this, _instance, adapterName, _uuid, _name);
    }

    @Override
    public java.util.List<IceInternal.EndpointI> expandIfWildcard()
    {
        java.util.List<IceInternal.EndpointI> endps = new java.util.ArrayList<IceInternal.EndpointI>();
        if(_addr.isEmpty())
        {
            //
            // Starting in Android 6 (API 23), BluetoothAdapter.getAddress() returns a bogus constant value.
            //
            String addr = BluetoothAdapter.getDefaultAdapter().getAddress();
            endps.add(new EndpointI(_instance, addr, _uuid, _name, _channel, _timeout, _connectionId, _compress));
        }
        else
        {
            endps.add(this);
        }
        return endps;
    }

    @Override
    public java.util.List<IceInternal.EndpointI> expandHost(Ice.Holder<IceInternal.EndpointI> publish)
    {
        publish.value = null;
        java.util.List<IceInternal.EndpointI> endps = new java.util.ArrayList<IceInternal.EndpointI>();
        endps.add(this);
        return endps;
    }

    @Override
    public boolean equivalent(IceInternal.EndpointI endpoint)
    {
        if(!(endpoint instanceof EndpointI))
        {
            return false;
        }
        EndpointI btEndpointI = (EndpointI)endpoint;
        return btEndpointI.type() == type() && btEndpointI._addr.equals(_addr) && btEndpointI._uuid.equals(_uuid) &&
            btEndpointI._channel == _channel;
    }

    @Override
    public String options()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        String s = "";

        if(!_addr.isEmpty())
        {
            s += " -a ";
            boolean addQuote = _addr.indexOf(':') != -1;
            if(addQuote)
            {
                s += "\"";
            }
            s += _addr;
            if(addQuote)
            {
                s += "\"";
            }
        }

        if(!_uuid.isEmpty())
        {
            s += " -u ";
            boolean addQuote = _uuid.indexOf(':') != -1;
            if(addQuote)
            {
                s += "\"";
            }
            s += _uuid;
            if(addQuote)
            {
                s += "\"";
            }
        }

        if(_channel > 0)
        {
            s += " -c " + _channel;
        }

        if(_timeout == -1)
        {
            s += " -t infinite";
        }
        else
        {
            s += " -t " + _timeout;
        }

        if(_compress)
        {
            s += " -z";
        }

        return s;
    }

    public void initWithOptions(java.util.ArrayList<String> args, boolean oaEndpoint)
    {
        super.initWithOptions(args);

        if(_addr.isEmpty())
        {
            _addr = _instance.defaultHost();
            if(_addr == null)
            {
                _addr = "";
            }
        }
        else if(_addr.equals("*"))
        {
            if(oaEndpoint)
            {
                _addr = "";
            }
            else
            {
                throw new Ice.EndpointParseException("`-a *' not valid for proxy endpoint `" + toString() + "'");
            }
        }

        if(_name.isEmpty())
        {
            _name = "Ice Service";
        }

        if(_uuid.isEmpty())
        {
            if(oaEndpoint)
            {
                //
                // Generate a UUID for object adapters that don't specify one.
                //
                _uuid = UUID.randomUUID().toString();
            }
            else
            {
                throw new Ice.EndpointParseException("a UUID must be specified using the -u option");
            }
        }

        hashInit();
    }

    @Override
    public Ice.EndpointInfo getInfo()
    {
        EndpointInfo info = new EndpointInfo()
        {
            @Override
            public short type()
            {
                return EndpointI.this.type();
            }

            @Override
            public boolean datagram()
            {
                return EndpointI.this.datagram();
            }

            @Override
            public boolean secure()
            {
                return EndpointI.this.secure();
            }
        };
        info.addr = _addr;
        info.uuid = _uuid;
        return info;
    }

    @Override
    public int compareTo(IceInternal.EndpointI obj) // From java.lang.Comparable
    {
        if(!(obj instanceof EndpointI))
        {
            return type() < obj.type() ? -1 : 1;
        }

        EndpointI p = (EndpointI)obj;
        if(this == p)
        {
            return 0;
        }

        int v = _addr.compareTo(p._addr);
        if(v != 0)
        {
            return v;
        }

        v = _uuid.compareTo(p._uuid);
        if(v != 0)
        {
            return v;
        }

        if(_channel < p._channel)
        {
            return -1;
        }
        else if(p._channel < _channel)
        {
            return 1;
        }

        if(_timeout < p._timeout)
        {
            return -1;
        }
        else if(p._timeout < _timeout)
        {
            return 1;
        }

        v = _connectionId.compareTo(p._connectionId);
        if(v != 0)
        {
            return v;
        }

        if(!_compress && p._compress)
        {
            return -1;
        }
        else if(!p._compress && _compress)
        {
            return 1;
        }

        return 0;
    }

    @Override
    public int hashCode()
    {
        return _hashValue;
    }

    @Override
    protected boolean checkOption(String option, String argument, String endpoint)
    {
        if(super.checkOption(option, argument, endpoint))
        {
            return true;
        }

        if(option.equals("-a"))
        {
            if(argument == null)
            {
                throw new Ice.EndpointParseException("no argument provided for -a option in endpoint " + endpoint);
            }
            if(!argument.equals("*") && !BluetoothAdapter.checkBluetoothAddress(argument.toUpperCase()))
            {
                throw new Ice.EndpointParseException("invalid address provided for -a option in endpoint " + endpoint);
            }
            _addr = argument.toUpperCase(); // Android requires a hardware address to use upper case letters.
        }
        else if(option.equals("-u"))
        {
            if(argument == null)
            {
                throw new Ice.EndpointParseException("no argument provided for -u option in endpoint " + endpoint);
            }
            try
            {
                UUID.fromString(argument);
            }
            catch(IllegalArgumentException ex)
            {
                throw new Ice.EndpointParseException("invalid UUID for Bluetooth endpoint", ex);
            }
            _uuid = argument;
        }
        else if(option.equals("-c"))
        {
            if(argument == null)
            {
                throw new Ice.EndpointParseException("no argument provided for -c option in endpoint " + endpoint);
            }

            try
            {
                _channel = Integer.parseInt(argument);
            }
            catch(NumberFormatException ex)
            {
                throw new Ice.EndpointParseException("invalid channel value `" + argument +
                                                     "' in endpoint " + endpoint);
            }

            if(_channel < 0 || _channel > 30) // RFCOMM channel limit is 30
            {
                throw new Ice.EndpointParseException("channel value `" + argument +
                                                     "' out of range in endpoint " + endpoint);
            }
        }
        else if(option.equals("-t"))
        {
            if(argument == null)
            {
                throw new Ice.EndpointParseException("no argument provided for -t option in endpoint " + endpoint);
            }

            if(argument.equals("infinite"))
            {
                _timeout = -1;
            }
            else
            {
                try
                {
                    _timeout = Integer.parseInt(argument);
                    if(_timeout < 1)
                    {
                        throw new Ice.EndpointParseException("invalid timeout value `" + argument + "' in endpoint " +
                                                             endpoint);
                    }
                }
                catch(NumberFormatException ex)
                {
                    throw new Ice.EndpointParseException("invalid timeout value `" + argument + "' in endpoint " +
                                                         endpoint);
                }
            }
        }
        else if(option.equals("-z"))
        {
            if(argument != null)
            {
                throw new Ice.EndpointParseException("unexpected argument `" + argument +
                                                     "' provided for -z option in " + endpoint);
            }

            _compress = true;
        }
        else if(option.equals("--name"))
        {
            if(argument == null)
            {
                throw new Ice.EndpointParseException("no argument provided for --name option in endpoint " + endpoint);
            }

            _name = argument;
        }
        else
        {
            return false;
        }
        return true;
    }

    private void hashInit()
    {
        int h = 5381;
        h = IceInternal.HashUtil.hashAdd(h, _addr);
        h = IceInternal.HashUtil.hashAdd(h, _uuid);
        h = IceInternal.HashUtil.hashAdd(h, _timeout);
        h = IceInternal.HashUtil.hashAdd(h, _connectionId);
        h = IceInternal.HashUtil.hashAdd(h, _compress);
        _hashValue = h;
    }

    private Instance _instance;
    private String _addr;
    private String _uuid;
    private String _name;
    private int _channel;
    private int _timeout;
    private String _connectionId;
    private boolean _compress;
    private int _hashValue;
}

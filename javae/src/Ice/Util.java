// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class Util
{
    public static Properties
    createProperties()
    {
        return new Properties();
    }

    public static Properties
    createProperties(StringSeqHolder args)
    {
        return new Properties(args, null);
    }

    public static Properties
    createProperties(StringSeqHolder args, Properties defaults)
    {
        return new Properties(args, defaults);
    }

    public static Properties
    createProperties(String[] args)
    {
	StringSeqHolder argsH = new StringSeqHolder(args);
        return createProperties(argsH);
    }

    public static Properties
    createProperties(String[] args, Properties defaults)
    {
	StringSeqHolder argsH = new StringSeqHolder(args);
        return createProperties(argsH, defaults);
    }

    public static Communicator
    initialize(StringSeqHolder args)
    {
        InitializationData initData = new InitializationData();
	return initialize(args, initData);
    }

    public static Communicator
    initialize(String[] args)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
	return initialize(argsH);
    }

    public static Communicator
    initialize(StringSeqHolder args, InitializationData initData)
    {
	if(initData == null)
	{
	    initData = new InitializationData();
	}
	else
	{
	    initData = (InitializationData)initData.ice_clone();
	}
	initData.properties = createProperties(args, initData.properties);

        Communicator result = new Communicator(initData);
        result.finishSetup(args);
        return result;
    }

    public static Communicator
    initialize(String[] args, InitializationData initData)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
	return initialize(argsH, initData);
    }

    public static Communicator
    initialize(InitializationData initData)
    {
	if(initData == null)
	{
	    initData = new InitializationData();
	}
	else
	{
	    initData = (InitializationData)initData.ice_clone();
	}

	Communicator result = new Communicator(initData);
        result.finishSetup(new StringSeqHolder(new String[0]));
        return result;
    }

    public static Communicator
    initialize()
    {
        return initialize(new InitializationData());
    }

    public static IceInternal.Instance
    getInstance(Communicator communicator)
    {
        return communicator.getInstance();
    }

    public static synchronized String
    generateUUID()
    {
	if(_localAddress == null)
        {
            byte[] ip = IceInternal.Network.getLocalAddress();
            _localAddress = "";
            for(int i = 0; i < ip.length; i++)
            {
                if(i > 0)
                {
                    _localAddress += ":";
                }
                int n = ip[i] < 0 ? ip[i] + 256 : ip[i];
                _localAddress += Integer.toHexString(n);
            }
        }
	
        return _localAddress + ":" + IceUtil.UUID.create();
    }

    public static int
    proxyIdentityCompare(ObjectPrx lhs, ObjectPrx rhs)
    {
	if(lhs == null && rhs == null)
	{
	    return 0;
	}
	else if(lhs == null && rhs != null)
	{
	    return -1;
	}
	else if(lhs != null && rhs == null)
	{
	    return 1;
	}
	else
	{
	    Identity lhsIdentity = lhs.ice_getIdentity();
	    Identity rhsIdentity = rhs.ice_getIdentity();
	    int n;
	    if((n = lhsIdentity.name.compareTo(rhsIdentity.name)) != 0)
	    {
		return n;
	    }
	    return lhsIdentity.category.compareTo(rhsIdentity.category);
	}
    }

    public static int
    proxyIdentityAndFacetCompare(ObjectPrx lhs, ObjectPrx rhs)
    {
	if(lhs == null && rhs == null)
	{
	    return 0;
	}
	else if(lhs == null && rhs != null)
	{
	    return -1;
	}
	else if(lhs != null && rhs == null)
	{
	    return 1;
	}
	else
	{
	    Identity lhsIdentity = lhs.ice_getIdentity();
	    Identity rhsIdentity = rhs.ice_getIdentity();
	    int n;
	    if((n = lhsIdentity.name.compareTo(rhsIdentity.name)) != 0)
	    {
		return n;
	    }
	    if((n = lhsIdentity.category.compareTo(rhsIdentity.category)) != 0)
	    {
		return n;
	    }
	    
	    String lhsFacet = lhs.ice_getFacet();
	    String rhsFacet = rhs.ice_getFacet();
            if(lhsFacet == null && rhsFacet == null)
            {
                return 0;
            }
            else if(lhsFacet == null)
            {
                return -1;
            }
            else if(rhsFacet == null)
            {
                return 1;
            }
            return lhsFacet.compareTo(rhsFacet);
	}
    }

    public static void
    throwBadMagicException(byte[] badMagic)
    {
        String out = "unknown magic number: ";
        out += "0x" + Integer.toHexString((int)badMagic[0]) + ", ";
        out += "0x" + Integer.toHexString((int)badMagic[1]) + ", ";
        out += "0x" + Integer.toHexString((int)badMagic[2]) + ", ";
        out += "0x" + Integer.toHexString((int)badMagic[3]) + "";
        throw new ProtocolException(out);
    }

    public static void
    throwUnsupportedProtocolException(int badMajor, int badMinor)
    {
        String out = "unsupported protocol version: " + badMajor + "." + badMinor;
        out += "\n(can only support protocols compatible with version ";
        out += IceInternal.Protocol.protocolMajor + "." + IceInternal.Protocol.protocolMinor;
        throw new ProtocolException(out);
    }

    public static void
    throwUnsupportedEncodingException(int badMajor, int badMinor)
    {
        String out = "unsupported encoding version: " + badMajor + "." + badMinor;
        out += "\n(can only support protocols compatible with version ";
        out += IceInternal.Protocol.encodingMajor + "." + IceInternal.Protocol.encodingMinor;
        throw new ProtocolException(out);
    }

    public static void
    throwUnknownMessageException()
    {
        throw new ProtocolException("unknown message type");
    }

    public static void
    throwConnectionNotValidatedException()
    {
        throw new ProtocolException("received message over unvalidated connection");
    }

    public static void
    throwUnknownRequestIdException()
    {
        throw new ProtocolException("unknown request id");
    }

    public static void
    throwUnknownReplyStatusException()
    {
        throw new ProtocolException("unknown reply status");
    }

    public static void
    throwIllegalMessageSizeException()
    {
        throw new ProtocolException("illegal message size");
    }

    public static void
    throwUnmarshalOutOfBoundsException()
    {
        throw new MarshalException("out of bounds during unmarshaling");
    }

    public static void
    throwNegativeSizeException()
    {
        throw new MarshalException("negative size for sequence, dictionary, etc.");
    }

    public static void
    throwProxyUnmarshalException()
    {
        throw new MarshalException("inconsistent proxy data during unmarshaling");
    }

    private static String _localAddress = null;
}

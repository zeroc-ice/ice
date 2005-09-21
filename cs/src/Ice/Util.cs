// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    using IceInternal;
    using System.Runtime.InteropServices;
    using System.Diagnostics;
    using System.IO;
    using System.Text;

    public sealed class Util
    {
	public static Properties createProperties()
	{
	    return new PropertiesI();
	}

	public static Properties createProperties(ref string[] args)
	{
	    return new PropertiesI(ref args);
	}
	
	public static Properties getDefaultProperties()
	{
	    lock(_defaultPropertiesMutex)
	    {
		if(_defaultProperties == null)
		{
		    _defaultProperties = createProperties();
		}
		return _defaultProperties;
	    }
	}
	
	public static Properties getDefaultProperties(ref string[] args)
	{
	    lock(_defaultPropertiesMutex)
	    {
		if(_defaultProperties == null)
		{
		    _defaultProperties = createProperties(ref args);
		}
		return _defaultProperties;
	    }
	}

	public static Communicator initialize(ref string[] args)
	{
	    Properties defaultProperties = getDefaultProperties(ref args);
	    return initializeWithProperties(ref args, defaultProperties);
	}
	
	public static Communicator initializeWithProperties(ref string[] args, Properties properties)
	{
	    args = properties.parseIceCommandLineOptions(args);

	    CommunicatorI result = new CommunicatorI(properties);
	    result.finishSetup(ref args);
	    return result;
	}
	
	public static IceInternal.Instance getInstance(Communicator communicator)
	{
	    CommunicatorI p = (CommunicatorI) communicator;
	    return p.getInstance();
	}
	
	public static Identity stringToIdentity(string s)
	{
	    Identity ident = new Identity();
	    
	    //
	    // Find unescaped separator.
	    //
	    int slash = -1, pos = 0;
	    while((pos = s.IndexOf((System.Char) '/', pos)) != -1)
	    {
		if(pos == 0 || s[pos - 1] != '\\')
		{
		    if(slash == -1)
		    {
			slash = pos;
		    }
		    else
		    {
			//
			// Extra unescaped slash found.
			//
			IdentityParseException ex = new IdentityParseException();
			ex.str = s;
			throw ex;
		    }
		}
		pos++;
	    }
	    
	    if(slash == -1)
	    {
		if(!IceUtil.StringUtil.unescapeString(s, 0, s.Length, out ident.name))
		{
		    IdentityParseException ex = new IdentityParseException();
		    ex.str = s;
		    throw ex;
		}
		ident.category = "";
	    }
	    else
	    {
		if(!IceUtil.StringUtil.unescapeString(s, 0, slash, out ident.category))
		{
		    IdentityParseException ex = new IdentityParseException();
		    ex.str = s;
		    throw ex;
		}
		if(slash + 1 < s.Length)
		{
		    if(!IceUtil.StringUtil.unescapeString(s, slash + 1, s.Length, out ident.name))
		    {
			IdentityParseException ex = new IdentityParseException();
			ex.str = s;
			throw ex;
		    }
		}
		else
		{
		    ident.name = "";
		}
	    }
	    
	    return ident;
	}
	
	public static string identityToString(Identity ident)
	{
	    if(ident.category.Length == 0)
	    {
		return IceUtil.StringUtil.escapeString(ident.name, "/");
	    }
	    else
	    {
		return IceUtil.StringUtil.escapeString(ident.category, "/") + '/' + IceUtil.StringUtil.escapeString(ident.name, "/");
	    }
	}
	
	[StructLayout(LayoutKind.Sequential)]
	private struct UUID
	{
	    public ulong Data1;
	    public ushort Data2;
	    public ushort Data3;
	    [MarshalAs(UnmanagedType.ByValTStr, SizeConst=8)]
	    public string Data4;
	}

	[DllImport("rpcrt4.dll")]
	private static extern int UuidCreate(ref UUID uuid); 

	[DllImport("rpcrt4.dll")]
	private static extern int UuidToString(ref UUID uuid, ref System.IntPtr str);

	[DllImport("rpcrt4.dll")]
	private static extern int RpcStringFree(ref System.IntPtr str);

	private const int RPC_S_OK = 0;
	private const int RPC_S_OUT_OF_MEMORY = 14;
	private const int RPC_S_UUID_LOCAL_ONLY = 1824;

	public static string generateUUID()
	{
	    if(AssemblyUtil.platform_ == AssemblyUtil.Platform.Windows)
	    {
	        //
		// Under Windows, with both .NET and Mono, there is no /dev/urandom.
		//
		int rc;
		UUID uuid = new UUID();
		rc = UuidCreate(ref uuid);
		if(rc != RPC_S_OK && rc != RPC_S_UUID_LOCAL_ONLY)
		{
		    Ice.SyscallException ex = new Ice.SyscallException("UuidCreate() failed");
		    ex.error = rc;
		    throw ex;
		}
		System.IntPtr str = new System.IntPtr();
		rc = UuidToString(ref uuid, ref str);
		switch(rc)
		{
		    case RPC_S_OK:
		    {
			break;
		    }
		    case RPC_S_OUT_OF_MEMORY:
		    {
			throw new Ice.MemoryLimitException("No memory for UUID string");
		    }
		    default:
		    {
			Ice.SyscallException ex = new Ice.SyscallException("UuidToString() failed");
			ex.error = rc;
			throw ex;
		    }
		}
		string result;
		try 
		{
		    result = Marshal.PtrToStringAnsi(str);
		}
		catch(System.Exception ex)
		{
		    throw new Ice.SyscallException(ex);
		}
		if((rc = RpcStringFree(ref str)) != RPC_S_OK)
		{
		    Ice.SyscallException ex = new Ice.SyscallException("Cannot deallocate UUID");
		    ex.error = rc;
		    throw ex;
		}
		return result;
	    }
	    else
	    {
		//
		// On Linux, /dev/random, even when used in blocking mode, sometimes 
		// fails or returns fewer bytes.
		// Maybe we should use a combination of version 4 UUIDs (with /dev/random),
		// and version 1 UUIDs (MAC address + time), when /dev/random is exhausted?
		//
		FileStream f = File.OpenRead("/dev/urandom");

		const int uuidSize = 16;
		byte[] buf = new byte[uuidSize];

		//
		// Limit the number of attempts to 20 reads to avoid an infinite loop.
		//
		int reads = 0;
		int index = 0;
		while(reads < 20 && index != uuidSize)
		{
		    int bytesRead = f.Read(buf, index, uuidSize - index);
		    index += bytesRead;
		    reads++;
		}
		f.Close();
		if(index != uuidSize)
		{
		     throw new System.ApplicationException("generateUUID(): could not get 16 random bytes");
		}

		//
		// Adjust the bits that say "version 4" UUUID
		//
		buf[6] &= 0x0F;
		buf[6] |= (4 << 4);
		buf[8] &= 0x3F;
		buf[8] |= 0x80;

		StringBuilder sb = new StringBuilder();
		for(int i = 0; i < 4; ++i)
		{
		    sb.AppendFormat("{0:X2}", buf[i]);
		}
		sb.Append("-");
		for(int i = 4; i < 6; ++i)
		{
		    sb.AppendFormat("{0:X2}", buf[i]);
		}
		sb.Append("-");
		for(int i = 6; i < 8; ++i)
		{
		    sb.AppendFormat("{0:X2}", buf[i]);
		}
		sb.Append("-");
		for(int i = 8; i < 10; ++i)
		{
		    sb.AppendFormat("{0:X2}", buf[i]);
		}
		sb.Append("-");
		for(int i = 10; i < 16; ++i)
		{
		    sb.AppendFormat("{0:X2}", buf[i]);
		}
		return sb.ToString();
	    }
	}
	
	public static int proxyIdentityCompare(ObjectPrx lhs, ObjectPrx rhs)
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
		if((n = lhsIdentity.name.CompareTo(rhsIdentity.name)) != 0)
		{
		    return n;
		}
		return lhsIdentity.category.CompareTo(rhsIdentity.category);
	    }
	}
	
	public static int proxyIdentityAndFacetCompare(ObjectPrx lhs, ObjectPrx rhs)
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
		if((n = lhsIdentity.name.CompareTo(rhsIdentity.name)) != 0)
		{
		    return n;
		}
		if((n = lhsIdentity.category.CompareTo(rhsIdentity.category)) != 0)
		{
		    return n;
		}

		string lhsFacet = lhs.ice_getFacet();
		string rhsFacet = rhs.ice_getFacet();
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
                else
                {
                    return lhsFacet.CompareTo(rhsFacet);
                }
	    }
	}

        public static InputStream createInputStream(Communicator communicator, byte[] bytes)
        {
            return new InputStreamI(communicator, bytes);
        }

        public static OutputStream createOutputStream(Communicator communicator)
        {
            return new OutputStreamI(communicator);
        }

	private static Properties _defaultProperties = null;
	internal static object _defaultPropertiesMutex;
	static Util()
	{
	    _defaultPropertiesMutex = new object();
	}
    }

}

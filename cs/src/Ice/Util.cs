// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************
namespace Ice
{

using System.Runtime.InteropServices;

public sealed class Util
{
    public static Properties
    getDefaultProperties()
    {
	if(_defaultProperties == null)
	{
	    _defaultProperties = createProperties();
	}
	return _defaultProperties;
    }
    
    public static Properties
    getDefaultProperties(ref string[] args)
    {
	if(_defaultProperties == null)
	{
	    _defaultProperties = createProperties(ref args);
	}
	return _defaultProperties;
    }
    
    public static Properties
    createProperties()
    {
	return new PropertiesI();
    }
    
    public static Properties
    createProperties(ref string[] args)
    {
	return new PropertiesI(ref args);
    }
    
    public static Communicator
    initialize(ref string[] args)
    {
	Properties defaultProperties = getDefaultProperties(ref args);
	CommunicatorI result = new CommunicatorI(ref args, defaultProperties);
	result.finishSetup(ref args);
	return result;
    }
    
    public static Communicator
    initializeWithProperties(ref string[] args, Properties properties)
    {
	CommunicatorI result = new CommunicatorI(ref args, properties);
	result.finishSetup(ref args);
	return result;
    }
    
    public static IceInternal.Instance
    getInstance(Communicator communicator)
    {
	CommunicatorI p = (CommunicatorI) communicator;
	return p.getInstance();
    }
    
    public static Identity
    stringToIdentity(string s)
    {
	Identity ident = new Identity();
	
	//
	// Find unescaped separator.
	//
	int slash = - 1, pos = 0;
	while((pos = s.IndexOf((System.Char) '/', pos)) != - 1)
	{
	    if(pos == 0 || s[pos - 1] != '\\')
	    {
		if(slash == - 1)
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
	
	if(slash == - 1)
	{
	    if(!IceInternal.StringUtil.decodeString(s, 0, s.Length, out ident.name))
	    {
		IdentityParseException ex = new IdentityParseException();
		ex.str = s;
		throw ex;
	    }
	    ident.category = "";
	}
	else
	{
	    if(!IceInternal.StringUtil.decodeString(s, 0, slash, out ident.category))
	    {
		IdentityParseException ex = new IdentityParseException();
		ex.str = s;
		throw ex;
	    }
	    if(slash + 1 < s.Length)
	    {
		if(!IceInternal.StringUtil.decodeString(s, slash + 1, s.Length, out ident.name))
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
    
    public static string
    identityToString(Identity ident)
    {
	if(ident.category.Length == 0)
	{
	    return IceInternal.StringUtil.encodeString(ident.name, "/");
	}
	else
	{
	    return IceInternal.StringUtil.encodeString(ident.category, "/") + '/' + IceInternal.StringUtil.encodeString(ident.name, "/");
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
    private const int RPC_S_UUID_NO_ADDRESS = 1739;

    public static string
    generateUUID()
    {
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
    
    public static int
    proxyIdentityCompare(ObjectPrx lhs, ObjectPrx rhs)
    {
	if(lhs == null && rhs == null)
	{
	    return 0;
	}
	else if(lhs == null && rhs != null)
	{
	    return - 1;
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
    
    public static int
    proxyIdentityAndFacetCompare(ObjectPrx lhs, ObjectPrx rhs)
    {
	if(lhs == null && rhs == null)
	{
	    return 0;
	}
	else if(lhs == null && rhs != null)
	{
	    return - 1;
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

	    FacetPath lhsFacet = lhs.ice_getFacet();
	    FacetPath rhsFacet = rhs.ice_getFacet();
	    for(int i = 0; i < lhsFacet.Count && i < rhsFacet.Count; ++i)
	    {
	        if((n = lhsFacet[i].CompareTo(rhsFacet[i])) != 0)
		{
		    return n;
		}
	    }

	    if(lhsFacet.Count == rhsFacet.Count)
	    {
	        return 0;
	    }
	    else if(lhsFacet.Count < rhsFacet.Count)
	    {
	        return -1;

	    }
	    else
	    {
	        return 1;
	    }
	}
    }
    
    private static Properties _defaultProperties = null;
}

}

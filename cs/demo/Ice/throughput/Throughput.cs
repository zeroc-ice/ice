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

// Ice version 1.3.0
// Generated from file `C:\cygwin\home\michi\src\icicle\demo\Ice\throughput\Throughput.ice'

using _System = System;
using _Microsoft = Microsoft;

public class seqSize
{
    public const int value = 500000;
}

public class ByteSeq : _System.Collections.CollectionBase, _System.ICloneable, Ice.SequenceBase
{
    #region Constructors

    public ByteSeq() : base()
    {
    }

    public ByteSeq(int capacity) : base()
    {
	InnerList.Capacity = capacity;
    }

    public ByteSeq(_System.Collections.ICollection __s)
    {
	InnerList.AddRange(__s);
    }

    #endregion

    #region Array copy and conversion

    public void CopyTo(byte[] __a)
    {
	InnerList.CopyTo(__a);
    }

    public void CopyTo(byte[] __a, int __i)
    {
	InnerList.CopyTo(__a, __i);
    }

    public void CopyTo(int __i, byte[] __a, int __ai, int __c)
    {
	InnerList.CopyTo(__i, __a, __ai, __c);
    }

    public byte[] ToArray()
    {
	byte[] __a = new byte[InnerList.Count];
	InnerList.CopyTo(__a, 0);
	return __a;
    }

    #endregion

    #region Indexer

    public byte this[int index]
    {
	get
	{
	    return (byte)InnerList[index];
	}
	set
	{
	    InnerList[index] = value;
	}
    }

    #endregion

    #region byte members

    public void AddRange(_System.Collections.ICollection __s)
    {
	InnerList.AddRange(__s);
    }

    #endregion

    #region ICollectionBase members

    public int Add(byte value)
    {
	return InnerList.Add(value);
    }

    public int IndexOf(byte value)
    {
	return InnerList.IndexOf(value);
    }

    public void Insert(int index, byte value)
    {
	InnerList.Insert(index, value);
    }

    public void Remove(byte value)
    {
	InnerList.Remove(value);
    }

    public bool Contains(byte value)
    {
	return InnerList.Contains(value);
    }

    #endregion

    #region Comparison operators

    public static bool operator==(ByteSeq __lhs, ByteSeq __rhs)
    {
	return Equals(__lhs, __rhs);
    }

    public static bool operator!=(ByteSeq __lhs, ByteSeq __rhs)
    {
	return !Equals(__lhs, __rhs);
    }

    #endregion

    #region ICloneable members

    public object Clone()
    {
	ByteSeq __ret = new ByteSeq();
	__ret.InnerList.AddRange(InnerList);
	return __ret;
    }

    #endregion

    #region SequenceBase members

    public void set(int index, object o)
    {
	for(int i = InnerList.Count; i <= index; ++i)
	{
	    InnerList.Add(null);
	}
	InnerList[index] = (byte)o;
    }

    #endregion

    #region Object members

    public override int GetHashCode()
    {
	int hash = 0;
	for(int i = 0; i < Count; ++i)
	{
	    hash = 5 * hash + InnerList[i].GetHashCode();
	}
	return hash;
    }

    public override bool Equals(object other)
    {
	if(object.ReferenceEquals(this, other))
	{
	    return true;
	}
	if(!(other is ByteSeq))
	{
	    return false;
	}
	if(Count != ((ByteSeq)other).Count)
	{
	    return false;
	}
	for(int __i = 0; __i < Count; ++__i)
	{
	    if(!((byte)(InnerList[__i])).Equals(((ByteSeq)other)[__i]))
	    {
		return false;
	    }
	}
	return true;
    }

    public static bool Equals(ByteSeq __lhs, ByteSeq __rhs)
    {
	return object.ReferenceEquals(__lhs, null)
	           ? object.ReferenceEquals(__rhs, null)
	           : __lhs.Equals(__rhs);
    }

    #endregion
}

public interface Throughput : Ice.Object,
			      Throughput_Operations
{
}

public interface ThroughputPrx : Ice.ObjectPrx
{
    void sendByteSeq(ByteSeq seq);
    void sendByteSeq(ByteSeq seq, Ice.Context __context);

    ByteSeq recvByteSeq();
    ByteSeq recvByteSeq(Ice.Context __context);

    ByteSeq echoByteSeq(ByteSeq seq);
    ByteSeq echoByteSeq(ByteSeq seq, Ice.Context __context);
}

public interface Throughput_Operations
{
    void sendByteSeq(ByteSeq seq, Ice.Current __current);

    ByteSeq recvByteSeq(Ice.Current __current);

    ByteSeq echoByteSeq(ByteSeq seq, Ice.Current __current);
}

public sealed class ByteSeqHelper
{
    public static void write(IceInternal.BasicStream __os, ByteSeq __v)
    {
	__os.writeByteSeq(__v.ToArray());
    }

    public static ByteSeq read(IceInternal.BasicStream __is)
    {
	ByteSeq __v = new ByteSeq();
	__v = new ByteSeq(__is.readByteSeq());
	return __v;
    }
}

public class ThroughputPrxHelper : Ice.ObjectPrxHelper, ThroughputPrx
{
    public ByteSeq echoByteSeq(ByteSeq seq)
    {
	return echoByteSeq(seq, __defaultContext());
    }

    public ByteSeq echoByteSeq(ByteSeq seq, Ice.Context __context)
    {
	int __cnt = 0;
	while(true)
	{
	    try
	    {
		__checkTwowayOnly("echoByteSeq");
		Ice.Object_Del __delBase = __getDelegate();
		Throughput_Del __del = (Throughput_Del)__delBase;
		return __del.echoByteSeq(seq, __context);
	    }
	    catch(IceInternal.NonRepeatable __ex)
	    {
		__rethrowException(__ex.get());
	    }
	    catch(Ice.LocalException __ex)
	    {
		__cnt = __handleException(__ex, __cnt);
	    }
	}
    }

    public ByteSeq recvByteSeq()
    {
	return recvByteSeq(__defaultContext());
    }

    public ByteSeq recvByteSeq(Ice.Context __context)
    {
	int __cnt = 0;
	while(true)
	{
	    try
	    {
		__checkTwowayOnly("recvByteSeq");
		Ice.Object_Del __delBase = __getDelegate();
		Throughput_Del __del = (Throughput_Del)__delBase;
		return __del.recvByteSeq(__context);
	    }
	    catch(IceInternal.NonRepeatable __ex)
	    {
		__rethrowException(__ex.get());
	    }
	    catch(Ice.LocalException __ex)
	    {
		__cnt = __handleException(__ex, __cnt);
	    }
	}
    }

    public void sendByteSeq(ByteSeq seq)
    {
	sendByteSeq(seq, __defaultContext());
    }

    public void sendByteSeq(ByteSeq seq, Ice.Context __context)
    {
	int __cnt = 0;
	while(true)
	{
	    try
	    {
		Ice.Object_Del __delBase = __getDelegate();
		Throughput_Del __del = (Throughput_Del)__delBase;
		__del.sendByteSeq(seq, __context);
		return;
	    }
	    catch(IceInternal.NonRepeatable __ex)
	    {
		__rethrowException(__ex.get());
	    }
	    catch(Ice.LocalException __ex)
	    {
		__cnt = __handleException(__ex, __cnt);
	    }
	}
    }

    protected override Ice.Object_DelM __createDelegateM()
    {
	return new Throughput_DelM();
    }

    protected override Ice.Object_DelD __createDelegateD()
    {
	return new Throughput_DelD();
    }

    public static ThroughputPrx checkedCast(Ice.ObjectPrx b)
    {
	if(b == null)
	{
	    return null;
	}
	if(b is ThroughputPrx)
	{
	    return (ThroughputPrx)b;
	}
	if(b.ice_isA("::Throughput"))
	{
	    ThroughputPrxHelper h = new ThroughputPrxHelper();
	    h.__copyFrom(b);
	    return h;
	}
	return null;
    }

    public static ThroughputPrx checkedCast(Ice.ObjectPrx b, string f)
    {
	if(b == null)
	{
	    return null;
	}
	Ice.ObjectPrx bb = b.ice_appendFacet(f);
	try
	{
	    if(bb.ice_isA("::Throughput"))
	    {
		ThroughputPrxHelper h = new ThroughputPrxHelper();
		h.__copyFrom(bb);
		return h;
	    }
	}
	catch(Ice.FacetNotExistException)
	{
	}
	return null;
    }

    public static ThroughputPrx uncheckedCast(Ice.ObjectPrx b)
    {
	if(b == null)
	{
	    return null;
	}
	ThroughputPrxHelper h = new ThroughputPrxHelper();
	h.__copyFrom(b);
	return h;
    }

    public static ThroughputPrx uncheckedCast(Ice.ObjectPrx b, string f)
    {
	if(b == null)
	{
	    return null;
	}
	Ice.ObjectPrx bb = b.ice_appendFacet(f);
	ThroughputPrxHelper h = new ThroughputPrxHelper();
	h.__copyFrom(bb);
	return h;
    }

    public static void __write(IceInternal.BasicStream __os, ThroughputPrx __v)
    {
	__os.writeProxy(__v);
    }

    public static ThroughputPrx __read(IceInternal.BasicStream __is)
    {
	Ice.ObjectPrx proxy = __is.readProxy();
	if(proxy != null)
	{
	    ThroughputPrxHelper result = new ThroughputPrxHelper();
	    result.__copyFrom(proxy);
	    return result;
	}
	return null;
    }
}

public interface Throughput_Del : Ice.Object_Del
{
    void sendByteSeq(ByteSeq seq, Ice.Context __context);

    ByteSeq recvByteSeq(Ice.Context __context);

    ByteSeq echoByteSeq(ByteSeq seq, Ice.Context __context);
}

public sealed class Throughput_DelM : Ice.Object_DelM, Throughput_Del
{
    public ByteSeq echoByteSeq(ByteSeq seq, Ice.Context __context)
    {
	IceInternal.Outgoing __out = getOutgoing("echoByteSeq", Ice.OperationMode.Normal, __context);
	try
	{
	    IceInternal.BasicStream __os = __out.ostr();
	    IceInternal.BasicStream __is = __out.istr();
	    __os.writeByteSeq(seq.ToArray());
	    if(!__out.invoke())
	    {
		throw new Ice.UnknownUserException();
	    }
	    try
	    {
		ByteSeq __ret;
		__ret = new ByteSeq(__is.readByteSeq());
		return __ret;
	    }
	    catch(Ice.LocalException __ex)
	    {
		throw new IceInternal.NonRepeatable(__ex);
	    }
	}
	finally
	{
	    reclaimOutgoing(__out);
	}
    }

    public ByteSeq recvByteSeq(Ice.Context __context)
    {
	IceInternal.Outgoing __out = getOutgoing("recvByteSeq", Ice.OperationMode.Normal, __context);
	try
	{
	    IceInternal.BasicStream __is = __out.istr();
	    if(!__out.invoke())
	    {
		throw new Ice.UnknownUserException();
	    }
	    try
	    {
		ByteSeq __ret;
		__ret = new ByteSeq(__is.readByteSeq());
		return __ret;
	    }
	    catch(Ice.LocalException __ex)
	    {
		throw new IceInternal.NonRepeatable(__ex);
	    }
	}
	finally
	{
	    reclaimOutgoing(__out);
	}
    }

    public void sendByteSeq(ByteSeq seq, Ice.Context __context)
    {
	IceInternal.Outgoing __out = getOutgoing("sendByteSeq", Ice.OperationMode.Normal, __context);
	try
	{
	    IceInternal.BasicStream __os = __out.ostr();
	    __os.writeByteSeq(seq.ToArray());
	    if(!__out.invoke())
	    {
		throw new Ice.UnknownUserException();
	    }
	}
	finally
	{
	    reclaimOutgoing(__out);
	}
    }
}

public sealed class Throughput_DelD : Ice.Object_DelD, Throughput_Del
{
    public ByteSeq echoByteSeq(ByteSeq seq, Ice.Context __context)
    {
	Ice.Current __current = new Ice.Current();
	__initCurrent(__current, "echoByteSeq", Ice.OperationMode.Normal, __context);
	while(true)
	{
	    IceInternal.Direct __direct = new IceInternal.Direct(__current);
	    object __servant = __direct.facetServant();
	    if(__servant is Throughput)
	    {
		try
		{
		    return ((Throughput)__servant).echoByteSeq(seq, __current);
		}
		catch(Ice.LocalException __ex)
		{
		    throw new IceInternal.NonRepeatable(__ex);
		}
		finally
		{
		    __direct.destroy();
		}
	    }
	    else
	    {
		__direct.destroy();
		Ice.OperationNotExistException __opEx = new Ice.OperationNotExistException();
		__opEx.id = __current.id;
		__opEx.facet = __current.facet;
		__opEx.operation = __current.operation;
		throw __opEx;
	    }
	}
    }

    public ByteSeq recvByteSeq(Ice.Context __context)
    {
	Ice.Current __current = new Ice.Current();
	__initCurrent(__current, "recvByteSeq", Ice.OperationMode.Normal, __context);
	while(true)
	{
	    IceInternal.Direct __direct = new IceInternal.Direct(__current);
	    object __servant = __direct.facetServant();
	    if(__servant is Throughput)
	    {
		try
		{
		    return ((Throughput)__servant).recvByteSeq(__current);
		}
		catch(Ice.LocalException __ex)
		{
		    throw new IceInternal.NonRepeatable(__ex);
		}
		finally
		{
		    __direct.destroy();
		}
	    }
	    else
	    {
		__direct.destroy();
		Ice.OperationNotExistException __opEx = new Ice.OperationNotExistException();
		__opEx.id = __current.id;
		__opEx.facet = __current.facet;
		__opEx.operation = __current.operation;
		throw __opEx;
	    }
	}
    }

    public void sendByteSeq(ByteSeq seq, Ice.Context __context)
    {
	Ice.Current __current = new Ice.Current();
	__initCurrent(__current, "sendByteSeq", Ice.OperationMode.Normal, __context);
	while(true)
	{
	    IceInternal.Direct __direct = new IceInternal.Direct(__current);
	    object __servant = __direct.facetServant();
	    if(__servant is Throughput)
	    {
		try
		{
		    ((Throughput)__servant).sendByteSeq(seq, __current);
		    return;
		}
		catch(Ice.LocalException __ex)
		{
		    throw new IceInternal.NonRepeatable(__ex);
		}
		finally
		{
		    __direct.destroy();
		}
	    }
	    else
	    {
		__direct.destroy();
		Ice.OperationNotExistException __opEx = new Ice.OperationNotExistException();
		__opEx.id = __current.id;
		__opEx.facet = __current.facet;
		__opEx.operation = __current.operation;
		throw __opEx;
	    }
	}
    }
}

public abstract class Throughput_Disp : Ice.ObjectImpl, Throughput
{
    #region Slice operations

    public abstract void sendByteSeq(ByteSeq seq, Ice.Current __current);

    public abstract ByteSeq recvByteSeq(Ice.Current __current);

    public abstract ByteSeq echoByteSeq(ByteSeq seq, Ice.Current __current);

    #endregion

    #region Slice type-related members

    public static new string[] __ids = 
    {
	"::Ice::Object",
	"::Throughput"
    };

    private static readonly Ice.StringSeq __idSeq = new Ice.StringSeq(__ids);

    public override bool ice_isA(string s, Ice.Current __current)
    {
	return _System.Array.BinarySearch(__ids, s, _System.Collections.Comparer.DefaultInvariant) >= 0;
    }

    public override Ice.StringSeq ice_ids(Ice.Current __current)
    {
	return __idSeq;
    }

    public override string ice_id(Ice.Current __current)
    {
	return __ids[1];
    }

    public static new string ice_staticId()
    {
	return __ids[1];
    }

    #endregion

    #region Operation dispatch

    public static IceInternal.DispatchStatus ___sendByteSeq(Throughput __obj, IceInternal.Incoming __in, Ice.Current __current)
    {
	IceInternal.BasicStream __is = __in.istr();
	ByteSeq seq;
	seq = new ByteSeq(__is.readByteSeq());
	__obj.sendByteSeq(seq, __current);
	return IceInternal.DispatchStatus.DispatchOK;
    }

    public static IceInternal.DispatchStatus ___recvByteSeq(Throughput __obj, IceInternal.Incoming __in, Ice.Current __current)
    {
	IceInternal.BasicStream __os = __in.ostr();
	ByteSeq __ret = __obj.recvByteSeq(__current);
	__os.writeByteSeq(__ret.ToArray());
	return IceInternal.DispatchStatus.DispatchOK;
    }

    public static IceInternal.DispatchStatus ___echoByteSeq(Throughput __obj, IceInternal.Incoming __in, Ice.Current __current)
    {
	IceInternal.BasicStream __is = __in.istr();
	IceInternal.BasicStream __os = __in.ostr();
	ByteSeq seq;
	seq = new ByteSeq(__is.readByteSeq());
	ByteSeq __ret = __obj.echoByteSeq(seq, __current);
	__os.writeByteSeq(__ret.ToArray());
	return IceInternal.DispatchStatus.DispatchOK;
    }

    private static string[] __all =
    {
	"echoByteSeq",
	"ice_facets",
	"ice_id",
	"ice_ids",
	"ice_isA",
	"ice_ping",
	"recvByteSeq",
	"sendByteSeq"
    };

    public override IceInternal.DispatchStatus __dispatch(IceInternal.Incoming __in, Ice.Current __current)
    {
	int pos = _System.Array.BinarySearch(__all, __current.operation, _System.Collections.Comparer.DefaultInvariant);
	if(pos < 0)
	{
	    return IceInternal.DispatchStatus.DispatchOperationNotExist;
	}

	switch(pos)
	{
	    case 0:
	    {
		return ___echoByteSeq(this, __in, __current);
	    }
	    case 1:
	    {
		return ___ice_facets(this, __in, __current);
	    }
	    case 2:
	    {
		return ___ice_id(this, __in, __current);
	    }
	    case 3:
	    {
		return ___ice_ids(this, __in, __current);
	    }
	    case 4:
	    {
		return ___ice_isA(this, __in, __current);
	    }
	    case 5:
	    {
		return ___ice_ping(this, __in, __current);
	    }
	    case 6:
	    {
		return ___recvByteSeq(this, __in, __current);
	    }
	    case 7:
	    {
		return ___sendByteSeq(this, __in, __current);
	    }
	}

	_System.Diagnostics.Debug.Assert(false);
	return IceInternal.DispatchStatus.DispatchOperationNotExist;
    }

    #endregion
}
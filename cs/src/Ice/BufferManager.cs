// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


namespace IceInternal
{

    public sealed class BufferManager
    {
	internal BufferManager()
	{
	    _head = null;
	}
	
	internal ByteBuffer allocate(int size)
	{
	    ByteBuffer buf = getBuffer(size);
	    if(buf == null)
	    {
		try
		{
		    buf = ByteBuffer.allocate(size);
		}
		catch(System.OutOfMemoryException ex)
		{
		    Ice.MemoryLimitException e = new Ice.MemoryLimitException(ex);
		    throw e;
		}
		buf.order(ByteBuffer.ByteOrder.LITTLE_ENDIAN);
	    }
	    return buf;
	}
	
	internal ByteBuffer reallocate(ByteBuffer old, int size)
	{
	    ByteBuffer buf = getBuffer(size);
	    if(buf == null)
	    {
		try
		{
		    buf = ByteBuffer.allocate(size);
		}
		catch(System.OutOfMemoryException ex)
		{
		    throw new Ice.MemoryLimitException(ex);
		}
		buf.order(ByteBuffer.ByteOrder.LITTLE_ENDIAN);
	    }
	    old.position(0);
	    buf.put(old);
	    reclaim(old);
	    return buf;
	}
	
	internal void reclaim(ByteBuffer buf)
	{
	    lock(this)
	    {
		BufferNode node;
		if(_nodeCache == null)
		{
		    node = new BufferNode();
		}
		else
		{
		    node = _nodeCache;
		    _nodeCache = _nodeCache.next;
		}
		node.buf = buf;
		node.capacity = buf.capacity();
		node.next = _head;
		_head = node;
	    }
	}
	
	private ByteBuffer getBuffer(int size)
	{
	    lock(this)
	    {
		BufferNode node = _head;
		BufferNode prev = null;
		while(node != null)
		{
		    if(size <= node.capacity)
		    {
			break;
		    }
		    prev = node;
		    node = node.next;
		}
		if(node != null)
		{
		    if(prev != null)
		    {
			prev.next = node.next;
		    }
		    else
		    {
			_head = node.next;
		    }
		    node.next = _nodeCache;
		    _nodeCache = node;
		    node.buf.clear();
		    return node.buf;
		}
		else
		{
		    return null;
		}
	    }
	}
	
	private sealed class BufferNode
	{
	    internal ByteBuffer buf;
	    internal int capacity;
	    internal BufferNode next;
	}
	
	private BufferNode _head;
	private BufferNode _nodeCache;
    }

}

// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class BufferManager
{
    BufferManager()
    {
        _head = null;
    }

    java.nio.ByteBuffer
    allocate(int size)
    {
        java.nio.ByteBuffer buf = getBuffer(size);
        if(buf == null)
        {
            try
            {
                //buf = java.nio.ByteBuffer.allocateDirect(size);
                buf = java.nio.ByteBuffer.allocate(size);
            }
            catch(OutOfMemoryError ex)
            {
                Ice.MemoryLimitException e = new Ice.MemoryLimitException();
                e.initCause(ex);
                throw e;
            }
            buf.order(java.nio.ByteOrder.LITTLE_ENDIAN);
        }
        return buf;
    }

    java.nio.ByteBuffer
    reallocate(java.nio.ByteBuffer old, int size)
    {
        java.nio.ByteBuffer buf = getBuffer(size);
        if(buf == null)
        {
            try
            {
                //buf = java.nio.ByteBuffer.allocateDirect(size);
                buf = java.nio.ByteBuffer.allocate(size);
            }
            catch(OutOfMemoryError ex)
            {
                Ice.MemoryLimitException e = new Ice.MemoryLimitException();
                e.initCause(ex);
                throw e;
            }
            buf.order(java.nio.ByteOrder.LITTLE_ENDIAN);
        }
        old.position(0);
        buf.put(old);
        reclaim(old);
        return buf;
    }

    synchronized void
    reclaim(java.nio.ByteBuffer buf)
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

    private synchronized java.nio.ByteBuffer
    getBuffer(int size)
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

    private static final class BufferNode
    {
        java.nio.ByteBuffer buf;
        int capacity;
        BufferNode next;
    }

    private BufferNode _head;
    private BufferNode _nodeCache;
}

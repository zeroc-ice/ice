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

final class BufferManager
{
    BufferManager()
    {
        _head = null;
    }

    void
    destroy()
    {
    }

    java.nio.ByteBuffer
    allocate(int size)
    {
        java.nio.ByteBuffer buf = getBuffer(size);
        if (buf == null)
        {
            //buf = java.nio.ByteBuffer.allocateDirect(size);
            buf = java.nio.ByteBuffer.allocate(size);
            buf.order(java.nio.ByteOrder.LITTLE_ENDIAN);
        }
        return buf;
    }

    java.nio.ByteBuffer
    reallocate(java.nio.ByteBuffer old, int size)
    {
        java.nio.ByteBuffer buf = getBuffer(size);
        if (buf == null)
        {
            //buf = java.nio.ByteBuffer.allocateDirect(size);
            buf = java.nio.ByteBuffer.allocate(size);
            buf.order(java.nio.ByteOrder.LITTLE_ENDIAN);
        }
        buf.put(old);
        return buf;
    }

    synchronized void
    reclaim(java.nio.ByteBuffer buf)
    {
        BufferNode node;
        if (_nodeCache == null)
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
        while (node != null)
        {
            if (size <= node.capacity)
            {
                break;
            }
            prev = node;
            node = node.next;
        }
        if (node != null)
        {
            if (prev != null)
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

    private static class BufferNode
    {
        java.nio.ByteBuffer buf;
        int capacity;
        BufferNode next;
    }

    private BufferNode _head;
    private BufferNode _nodeCache;
}

// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Freeze;

class EvictorIteratorI implements EvictorIterator
{
    EvictorIteratorI(java.util.Iterator iterator)
    {
	_iterator = iterator;
    }

    public boolean
    hasNext()
    {
	return _iterator.hasNext();
    }

    public Ice.Identity
    next()
    {
	try
	{
	    return (Ice.Identity)((Map.Entry)_iterator.next()).getKey();
	}
	catch(java.util.NoSuchElementException e)
	{
	    throw new Freeze.NoSuchElementException();
	}
    }

    public void
    destroy()
    {
	((Map.EntryIterator)_iterator).close();
    }
    
    private java.util.Iterator _iterator;
}

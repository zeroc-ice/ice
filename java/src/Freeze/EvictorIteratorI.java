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

package Freeze;

class EvictorIteratorI extends Ice.LocalObjectImpl implements EvictorIterator
{
    EvictorIteratorI(java.util.Iterator iterator)
    {
        //
        // Copy the identities from the map iterator, to allow
        // this iterator to continue to function even if the
        // database is accessed and the map iterator is invalidated.
        //
        while(iterator.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)iterator.next();
            _identities.add(entry.getKey());
        }

	//
	// Close the iterator explicitly. We don't want to wait for
	// the garbage collection to destroy the iterator since the
	// iterator consumes resources (database locks for instance).
	//
	((Freeze.Map.EntryIterator)iterator).close();

        _iterator = _identities.iterator();
    }

    public boolean
    hasNext()
    {
        return _iterator != null && _iterator.hasNext();
    }

    public Ice.Identity
    next()
    {
        if(_iterator != null)
        {
            return (Ice.Identity)_iterator.next();
        }
        else
        {
            throw new Freeze.NoSuchElementException();
        }
    }

    public void
    destroy()
    {
        _identities = null;
        _iterator = null;
    }
    
    private java.util.ArrayList _identities = new java.util.ArrayList();
    private java.util.Iterator _iterator;
}

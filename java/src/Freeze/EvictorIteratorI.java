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

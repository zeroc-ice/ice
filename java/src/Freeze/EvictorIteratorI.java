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
        java.util.ArrayList list = new java.util.ArrayList();
        while(iterator.hasNext())
        {
            list.add(iterator.next());
        }
        _identities= new Ice.Identity[list.size()];
        list.toArray(_identities);
        _pos = 0;
    }

    public boolean
    hasNext()
    {
        return _pos < _identities.length;
    }

    public Ice.Identity
    next()
    {
        if(_pos < _identities.length)
        {
            return _identities[_pos++];
        }
        else
        {
            throw new Freeze.NoSuchElementException();
        }
    }

    public void
    destroy()
    {
        _pos = _identities.length;
    }
    
    private Ice.Identity[] _identities;
    private int _pos;
}

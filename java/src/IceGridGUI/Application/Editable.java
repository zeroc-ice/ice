// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

class Editable implements Cloneable
{
    Editable(boolean brandNew)
    {
        _isNew = brandNew;
    }

    boolean isNew()
    {
        return _isNew;
    }

    boolean isModified()
    {
        return _modified;
    }

    void markModified()
    {
        _modified = true;
    }

    void commit()
    {
        _isNew = false;
        _modified = false;
        _removedElements.clear();
    }
 
    void markNew()
    {
        _isNew = true;
    }

    void removeElement(String id, Editable editable, Class forClass)
    {
        if(!editable.isNew())
        {
            java.util.TreeSet set = (java.util.TreeSet)_removedElements.get(forClass);
            if(set == null)
            {
                set = new java.util.TreeSet();
                _removedElements.put(forClass, set);
            }
            set.add(id);
        }
    }
    
    String[] removedElements(Class forClass)
    {
        java.util.TreeSet set = (java.util.TreeSet)_removedElements.get(forClass);
        if(set == null)
        {
            return new String[0];
        }
        else
        {
            return (String[])set.toArray(new String[0]);
        }
    }

    Editable save()
    {
        try
        {
            Editable result = (Editable)clone();
            java.util.HashMap removedElements = new java.util.HashMap();
            java.util.Iterator p = result._removedElements.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
                Object val = ((java.util.TreeSet)entry.getValue()).clone();
                removedElements.put(entry.getKey(), val);
            }
            result._removedElements = removedElements;
            return result;
        }
        catch(CloneNotSupportedException e)
        {
            assert false;
            return null;
        }
    }

    void restore(Editable clone)
    {
        _isNew = clone._isNew;
        _modified = clone._modified;
        _removedElements = clone._removedElements;
    }

    private boolean _isNew = false;
    private boolean _modified = false;

    private java.util.HashMap _removedElements = new java.util.HashMap();
}

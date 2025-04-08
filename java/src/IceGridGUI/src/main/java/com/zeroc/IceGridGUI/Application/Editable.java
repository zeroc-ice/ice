// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import java.util.HashMap;
import java.util.Map;
import java.util.TreeSet;

class Editable implements Cloneable {
    Editable(boolean brandNew) {
        _isNew = brandNew;
    }

    boolean isNew() {
        return _isNew;
    }

    boolean isModified() {
        return _modified;
    }

    void markModified() {
        _modified = true;
    }

    void commit() {
        _isNew = false;
        _modified = false;
        _removedElements.clear();
    }

    void markNew() {
        _isNew = true;
    }

    void removeElement(String id, Editable editable, Class forClass) {
        if (!editable.isNew()) {
            TreeSet<String> set = _removedElements.get(forClass);
            if (set == null) {
                set = new TreeSet<>();
                _removedElements.put(forClass, set);
            }
            set.add(id);
        }
    }

    String[] removedElements(Class forClass) {
        TreeSet<String> set = _removedElements.get(forClass);
        if (set == null) {
            return new String[0];
        } else {
            return set.toArray(new String[0]);
        }
    }

    Editable save() {
        try {
            Editable result = (Editable) clone();
            HashMap<Class, TreeSet<String>> removedElements =
                new HashMap<>();
            for (Map.Entry<Class, TreeSet<String>> p :
                    result._removedElements.entrySet()) {
                TreeSet<String> val = new TreeSet<>(p.getValue());
                removedElements.put(p.getKey(), val);
            }
            result._removedElements = removedElements;
            return result;
        } catch (CloneNotSupportedException e) {
            assert false;
            return null;
        }
    }

    void restore(Editable clone) {
        _isNew = clone._isNew;
        _modified = clone._modified;
        _removedElements = clone._removedElements;
    }

    private boolean _isNew;
    private boolean _modified;

    private HashMap<Class, TreeSet<String>> _removedElements =
        new HashMap<>();
}

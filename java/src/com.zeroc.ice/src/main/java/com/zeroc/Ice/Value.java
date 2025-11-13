// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.Serializable;

/** The base class for instances of Slice-defined classes. */
public abstract class Value implements Cloneable, Serializable {
    /**
     * Returns a shallow field-for-field copy of this object.
     *
     * @return the cloned object
     */
    @Override
    public Value clone() {
        Value c = null;

        try {
            c = (Value) super.clone();
        } catch (CloneNotSupportedException ex) {
            assert false;
        }

        return c;
    }

    /**
     * The Ice runtime calls this method before marshaling a class instance.
     * Subclasses can override this method in order to update or validate their fields before marshalling.
     */
    public void ice_preMarshal() {}

    /**
     * The Ice runtime calls this method after unmarshaling a class instance.
     * Subclasses can override this method in order to update or validate their fields after unmarshaling.
     */
    public void ice_postUnmarshal() {}

    /**
     * Returns the Slice type ID of the most-derived class supported by this object.
     *
     * @return the Slice type ID
     */
    public String ice_id() {
        return ice_staticId();
    }

    /**
     * Returns the sliced data associated with this instance.
     *
     * @return If this value has a preserved-slice base class and has been sliced during unmarshaling, this returns the
     *     sliced data; otherwise this returns {@code null}.
     */
    public SlicedData ice_getSlicedData() {
        return _slicedData;
    }

    /**
     * Returns the Slice type ID of this type.
     *
     * @return The return value is always {@code "::Ice::Object"}.
     */
    public static String ice_staticId() {
        return "::Ice::Object";
    }

    /**
     * @hidden
     * @param ostr -
     */
    public void _iceWrite(OutputStream ostr) {
        ostr.startValue(_slicedData);
        _iceWriteImpl(ostr);
        ostr.endValue();
    }

    /**
     * @hidden
     * @param istr -
     */
    public void _iceRead(InputStream istr) {
        istr.startValue();
        _iceReadImpl(istr);
        _slicedData = istr.endValue();
    }

    /**
     * @hidden
     * @param ostr -
     */
    protected void _iceWriteImpl(OutputStream ostr) {}

    /**
     * @hidden
     * @param istr -
     */
    protected void _iceReadImpl(InputStream istr) {}

    /** The sliced data associated with this instance. */
    private SlicedData _slicedData;

    private static final long serialVersionUID = 0L;
}

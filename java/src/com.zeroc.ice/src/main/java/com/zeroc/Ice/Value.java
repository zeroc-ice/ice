// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.Serializable;

/** The base class for instances of Slice-defined classes. */
public abstract class Value implements Cloneable, Serializable {
    /**
     * Returns a field-for-field copy of this object.
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
     * The Ice run time calls this method before marshaling an object's data members.
     * This allows a subclass to override this method in order to validate its data members.
     */
    public void ice_preMarshal() {}

    /**
     * The Ice run time calls this method after unmarshaling an object's data members.
     * This allows a subclass to override this method in order to perform additional initialization.
     */
    public void ice_postUnmarshal() {}

    /**
     * Returns the type ID of the most-derived Slice interface supported by this object.
     *
     * @return The return value is always {@code::Ice::Object}.
     */
    public String ice_id() {
        return ice_staticId();
    }

    /**
     * Returns the sliced data associated with this instance.
     *
     * @return The sliced data if the value has a preserved-slice base class and has been sliced
     *     during unmarshaling of the value; {@code null} otherwise.
     */
    public SlicedData ice_getSlicedData() {
        return _slicedData;
    }

    /**
     * Gets the Slice type ID of this type.
     *
     * @return The return value is always {@code::Ice::Object}.
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

    /** The sliced data associated with this object. */
    private SlicedData _slicedData;

    private static final long serialVersionUID = 0L;
}

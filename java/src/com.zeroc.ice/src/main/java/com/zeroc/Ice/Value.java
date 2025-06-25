// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.Serializable;

/** The base class for instances of Slice classes. */
public abstract class Value implements Cloneable, Serializable {
    /**
     * Returns a copy of the object. The cloned object contains field-for-field copies of the state.
     *
     * @return The cloned object.
     */
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
     * The Ice run time invokes this method prior to marshaling an object's data members. This
     * allows a subclass to override this method in order to validate its data members.
     */
    public void ice_preMarshal() {}

    /**
     * The Ice run time invokes this method after unmarshaling an object's data members. This allows
     * a subclass to override this method in order to perform additional initialization.
     */
    public void ice_postUnmarshal() {}

    /**
     * Returns the Slice type ID of the most-derived interface supported by this object.
     *
     * @return The return value is always <code>::Ice::Object</code>.
     */
    public String ice_id() {
        return ice_staticId();
    }

    /**
     * Returns the sliced data if the value has a preserved-slice base class and has been sliced
     * during un-marshaling of the value, null is returned otherwise.
     *
     * @return The sliced data or null.
     */
    public SlicedData ice_getSlicedData() {
        return _slicedData;
    }

    /**
     * Gets the Slice type ID of this type.
     *
     * @return the Slice type ID of this type, which is always <code>::Ice::Object</code>.
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

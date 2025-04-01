// Copyright (c) ZeroC, Inc.

#nullable enable

using System.ComponentModel;

namespace Ice;

public abstract class Value
{
    private const string _id = "::Ice::Object";

    /// <summary>
    /// Returns the Slice type ID of the interface supported by this object.
    /// </summary>
    /// <returns>The return value is always ::Ice::Object.</returns>
    public static string ice_staticId() => _id;

    /// <summary>
    /// Returns the Slice type ID of the most-derived interface supported by this object.
    /// </summary>
    /// <returns>The return value is always ::Ice::Object.</returns>
    public virtual string ice_id() => _id;

    /// <summary>
    /// The Ice run time invokes this method prior to marshaling an object's data members. This allows a subclass
    /// to override this method in order to validate its data members.
    /// </summary>
    public virtual void ice_preMarshal()
    {
    }

    /// <summary>
    /// This Ice run time invokes this method after unmarshaling an object's data members. This allows a
    /// subclass to override this method in order to perform additional initialization.
    /// </summary>
    public virtual void ice_postUnmarshal()
    {
    }

    /// <summary>
    /// Returns the sliced data if the value has a preserved-slice base class and has been sliced during
    /// un-marshaling of the value, null is returned otherwise.
    /// </summary>
    /// <returns>The sliced data or null.</returns>
    public SlicedData? ice_getSlicedData() => _slicedData;

    [EditorBrowsable(EditorBrowsableState.Never)]
    public virtual void iceWrite(OutputStream ostr)
    {
        ostr.startValue(_slicedData);
        iceWriteImpl(ostr);
        ostr.endValue();
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    public virtual void iceRead(InputStream istr)
    {
        istr.startValue();
        iceReadImpl(istr);
        _slicedData = istr.endValue();
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    protected virtual void iceWriteImpl(OutputStream ostr)
    {
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    protected virtual void iceReadImpl(InputStream istr)
    {
    }

    /// <summary>
    /// Returns a copy of the object. The cloned object contains field-for-field copies
    /// of the state.
    /// </summary>
    /// <returns>The cloned object.</returns>
    public Value Clone() => (Value)MemberwiseClone();

    private SlicedData? _slicedData;
}

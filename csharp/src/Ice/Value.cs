// Copyright (c) ZeroC, Inc.

#nullable enable

using System.ComponentModel;

namespace Ice;

/// <summary>
/// The base class for instances of Slice-defined classes.
/// </summary>
public abstract class Value
{
    private const string _id = "::Ice::Object";

    /// <summary>
    /// Returns the Slice type ID of this type.
    /// </summary>
    /// <returns>The return value is always <c>::Ice::Object</c>.</returns>
    public static string ice_staticId() => _id;

    /// <summary>
    /// Returns the Slice type ID of the most-derived class supported by this object.
    /// </summary>
    /// <returns>The Slice type ID.</returns>
    public virtual string ice_id() => _id;

    /// <summary>
    /// The Ice runtime calls this method before marshaling a class instance.
    /// Subclasses can override this method in order to update or validate their fields before marshaling.
    /// </summary>
    public virtual void ice_preMarshal()
    {
    }

    /// <summary>
    /// The Ice runtime calls this method after unmarshaling a class instance.
    /// Subclasses can override this method in order to update or validate their fields after unmarshaling.
    /// </summary>
    public virtual void ice_postUnmarshal()
    {
    }

    /// <summary>
    /// Returns the sliced data associated with this instance.
    /// </summary>
    /// <returns>If this value has a preserved-slice base class and has been sliced during unmarshaling,
    /// this returns the sliced data; otherwise this returns null.</returns>
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
    /// Returns a shallow field-for-field copy of this object.
    /// </summary>
    /// <returns>The cloned object.</returns>
    public Value Clone() => (Value)MemberwiseClone();

    private SlicedData? _slicedData;
}

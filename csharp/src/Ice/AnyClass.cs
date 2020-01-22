//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;

namespace Ice
{
    [Serializable]
    public abstract class AnyClass : ICloneable
    {
        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by this object.
        /// </summary>
        public abstract string ice_id();

        protected virtual IReadOnlyList<SliceInfo>? IceSlicedData
        {
            get => null;
            set => Debug.Assert(false);
        }
        internal IReadOnlyList<SliceInfo>? SlicedData => IceSlicedData;

        [EditorBrowsable(EditorBrowsableState.Never)]
        public virtual void iceWrite(OutputStream ostr)
        {
            ostr.StartClass(null);
            iceWriteImpl(ostr);
            ostr.EndClass();
        }

        // Read all the fields of this instance from the stream. See InputStream.
        [EditorBrowsable(EditorBrowsableState.Never)]
        protected abstract void IceRead(InputStream istr, bool firstSlice);
        internal void Read(InputStream istr) => IceRead(istr, true);

        [EditorBrowsable(EditorBrowsableState.Never)]
        protected virtual void iceWriteImpl(OutputStream ostr)
        {
        }

        /// <summary>
        /// Returns a copy of the object. The cloned object contains field-for-field copies
        /// of the state.
        /// </summary>
        /// <returns>The cloned object.</returns>
        public object Clone()
        {
            return MemberwiseClone();
        }
    }

    public static class AnyClassExtensions
    {
        /// <summary>
        /// During unmarshaling, Ice can slice off derived slices that it does not know how to read, and it can
        /// optionally preserve those "unknown" slices. See the Slice preserve metadata directive and the
        /// class UnknownSlicedClass.
        /// </summary>
        /// <returns>The list of preserved sliced-off slices.</returns>
        public static IReadOnlyList<SliceInfo>? GetSlicedData(this AnyClass obj)
        {
            return obj.SlicedData;
        }
    }
}

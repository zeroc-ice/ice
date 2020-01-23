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

        protected virtual SlicedData? IceSlicedData
        {
            get => null;
            set => Debug.Assert(false);
        }
        internal SlicedData? SlicedData => IceSlicedData;

        // Read all the fields of this instance from the stream. See InputStream.
        [EditorBrowsable(EditorBrowsableState.Never)]
        protected abstract void IceRead(InputStream istr, bool firstSlice);
        internal void Read(InputStream istr) => IceRead(istr, true);

        [EditorBrowsable(EditorBrowsableState.Never)]
        protected abstract void IceWrite(OutputStream ostr, bool firstSlice);
        internal void Write(OutputStream ostr) => IceWrite(ostr, true);

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
        /// <returns>A SlicedData value that provides the list of sliced-off slices.</returns>
        public static SlicedData? GetSlicedData(this AnyClass obj)
        {
            return obj.SlicedData;
        }
    }
}

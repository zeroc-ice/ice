//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.ComponentModel;

namespace Ice
{
    [Serializable]
    public abstract class AnyClass : ICloneable
    {
        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by this object.
        /// </summary>
        public abstract string ice_id();

        /// <summary>
        /// Returns the sliced data if the value has a preserved-slice base class and has been sliced during
        /// un-marshaling of the value, null is returned otherwise.
        /// </summary>
        /// <returns>The sliced data or null.</returns>
        public virtual SlicedData? ice_getSlicedData() => null;

        [EditorBrowsable(EditorBrowsableState.Never)]
        public virtual void iceWrite(OutputStream ostr)
        {
            ostr.StartClass(null);
            iceWriteImpl(ostr);
            ostr.EndClass();
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public virtual void iceRead(InputStream istr, bool firstSlice)
        {
            istr.StartClass();
            iceReadImpl(istr, true);
            istr.EndClass(false);
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        protected virtual void iceWriteImpl(OutputStream ostr)
        {
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        protected virtual void iceReadImpl(InputStream istr, bool firstSlice)
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
}

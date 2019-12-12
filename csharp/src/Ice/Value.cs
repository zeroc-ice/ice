//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.ComponentModel;

namespace Ice
{
    [Serializable]
    public abstract class Value : ICloneable
    {
        private const string _id = "::Ice::Object";

        /// <summary>
        /// Returns the Slice type ID of the interface supported by this object.
        /// </summary>
        /// <returns>The return value is always ::Ice::IObject.</returns>
        public static string ice_staticId()
        {
            return _id;
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by this object.
        /// </summary>
        /// <returns>The return value is always ::Ice::IObject.</returns>
        public virtual string ice_id()
        {
            return _id;
        }

        /// <summary>
        /// Returns the sliced data if the value has a preserved-slice base class and has been sliced during
        /// un-marshaling of the value, null is returned otherwise.
        /// </summary>
        /// <returns>The sliced data or null.</returns>
        public virtual SlicedData? ice_getSlicedData()
        {
            return null;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public virtual void iceWrite(OutputStream ostr)
        {
            ostr.StartValue(null);
            iceWriteImpl(ostr);
            ostr.EndValue();
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public virtual void iceRead(InputStream istr)
        {
            istr.StartValue();
            iceReadImpl(istr);
            istr.EndValue(false);
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
        public object Clone()
        {
            return MemberwiseClone();
        }
    }

    public class InterfaceByValue : Value
    {
        public InterfaceByValue(string id)
        {
            _id = id;
        }

        public override string ice_id()
        {
            return _id;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        protected override void iceWriteImpl(OutputStream ostr)
        {
            ostr.StartSlice(ice_id(), -1, true);
            ostr.EndSlice();
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        protected override void iceReadImpl(InputStream istr)
        {
            istr.StartSlice();
            istr.EndSlice();
        }

        private string _id;
    }
}

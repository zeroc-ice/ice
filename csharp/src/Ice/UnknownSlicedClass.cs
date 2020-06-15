//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>UnknownSlicedClass represents a fully sliced class instance. The local Ice runtime does not known this
    /// type or any of its base classes (other than AnyClass).</summary>
    public sealed class UnknownSlicedClass : AnyClass
    {
        /// <summary>Returns the Slice type ID associated with this sliced class instance.</summary>
        /// <value>The type ID.</value>
        public string TypeId
        {
            get
            {
                Debug.Assert(IceSlicedData is SlicedData slicedData && slicedData.Slices.Count > 0);
                return IceSlicedData.Value.Slices[0].TypeId;
            }
        }

        protected override void IceRead(InputStream istr, bool firtSlice)
        {
            IceSlicedData = istr.IceStartFirstSlice();
            Debug.Assert(IceSlicedData != null);
        }

        protected override SlicedData? IceSlicedData { get; set; }

        protected override void IceWrite(OutputStream ostr, bool firstSlice) =>
            ostr.WriteSlicedData(IceSlicedData!.Value, Array.Empty<string>());

        internal UnknownSlicedClass()
        {
        }
    }
}

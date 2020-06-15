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
        /// <summary>Returns the most derived type ID this class instance.</summary>
        /// <value>The type ID.</value>
        public string TypeId => IceSlicedData!.Value.Slices[0].TypeId;

        protected override void IceRead(InputStream istr, bool firtSlice) => IceSlicedData = istr.SlicedData;

        protected override SlicedData? IceSlicedData { get; set; }

        protected override void IceWrite(OutputStream ostr, bool firstSlice) =>
            ostr.WriteSlicedData(IceSlicedData!.Value, Array.Empty<string>());

        internal UnknownSlicedClass()
        {
        }
    }
}

// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    /// <summary>
    /// Unknown sliced object holds an instance of unknown type.
    /// </summary>
    public sealed class UnknownSlicedObject : ObjectImpl
    {
        /// <summary>
        /// Instantiates the class for an Ice object having the given Slice type.
        /// </summary>
        /// <param name="unknownTypeId">The Slice type ID of the unknown object.</param>
        public UnknownSlicedObject(string unknownTypeId)
        {
            _unknownTypeId = unknownTypeId;
        }

        /// <summary>
        /// Determine the Slice type ID associated with this object.
        /// </summary>
        /// <returns>The type ID.</returns>
        public string getUnknownTypeId()
        {
            return _unknownTypeId;
        }

        public override void write__(OutputStream os__)
        {
            os__.startObject(_slicedData);
            os__.endObject();
        }

        public override void read__(InputStream is__)
        {
            is__.startObject();
            _slicedData = is__.endObject(true);
        }

        private string _unknownTypeId;
        private SlicedData _slicedData;
    }
}

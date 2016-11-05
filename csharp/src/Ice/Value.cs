// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

namespace Ice
{
    [Serializable]
    public abstract class Value : ICloneable
    {
        public static readonly string static_id__ = "::Ice::Object";

        /// <summary>
        /// Returns the Slice type ID of the interface supported by this object.
        /// </summary>
        /// <returns>The return value is always ::Ice::Object.</returns>
        public static string ice_staticId()
        {
            return static_id__;
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by this object.
        /// </summary>
        /// <returns>The return value is always ::Ice::Object.</returns>
        public virtual string ice_id()
        {
            return static_id__;
        }

        /// <summary>
        /// The Ice run time invokes this method prior to marshaling an object's data members. This allows a subclass
        /// to override this method in order to validate its data members.
        /// </summary>
        public virtual void ice_preMarshal()
        {
        }

        /// <summary>
        /// This Ice run time invokes this method vafter unmarshaling an object's data members. This allows a
        /// subclass to override this method in order to perform additional initialization.
        /// </summary>
        public virtual void ice_postUnmarshal()
        {
        }

        public virtual void write__(OutputStream os__)
        {
            os__.startValue(null);
            writeImpl__(os__);
            os__.endValue();
        }

        public virtual void read__(InputStream is__)
        {
            is__.startValue();
            readImpl__(is__);
            is__.endValue(false);
        }

        protected virtual void writeImpl__(OutputStream os__)
        {
        }

        protected virtual void readImpl__(InputStream is__)
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
            id_ = id;
        }

        public override string ice_id()
        {
            return id_;
        }

        protected override void writeImpl__(OutputStream os__)
        {
            os__.startSlice(ice_id(), -1, true);
            os__.endSlice();
        }

        protected override void readImpl__(InputStream is__)
        {
            is__.startSlice();
            is__.endSlice();
        }

        private string id_;
    } 
}

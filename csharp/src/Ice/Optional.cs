// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    using System;
    using System.Collections.Generic;
    using System.Runtime.Serialization;

    public struct NoneType
    {
    }

    /// <summary>
    /// Encapsulates an optional value. Instances of this type are immutable.
    /// </summary>
    [Serializable]
    public struct Optional<T> : ISerializable
    {
        /// <summary>
        /// Creates an optional value whose state is unset.
        /// </summary>
        public Optional(NoneType none)
        {
            _value = default(T);
            _isSet = false;
        }

        /// <summary>
        /// Creates an optional value and sets its value to the given argument.
        /// </summary>
        public Optional(T v)
        {
            _value = v;
            _isSet = true;
        }

        /// <summary>
        /// Creates an optional value whose state is copied from the given argument.
        /// </summary>
        public Optional(Optional<T> v)
        {
            _value = v._value;
            _isSet = v._isSet;
        }

        /// <summary>
        /// Initializes a new instance of the exception with serialized data.
        /// </summary>
        /// <param name="info">Holds the serialized object data about the exception being thrown.</param>
        /// <param name="context">Contains contextual information about the source or destination.</param>
        public Optional(SerializationInfo info, StreamingContext context)
        {
            _isSet = info.GetBoolean("isSet");
            if(_isSet)
            {
                _value = (T)info.GetValue("value", typeof(T));
            }
            else
            {
                _value = default(T);
            }
        }

        /// <summary>
        /// Conversion operator to the underlying type; a cast is required. An exception
        /// is raised if no value is set.
        /// </summary>
        /// <returns>The encapsulated value.</returns>
        /// <exception cref="System.InvalidOperationException">Thrown if no value is set.</exception>
        public static explicit operator T(Optional<T> v)
        {
            return v.Value;
        }

        /// <summary>
        /// Conversion operator from a value of the underlying type; no cast is required.
        /// </summary>
        public static implicit operator Optional<T>(T v)
        {
            return new Optional<T>(v);
        }

        /// <summary>
        /// Conversion operator from a None value; no cast is required.
        /// </summary>
        public static implicit operator Optional<T>(NoneType v)
        {
            return new Optional<T>();
        }

        /// <summary>
        /// Reads and writes the encapsulated value.
        /// </summary>
        /// <exception cref="System.InvalidOperationException">Thrown if the property is read and no value is
        /// set.</exception>
        public T Value
        {
            get
            {
                if(!_isSet)
                {
                    throw new System.InvalidOperationException();
                }
                return _value;
            }
        }

        /// <summary>
        /// Determines whether a value is set.
        /// </summary>
        /// <returns>True if a value is set, false otherwise.</returns>
        public bool HasValue
        {
            get
            {
                return _isSet;
            }
        }

        public override bool Equals(object other)
        {
            if(object.ReferenceEquals(this, other))
            {
                return true;
            }
            if(other == null)
            {
                return false;
            }

            try
            {
                Optional<T> o2 = (Optional<T>)other;

                if(_isSet != o2._isSet)
                {
                    return false;
                }
                else if(_isSet)
                {
                    EqualityComparer<T> comparer = EqualityComparer<T>.Default;
                    return comparer.Equals(_value, o2._value);
                }

                return true;
            }
            catch(System.Exception)
            {
                return false;
            }
        }

        public override int GetHashCode()
        {
            if(!_isSet)
            {
                return base.GetHashCode();
            }
            else
            {
                return _value.GetHashCode();
            }
        }

        /// <summary>
        /// Serializes an optional value.
        /// </summary>
        /// <param name="info">Holds the serialized object data about the exception being thrown.</param>
        /// <param name="context">Contains contextual information about the source or destination.</param>
        public void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("isSet", _isSet);
            if(_isSet)
            {
                info.AddValue("value", _value, typeof(T));
            }
        }

        private T _value;
        private bool _isSet;
    }

    /// <summary>
    /// Handles callbacks for an optional object parameter.
    /// </summary>
    public class OptionalPatcher<T> where T : Ice.Value
    {
        /// <summary>
        /// Instantiates the class with the given optional.
        /// </summary>
        /// <param name="type">The Slice type ID corresponding to the formal type.</param>
        public OptionalPatcher(string type)
        {
            _type = type;
        }

        /// <summary>
        /// Sets the Ice object of the optional to the passed instance.
        /// </summary>
        /// <param name="v">The new object for the optional.</param>
        public void patch(Ice.Value v)
        {
            if(v == null || typeof(T).IsAssignableFrom(v.GetType()))
            {
                //
                // The line below must assign to the Value property. We could also
                // have written it this way:
                //
                // this.opt = (T)v;
                //
                // However, when v is null, the optional might be cleared, which
                // is not the result we want.
                //
                this.value = new Optional<T>((T)v);
            }
            else
            {
                IceInternal.Ex.throwUOE(_type, v.ice_id());
            }
        }

        /// <summary>
        /// The target optional.
        /// </summary>
        public Optional<T> value = new Optional<T>();
        private string _type;
    }

    /// <summary>
    /// The optional format.
    ///
    /// An optional value is encoded with a specific optional format. This optional
    /// format describes how the data is encoded and how it can be skipped by the
    /// unmarshaling code if the optional is not known to the receiver.
    /// </summary>
    public enum OptionalFormat
    {
        F1 = 0,
        F2 = 1,
        F4 = 2,
        F8 = 3,
        Size = 4,
        VSize = 5,
        FSize = 6,
        Class = 7
    }
}

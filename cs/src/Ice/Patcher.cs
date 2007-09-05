// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace IceInternal
{
    public abstract class Patcher : Ice.ReadObjectCallback
    {
        public Patcher()
        {
        }

        public Patcher(System.Type type)
        {
            type_ = type;
        }

        public abstract void patch(Ice.Object v);

        public virtual string type()
        {
            Debug.Assert(type_ != null);
            return type_.FullName;
        }

        public virtual void invoke(Ice.Object v)
        {
            patch(v);
        }

        protected System.Type type_;
    }

    public sealed class ParamPatcher : Patcher
    {
        public ParamPatcher(System.Type type, string expectedSliceType) : base(type)
        {
            _expectedSliceType = expectedSliceType;
        }

        public override void patch(Ice.Object v)
        {
            Debug.Assert(type_ != null);
            if(v != null && !type_.IsInstanceOfType(v))
            {
                throw new Ice.UnexpectedObjectException(
                        "unexpected class instance of type `" + v.ice_id()
                            + "'; expected instance of type '" + _expectedSliceType + "'",
                        v.ice_id(), _expectedSliceType);
            }
            value = v;
        }

        public Ice.Object value;
        private string _expectedSliceType;
    }

    public sealed class SequencePatcher<T> : Patcher
    {
        public SequencePatcher(Ice.CollectionBase<T> seq, System.Type type, int index) : base(type)
        {
            _seqList = (List<T>)seq;
            _stype = SeqType.IList;
            _index = index;
        }

        public SequencePatcher(ICollection<T> seq, System.Type type, int index) : base(type)
        {
            if(seq is System.Array)
            {
                _seqArray = (T[])seq;
                _stype = SeqType.Array;
            }
            else if(seq is IList<T>)
            {
                _seqList = (IList<T>)seq;
                _stype = SeqType.IList;
            }
            else
            {
                _seqColl = seq;
                _stype = SeqType.Collection;
            }
            _index = index;
        }

        public override void patch(Ice.Object v)
        {
            Debug.Assert(type_ != null);
            if(v != null && !type_.IsInstanceOfType(v))
            {
                throw new System.InvalidCastException("expected element of type " + type() +
                                                      " but received " + v.GetType().FullName); 
            }

            switch(_stype)
            {
                case SeqType.IList:
                {
                    int count = _seqList.Count;
                    if(_index >= count) // Need to grow the sequence.
                    {
                        for(int i = count; i < _index; i++)
                        {
                            _seqList.Add(default(T));
                        }
                        _seqList.Add((T)v);
                    }
                    else
                    {
                        _seqList[_index] = (T)v;
                    }
                    break;
                }
                case SeqType.Array:
                {
                    _seqArray[_index] = (T)v;
                    break;
                }
                case SeqType.Collection:
                {
                            // TODO
                    break;
                }
            }
        }

        private enum SeqType { Array, IList , Collection };

        private SeqType _stype;

        //
        // Conceptually, these three members are in a union. We use separate members
        // here instead of a single member of base type to avoid a separate down-cast
        // each time patch() is called.
        //
        private IList<T> _seqList;
        private T[] _seqArray;
        private ICollection<T> _seqColl;

        private int _index; // The index at which to patch the sequence.
    }
}

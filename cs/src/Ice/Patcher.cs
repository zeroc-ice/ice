// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
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

    public sealed class SequencePatcher : Patcher
    {
        public SequencePatcher(ICollection seq, System.Type type, int index) : base(type)
        {
            _seq = seq;
            _index = index;
        }
    
        private static object dummyObject = new object();

        public override void patch(Ice.Object v)
        {
            Debug.Assert(type_ != null);
            if(v != null && !type_.IsInstanceOfType(v))
            {
                throw new System.InvalidCastException("expected element of type " + type() +
                                                      " but received " + v.GetType().FullName); 
            }

            try 
            {
                if(_seq is CollectionBase)
                {
                    if(_index >= _seq.Count) // Need to grow the sequence.
                    {
                        for(int i = _seq.Count; i <= _index; i++)
                        {
                            ((IList)_seq).Add(dummyObject); // Broken CollectionBase implementation
                        }
                    }

                    //
                    // Ugly work-around for broken CollectionBase implementation:
                    //
                    // CollectionBase provides implementations of the IList.Add method
                    // and indexer. However, these implementations do not permit
                    // null to be added or assigned even though, according to the doc, this should work.
                    // (Attempts to put a null into the collection raise ArgumentNullException.)
                    // That's why the above code grows the sequence by adding a dummy object.
                    //
                    // Furthermore, CollectionBase.Add and the indexer are (incorrectly) non-virtual so,
                    // when we treat _seq as an IList, we always dispatch into the CollectionBase
                    // implementation, not into the implementation of the generated sequence type.
                    // This makes it impossible to assign v to a sequence element if v is null.
                    //
                    // The ugly work-around is to use reflection to ensure that we get the
                    // actual run-time type of the generated sequence, and then
                    // use dynamic invocation to make sure that we dispatch to the generated indexer,
                    // instead of the broken indexer provided by CollectionBase.
                    //
                    if(v == null)
                    {
                        object[] ov = new object[2];
                        ov[0] = _index;
                        ov[1] = null;
                        _seq.GetType().GetProperty("Item").GetSetMethod().Invoke(_seq, ov);
                    }
                    else
                    {
                        ((IList)_seq)[_index] = v;
                    }
                }
                else
                {
                    ((System.Array)_seq).SetValue(v, _index);
                }
            }
            catch(System.Exception ex)
            {
                throw new Ice.MarshalException("Unexpected failure during patching", ex);
            }
        }

        private ICollection _seq;
        private int _index;
    }
}

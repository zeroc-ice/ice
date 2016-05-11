// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Reflection;
using System.Diagnostics;

namespace IceInternal
{
    public sealed class ParamPatcher<T>
    {
        public ParamPatcher(string type)
        {
            _type = type;
        }

        public void patch(Ice.Object v)
        {
            if(v != null && !typeof(T).IsAssignableFrom(v.GetType()))
            {
                IceInternal.Ex.throwUOE(_type, v.ice_id());
            }
            value = (T)v;
        }

        public T value;
        private string _type;
    }

    public sealed class CustomSeqPatcher<T>
    {
        public CustomSeqPatcher(string type, IEnumerable<T> seq, int index)
        {
            _type = type;
           _seq = seq;
           _seqType = seq.GetType();
           _index = index;

           setInvokeInfo(_seqType);
        }

        public void patch(Ice.Object v)
        {
            if(v != null && !typeof(T).IsAssignableFrom(v.GetType()))
            {
                IceInternal.Ex.throwUOE(_type, v.ice_id());
            }

            InvokeInfo info = getInvokeInfo(_seqType);
            int count = info.getCount(_seq);
            if(_index >= count) // Need to grow the sequence.
            {
                for(int i = count; i < _index; i++)
                {
                    info.invokeAdd(_seq, default(T));
                }
                info.invokeAdd(_seq, (T)v);
            }
            else
            {
                info.invokeSet(_seq, _index, (T)v);
            }
        }

        private static InvokeInfo getInvokeInfo(Type t)
        {
            lock(_methodTable)
            {
                try
                {
                    return _methodTable[t];
                }
                catch(KeyNotFoundException)
                {
                    throw new Ice.MarshalException("No invoke record for type " + t.ToString());
                }
            }
        }

        private static void setInvokeInfo(Type t)
        {
            lock(_methodTable)
            {
                if(_methodTable.ContainsKey(t))
                {
                    return;
                }

                MethodInfo am = t.GetMethod("Add", _params);
                if(am == null)
                {
                    throw new Ice.MarshalException("Cannot patch a collection without an Add() method");
                }

                PropertyInfo pi = t.GetProperty("Item");
                if(pi == null)
                {
                    throw new Ice.MarshalException("Cannot patch a collection without an indexer");
                }
                MethodInfo sm = pi.GetSetMethod();
                if(sm == null)
                {
                    throw new Ice.MarshalException("Cannot patch a collection without an indexer to set a value");
                }

                pi = t.GetProperty("Count");
                if(pi == null)
                {
                    throw new Ice.MarshalException("Cannot patch a collection without a Count property");
                }
                MethodInfo cm = pi.GetGetMethod();
                if(cm == null)
                {
                    throw new Ice.MarshalException("Cannot patch a collection without a readable Count property");
                }

                _methodTable.Add(t, new InvokeInfo(am, sm, cm));
            }
        }

        private class InvokeInfo
        {
            public InvokeInfo(MethodInfo am, MethodInfo sm, MethodInfo cm)
            {
                _addMethod = am;
                _setMethod = sm;
                _countMethod = cm;
            }

            internal int getCount(System.Collections.IEnumerable seq)
            {
                try
                {
                    return (int)_countMethod.Invoke(seq, null);
                }
                catch(Exception ex)
                {
                    throw  new Ice.MarshalException("Could not read Count property during patching", ex);
                }
            }

            internal void invokeAdd(System.Collections.IEnumerable seq, T v)
            {
                try
                {
                    object[] arg = new object[] { v };
                    _addMethod.Invoke(seq, arg);
                }
                catch(Exception ex)
                {
                    throw  new Ice.MarshalException("Could not invoke Add method during patching", ex);
                }
            }

            internal void invokeSet(System.Collections.IEnumerable seq, int index, T v)
            {
                try
                {
                    object[] args = new object[] { index, v };
                    _setMethod.Invoke(seq, args);
                }
                catch(Exception ex)
                {
                    throw  new Ice.MarshalException("Could not call indexer during patching", ex);
                }
            }

            private MethodInfo _addMethod;
            private MethodInfo _setMethod;
            private MethodInfo _countMethod;
        }

        private static Type[] _params = new Type[] { typeof(T) };
        private static Dictionary<Type, InvokeInfo> _methodTable = new Dictionary<Type, InvokeInfo>();

        private string _type;
        private IEnumerable<T> _seq;
        private Type _seqType;
        private int _index; // The index at which to patch the sequence.
    }

    public sealed class ArrayPatcher<T>
    {
        public ArrayPatcher(string type, T[] seq, int index)
        {
            _type = type;
            _seq = seq;
            _index = index;
        }

        public void patch(Ice.Object v)
        {
            if(v != null && !typeof(T).IsAssignableFrom(v.GetType()))
            {
                IceInternal.Ex.throwUOE(_type, v.ice_id());
            }

            _seq[_index] = (T)v;
        }

        private string _type;
        private T[] _seq;
        private int _index; // The index at which to patch the array.
    }

    public sealed class ListPatcher<T>
    {
        public ListPatcher(string type, List<T> seq, int index)
        {
            _type = type;
            _seq = seq;
            _index = index;
        }

        public void patch(Ice.Object v)
        {
            if(v != null && !typeof(T).IsAssignableFrom(v.GetType()))
            {
                IceInternal.Ex.throwUOE(_type, v.ice_id());
            }

            int count = _seq.Count;
            if(_index >= count) // Need to grow the sequence.
            {
                for(int i = count; i < _index; i++)
                {
                    _seq.Add(default(T));
                }
                _seq.Add((T)v);
            }
            else
            {
                _seq[_index] = (T)v;
            }
        }

        private string _type;
        private List<T> _seq;
        private int _index; // The index at which to patch the sequence.
    }
}

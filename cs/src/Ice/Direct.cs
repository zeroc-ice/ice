// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Diagnostics;

    public class Direct : Ice.Request
    {
        public delegate Ice.DispatchStatus RunDelegate(Ice.Object target);

        public bool isCollocated()
        {
            return true;
        }

        public Ice.Current getCurrent()
        {
            return _current;
        }

        public Ice.DispatchStatus run(Ice.Object target)
        {
            return _run(target);
        }

        public Direct(Ice.Current current, RunDelegate run)
        {
            _current = current;
            _run = run;
            
            Ice.ObjectAdapterI adapter = (Ice.ObjectAdapterI)_current.adapter;
            Debug.Assert(adapter != null);
            
            //
            // Must call incDirectCount() first, because it checks for
            // adapter deactivation, and prevents deactivation completion
            // until decDirectCount() is called. This is important,
            // because getServantManager() may not be called afer
            // deactivation completion.
            //
            adapter.incDirectCount();
            
            ServantManager servantManager = adapter.getServantManager();
            Debug.Assert(servantManager != null);
            
            _servant = servantManager.findServant(_current.id, _current.facet);
            if(_servant == null)
            {
                _locator = servantManager.findServantLocator(_current.id.category);
                if(_locator == null && _current.id.category.Length > 0)
                {
                    _locator = servantManager.findServantLocator("");
                }
                if(_locator != null)
                {
                    try
                    {
                        _servant = _locator.locate(_current, out _cookie);
                    }
                    catch(System.Exception)
                    {
                        adapter.decDirectCount();
                        throw;
                    }
                }
            }

            if(_servant == null)
            {
                adapter.decDirectCount();
                if(servantManager != null && servantManager.hasServant(_current.id))
                {
                    Ice.FacetNotExistException ex = new Ice.FacetNotExistException();
                    ex.id = _current.id;
                    ex.facet = _current.facet;
                    ex.operation = _current.operation;
                    throw ex;
                }
                else
                {
                    Ice.ObjectNotExistException ex = new Ice.ObjectNotExistException();
                    ex.id = _current.id;
                    ex.facet = _current.facet;
                    ex.operation = _current.operation;
                    throw ex;
                }
            }
        }
        
        public void destroy()
        {
            Ice.ObjectAdapterI adapter = (Ice.ObjectAdapterI)_current.adapter;
            Debug.Assert(adapter != null);
            
            try
            {
                if(_locator != null && _servant != null)
                {
                    _locator.finished(_current, _servant, _cookie);
                }
            }
            finally
            {
                adapter.decDirectCount();
            }
        }
        
        public Ice.Object getServant()
        {
            return _servant;
        }
        
        private readonly Ice.Current _current;
        private readonly RunDelegate _run;
        private Ice.Object _servant;
        private Ice.ServantLocator _locator;
        private System.Object _cookie;
    }

}

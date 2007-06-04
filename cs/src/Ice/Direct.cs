// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Diagnostics;

    public sealed class Direct
    {
        public Direct(Ice.Current current)
        {
            _current = current;
            
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
            
            try
            {
                _servant = servantManager.findServant(_current.id, current.facet);
                if(_servant == null)
                {
                    _locator = servantManager.findServantLocator(_current.id.category);
                    if(_locator == null && _current.id.category.Length > 0)
                    {
                        _locator = servantManager.findServantLocator("");
                    }
                    if(_locator != null)
                    {
                        _servant = _locator.locate(_current, out _cookie);
                    }
                }
                if(_servant == null)
                {
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
            catch(System.Exception)
            {
                try
                {
                    if(_locator != null && _servant != null)
                    {
                        _locator.finished(_current, _servant, _cookie);
                    }
                    throw;
                }
                finally
                {
                    adapter.decDirectCount();
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
        
        public Ice.Object servant()
        {
            return _servant;
        }
        
        private Ice.Current _current;
        private Ice.Object _servant;
        private Ice.ServantLocator _locator;
        private System.Object _cookie;
    }

}

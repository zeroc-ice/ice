// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class Incoming
{
    /*
    public
    Incoming(Instance instance, Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _is = new BasicStream(instance);
        _os = new BasicStream(instance);
    }

    public void
    invoke(BasicStream is)
    {
        _is.swap(is);
        String identity = _is.readString();
        String facet = _is.readString();
        String operation = _is.readString();

        int statusPos = _os.size();

        Ice.Object servant = null;
        Ice.ServantLocator locator = null;
        Ice.LocalObjectHolder cookie = new Ice.LocalObjectHolder();

        try
        {
            servant = _adapter.identityToServant(identity);

            if (servant == null)
            {
                int pos = identity.indexOf('#');
                if (pos != -1)
                {
                    locator = _adapter.findServantLocator(
                        identity.substring(0, pos));
                    if (locator != null)
                    {
                        servant = locator.locate(_adapter, identity, operation,
                                                 cookie);
                    }
                }
            }

            if (servant == null)
            {
                locator = _adapter.findServantLocator("");
                if (locator != null)
                {
                    servant = locator.locate(_adapter, identity, operation,
                                             cookie);
                }
            }

            if (servant == null)
            {
                _os.writeByte((byte)DispatchStatus._DispatchObjectNotExist);
            }
            else
            {
                if (facet.length() > 0)
                {
                    Ice.Object facetServant = servant.ice_findFacet(facet);
                    if (facetServant == null)
                    {
                        _os.writeByte(
                            (byte)DispatchStatus._DispatchFacetNotExist);
                    }
                    else
                    {
                        _os.writeByte((byte)DispatchStatus._DispatchOK);
                        DispatchStatus status =
                            facetServant.__dispatch(this, operation);
                        int save = _os.pos();
                        _os.pos(statusPos);
                        _os.writeByte((byte)status.value());
                        _os.pos(save);
                    }
                }
                else
                {
                    _os.writeByte((byte)DispatchStatus._DispatchOK);
                    DispatchStatus status =
                        servant.__dispatch(this, operation);
                    int save = _os.pos();
                    _os.pos(statusPos);
                    _os.writeByte((byte)status.value());
                    _os.pos(save);
                }
            }

            if (locator != null && servant != null)
            {
                locator.finished(_adapter, identity, operation, servant,
                                 cookie.value);
            }
        }
        catch (Ice.LocationForward ex)
        {
            if (locator != null && servant != null)
            {
                locator.finished(_adapter, identity, operation, servant,
                                 cookie.value);
            }
            _os.resize(statusPos);
            // TODO: Update position?
            _os.writeByte((byte)DispatchStatus._DispatchLocationForward);
            _os.writeProxy(ex._prx);
            return;
        }
        catch (Ice.LocalException ex)
        {
            if (locator != null && servant != null)
            {
                locator.finished(_adapter, identity, operation, servant,
                                 cookie.value);
            }
            _os.resize(statusPos);
            // TODO: Update position?
            _os.writeByte((byte)DispatchStatus._DispatchUnknownLocalException);
            throw ex;
        }
        catch (Ice.UserException ex)
        {
            if (locator != null && servant != null)
            {
                locator.finished(_adapter, identity, operation, servant,
                                 cookie.value);
            }
            _os.resize(statusPos);
            // TODO: Update position?
            _os.writeByte((byte)DispatchStatus._DispatchUnknownUserException);
            // TODO: Throw UserException here?
            // throw ex;
            return;
        }
        catch (Exception ex)
        {
            if (locator != null && servant != null)
            {
                locator.finished(_adapter, identity, operation, servant,
                                 cookie.value);
            }
            _os.resize(statusPos);
            // TODO: Update position?
            _os.writeByte((byte)DispatchStatus._DispatchUnknownException);
            throw new Ice.UnknownException(); // TODO: Chain?
        }
    }

    public BasicStream
    is()
    {
        return _is;
    }

    public BasicStream
    os()
    {
        return _os;
    }

    private Ice.ObjectAdapter _adapter;

    private BasicStream _is;
    private BasicStream _os;
    */
}

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
    public
    Incoming(Instance instance, Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _is = new StreamI(instance);
        _os = new StreamI(instance);
    }

    public void
    invoke(Stream is)
    {
        _is.swap(is);
        String identity = _is.readString();
        String facet = _is.readString();
        String operation = _is.readString();

        int statusPos = ...; // TODO

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
                        // TODO: Patch new status back into _os
                    }
                }
                else
                {
                    _os.writeByte((byte)DispatchStatus._DispatchOK);
                    DispatchStatus status =
                        servant.__dispatch(this, operation);
                    // TODO: Patch new status back into _os
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
            _os.writeByte((byte)DispatchStatus._DispatchLocationForward);
            // TODO
            return;
        }
        catch (Ice.LocalException ex)
        {
            if (locator != null && servant != null)
            {
                locator.finished(_adapter, identity, operation, servant,
                                 cookie.value);
            }
            // TODO
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
            // TODO
            _os.writeByte((byte)DispatchStatus._DispatchUnknownUserException);
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
            // TODO
            _os.writeByte((byte)DispatchStatus._DispatchUnknownException);
            throw new Ice.UnknownException(); // TODO: Chain?
        }
    }

    public Stream
    is()
    {
        return _is;
    }

    public Stream
    os()
    {
        return _os;
    }

    private Ice.ObjectAdapter _adapter;

    private Stream _is;
    private Stream _os;
}

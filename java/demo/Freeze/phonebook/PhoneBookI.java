// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class PhoneBookI extends _PhoneBookDisp
{
    public synchronized ContactPrx
    createContact(Ice.Current current)
	throws DatabaseException
    {
	//
	// Get a new unique identity.
	//
	Ice.Identity ident = getNewIdentity();

	//
	// Create a new Contact Servant.
	//
	ContactI contact = new ContactI(this, _evictor);
	contact.setIdentity(ident);
    
	//
	// Create a new Ice Object in the evictor, using the new //
	// identity and the new Servant.
	//
	// This can throw EvictorDeactivatedException (which indicates
	// an internal error). The exception is currently ignored.
	//
	_evictor.createObject(ident, contact);

	//
	// Add the identity to our name/identities map. The initial
	// name is the empty string. See the comment in getNewIdentity
	// why the prefix "N" is needed.
	//
	try
	{
	    Ice.Identity[] identities = (Ice.Identity[])_nameIdentitiesDict.get("N");
	    int length = (identities == null) ? 0 : identities.length;
	    Ice.Identity[] newIdents = new Ice.Identity[length+1];

	    if (identities != null)
	    {
		System.arraycopy(identities, 0, newIdents, 0, length);
	    }
	    newIdents[length] = ident;

	    _nameIdentitiesDict.put("N", newIdents);
	
	    //
	    // Turn the identity into a Proxy and return the Proxy to
	    // the caller.
	    //
	    return ContactPrxHelper.uncheckedCast(_adapter.createProxy(ident));
	}
	catch(Freeze.DBException ex)
	{
	    DatabaseException e = new DatabaseException();
	    e.message = ex.message;
	    throw e;
	}
    }

    public synchronized ContactPrx[]
    findContacts(String name, Ice.Current current)
	throws DatabaseException
    {
	try
	{
	    //
	    // Lookup all phone book contacts that match a name, and
	    // return them to the caller. See the comment in
	    // getNewIdentity why the prefix "N" is needed.
	    //
	    Ice.Identity[] identities = (Ice.Identity[])_nameIdentitiesDict.get("N" + name);

	    int length = (identities == null) ? 0 : identities.length;
	    ContactPrx[] contacts = new ContactPrx[length];

	    if (identities != null)
	    {
		for (int i = 0; i < length; ++i)
		{
		    contacts[i] = ContactPrxHelper.uncheckedCast(_adapter.createProxy(identities[i]));
		}
	    }

	    return contacts;
	}
	catch(Freeze.DBException ex)
	{
	    DatabaseException e = new DatabaseException();
	    e.message = ex.message;
	    throw e;
	}
    }

    public void
    setEvictorSize(int size, Ice.Current current)
	throws DatabaseException
    {
	//
	// No synchronization necessary, _evictor is immutable.
	//
	_evictor.setSize(size);
    }

    public void
    shutdown(Ice.Current current)
    {
	//
	// No synchronization necessary, _adapter is immutable.
	//
	_adapter.getCommunicator().shutdown();
    }

    protected synchronized void
    remove(Ice.Identity ident, String name)
	throws DatabaseException
    {
	try
	{
	    removeI(ident, name);
	}
	catch(Freeze.DBException ex)
	{
	    DatabaseException e = new DatabaseException();
	    e.message = ex.message;
	    throw e;
	}
    }

    protected synchronized void
    move(Ice.Identity ident, String oldName, String newName)
	throws DatabaseException
    {
	try
	{
	    //
	    // Called by ContactI in case the name has been
	    // changed. See the comment in getNewIdentity why the
	    // prefix "N" is needed.
	    //
	    removeI(ident, oldName);

	    Ice.Identity[] identities = (Ice.Identity[])_nameIdentitiesDict.get("N" + newName);
	    int length = (identities == null) ? 0 : identities.length;
	    Ice.Identity[] newIdents = new Ice.Identity[length+1];

	    if (identities != null)
	    {
		System.arraycopy(identities, 0, newIdents, 0, length);
	    }
	    newIdents[length] = ident;
	
	    _nameIdentitiesDict.put("N" + newName, newIdents);
	}
	catch(Freeze.DBNotFoundException ex)
	{
	    //
	    // Raised by remove. This should only happen under very
	    // rare circumstances if destroy() had gotten to the
	    // object prior to the setName() operation being
	    // dispatched. Ignore the exception.
	    //

	    //ex.printStackTrace();
	    //System.out.println(ex);
	}
	catch(Freeze.DBException ex)
	{
	    DatabaseException e = new DatabaseException();
	    e.message = ex.message;
	    throw e;
	}
    }

    protected synchronized Ice.Identity
    getNewIdentity()
	throws DatabaseException
    {
	try
	{
	    //
	    // This code is a bit of a hack. It stores the last
	    // identity that has been used (or the name component
	    // thereof, to be more precise) in the
	    // _nameIdentitiesDict, with the special prefix
	    // "ID". Because of this, all "real" names need to be
	    // prefixed with "N", so that there is no potential for a
	    // name clash.
	    //

	    long n = 0;
	    Ice.Identity[] ids = (Ice.Identity[])_nameIdentitiesDict.get("ID");
	    if (ids != null)
	    {
		assert(ids.length == 1);
		try
		{
		    n = Long.parseLong(ids[0].name) + 1;
		}
		catch (NumberFormatException ex)
		{
		    // TODO: Do anything?
		}
	    }
	    else
	    {
		ids = new Ice.Identity[1];
		ids[0] = new Ice.Identity();
		ids[0].category = new String();
	    }

	    String s = new Long(n).toString();

	    ids[0].name = s;
    
	    _nameIdentitiesDict.put("ID", ids);

	    Ice.Identity id = new Ice.Identity();
	    id.name = s;
	    id.category = "contact";
	    return id;
	}
	catch(Freeze.DBException ex)
	{
	    DatabaseException e = new DatabaseException();
	    e.message = ex.message;
	    throw e;
	}
    }

    PhoneBookI(Ice.ObjectAdapter adapter, Freeze.DB db, Freeze.Evictor evictor)
    {
	_adapter = adapter;
	_evictor = evictor;
	_nameIdentitiesDict = new NameIdentitiesDict(db);
    }

    private void
    removeI(Ice.Identity ident, String name)
    {
	//
	// See the comment in getNewIdentity why the prefix "N" is
	// needed.
	//
	String key = "N" + name;
	Ice.Identity[] identities = (Ice.Identity[])_nameIdentitiesDict.get(key);

	//
	// If the name isn't found then raise a record not found
	// exception.
	//
	if (identities == null)
	{
	    throw new Freeze.DBNotFoundException();
	}

	int i;
	for (i = 0; i < identities.length; ++i)
	{
	    if (identities[i].equals(ident))
	    {
		break;
	    }
	}

	if (i >= identities.length)
	{
	    throw new Freeze.DBNotFoundException();
	}

	if (identities.length == 1)
	{
	    _nameIdentitiesDict.remove(key);
	}
	else
	{
	    Ice.Identity[] newIdents = new Ice.Identity[identities.length-1];
	    System.arraycopy(identities, 0, newIdents, 0, i);
	    if (i < newIdents.length - 1)
	    {
		System.arraycopy(identities, i+1, newIdents, i, identities.length - i - 1);
	    }
	    
	    //
	    // See the comment in getNewIdentity why the prefix "N" is
	    // needed.
	    //
	    _nameIdentitiesDict.put(key, newIdents);
	}
    }

    private Ice.ObjectAdapter _adapter;
    private Freeze.DB _db;
    private Freeze.Evictor _evictor;
    private NameIdentitiesDict _nameIdentitiesDict;
}

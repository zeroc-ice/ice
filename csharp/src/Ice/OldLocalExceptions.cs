// Copyright (c) ZeroC, Inc.

namespace Ice;

/// <summary>
/// An attempt was made to register something more than once with the Ice run time.
/// This exception is raised if an
///  attempt is made to register a servant, servant locator, facet, value factory, plug-in, object adapter, object, or
///  user exception factory more than once for the same ID.
/// </summary>
public class AlreadyRegisteredException : LocalException
{
    public string kindOfObject;
    public string id;

    private void _initDM()
    {
        this.kindOfObject = "";
        this.id = "";
    }

    public AlreadyRegisteredException()
    {
        _initDM();
    }

    private void _initDM(string kindOfObject, string id)
    {
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    public AlreadyRegisteredException(string kindOfObject, string id)
    {
        _initDM(kindOfObject, id);
    }

    public override string ice_id()
    {
        return "::Ice::AlreadyRegisteredException";
    }
}

/// <summary>
/// An attempt was made to find or deregister something that is not registered with the Ice run time or Ice locator.
/// This exception is raised if an attempt is made to remove a servant, servant locator, facet, value factory, plug-in,
///  object adapter, object, or user exception factory that is not currently registered. It's also raised if the Ice
///  locator can't find an object or object adapter when resolving an indirect proxy or when an object adapter is
///  activated.
/// </summary>
public class NotRegisteredException : LocalException
{
    public string kindOfObject;
    public string id;

    private void _initDM()
    {
        this.kindOfObject = "";
        this.id = "";
    }

    public NotRegisteredException()
    {
        _initDM();
    }

    public NotRegisteredException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string kindOfObject, string id)
    {
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    public NotRegisteredException(string kindOfObject, string id)
    {
        _initDM(kindOfObject, id);
    }

    public override string ice_id()
    {
        return "::Ice::NotRegisteredException";
    }
}

/// <summary>
/// This exception is raised if an ObjectAdapter cannot be activated.
/// This happens if the Locator
///  detects another active ObjectAdapter with the same adapter id.
/// </summary>
public class ObjectAdapterIdInUseException : LocalException
{
    public string id;

    private void _initDM()
    {
        this.id = "";
    }

    public ObjectAdapterIdInUseException()
    {
        _initDM();
    }

    public override string ice_id()
    {
        return "::Ice::ObjectAdapterIdInUseException";
    }
}

/// <summary>
/// This exception is raised by an operation call if the application closes the connection locally using
///  Connection.close.
/// </summary>
public class ConnectionManuallyClosedException : LocalException
{
    public bool graceful;

    private void _initDM(bool graceful)
    {
        this.graceful = graceful;
    }

    public ConnectionManuallyClosedException(bool graceful)
    {
        _initDM(graceful);
    }

    public override string ice_id()
    {
        return "::Ice::ConnectionManuallyClosedException";
    }
}

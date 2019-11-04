//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    /// <summary>
    /// This exception is raised when a failure occurs during initialization.
    /// </summary>
    [global::System.Serializable]
    public partial class InitializationException : LocalException
    {
        public string reason;

        private void _initDM()
        {
            this.reason = "";
        }

        public InitializationException()
        {
            _initDM();
        }

        public InitializationException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public InitializationException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.reason = info.GetString("reason");
        }

        private void _initDM(string reason)
        {
            this.reason = reason;
        }

        public InitializationException(string reason)
        {
            _initDM(reason);
        }

        public InitializationException(string reason, global::System.Exception ex) : base(ex)
        {
            _initDM(reason);
        }

        public override string ice_id()
        {
            return "::Ice::InitializationException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::InitializationException");
            global::IceInternal.HashUtil.hashAdd(ref h_, reason);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            InitializationException o = other as InitializationException;
            if (o == null)
            {
                return false;
            }
            if (this.reason == null)
            {
                if (o.reason != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.reason.Equals(o.reason))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("reason", this.reason == null ? "" : this.reason);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(InitializationException lhs, InitializationException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(InitializationException lhs, InitializationException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// This exception indicates that a failure occurred while initializing
    /// a plug-in.
    /// </summary>
    [global::System.Serializable]
    public partial class PluginInitializationException : LocalException
    {
        public string reason;
        private void _initDM()
        {
            this.reason = "";
        }

        public PluginInitializationException()
        {
            _initDM();
        }

        public PluginInitializationException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public PluginInitializationException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.reason = info.GetString("reason");
        }

        private void _initDM(string reason)
        {
            this.reason = reason;
        }

        public PluginInitializationException(string reason)
        {
            _initDM(reason);
        }

        public PluginInitializationException(string reason, global::System.Exception ex) : base(ex)
        {
            _initDM(reason);
        }

        public override string ice_id()
        {
            return "::Ice::PluginInitializationException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::PluginInitializationException");
            global::IceInternal.HashUtil.hashAdd(ref h_, reason);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            PluginInitializationException o = other as PluginInitializationException;
            if (o == null)
            {
                return false;
            }
            if (this.reason == null)
            {
                if (o.reason != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.reason.Equals(o.reason))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("reason", this.reason == null ? "" : this.reason);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(PluginInitializationException lhs, PluginInitializationException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(PluginInitializationException lhs, PluginInitializationException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// An attempt was made to register something more than once with
    /// the Ice run time.
    /// This exception is raised if an attempt is made to register a
    /// servant, servant locator, facet, value factory, plug-in, object
    /// adapter, object, or user exception factory more than once for the
    /// same ID.
    /// </summary>

    [global::System.Serializable]
    public partial class AlreadyRegisteredException : LocalException
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

        public AlreadyRegisteredException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public AlreadyRegisteredException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.kindOfObject = info.GetString("kindOfObject");
            this.id = info.GetString("id");
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

        public AlreadyRegisteredException(string kindOfObject, string id, global::System.Exception ex) : base(ex)
        {
            _initDM(kindOfObject, id);
        }

        public override string ice_id()
        {
            return "::Ice::AlreadyRegisteredException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::AlreadyRegisteredException");
            global::IceInternal.HashUtil.hashAdd(ref h_, kindOfObject);
            global::IceInternal.HashUtil.hashAdd(ref h_, id);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            AlreadyRegisteredException o = other as AlreadyRegisteredException;
            if (o == null)
            {
                return false;
            }
            if (this.kindOfObject == null)
            {
                if (o.kindOfObject != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.kindOfObject.Equals(o.kindOfObject))
                {
                    return false;
                }
            }
            if (this.id == null)
            {
                if (o.id != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.id.Equals(o.id))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("kindOfObject", this.kindOfObject == null ? "" : this.kindOfObject);
            info.AddValue("id", this.id == null ? "" : this.id);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(AlreadyRegisteredException lhs, AlreadyRegisteredException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(AlreadyRegisteredException lhs, AlreadyRegisteredException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// An attempt was made to find or deregister something that is not
    /// registered with the Ice run time or Ice locator.
    /// This exception is raised if an attempt is made to remove a servant,
    /// servant locator, facet, value factory, plug-in, object adapter,
    /// object, or user exception factory that is not currently registered.
    ///
    /// It's also raised if the Ice locator can't find an object or object
    /// adapter when resolving an indirect proxy or when an object adapter
    /// is activated.
    /// </summary>

    [global::System.Serializable]
    public partial class NotRegisteredException : LocalException
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

        public NotRegisteredException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public NotRegisteredException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.kindOfObject = info.GetString("kindOfObject");
            this.id = info.GetString("id");
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

        public NotRegisteredException(string kindOfObject, string id, global::System.Exception ex) : base(ex)
        {
            _initDM(kindOfObject, id);
        }

        public override string ice_id()
        {
            return "::Ice::NotRegisteredException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::NotRegisteredException");
            global::IceInternal.HashUtil.hashAdd(ref h_, kindOfObject);
            global::IceInternal.HashUtil.hashAdd(ref h_, id);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            NotRegisteredException o = other as NotRegisteredException;
            if (o == null)
            {
                return false;
            }
            if (this.kindOfObject == null)
            {
                if (o.kindOfObject != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.kindOfObject.Equals(o.kindOfObject))
                {
                    return false;
                }
            }
            if (this.id == null)
            {
                if (o.id != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.id.Equals(o.id))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("kindOfObject", this.kindOfObject == null ? "" : this.kindOfObject);
            info.AddValue("id", this.id == null ? "" : this.id);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(NotRegisteredException lhs, NotRegisteredException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(NotRegisteredException lhs, NotRegisteredException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// The operation can only be invoked with a twoway request.
    /// This exception is raised if an attempt is made to invoke an
    /// operation with ice_oneway or ice_datagram and the operation has a return value,
    /// out-parameters, or an exception specification.
    /// </summary>

    [global::System.Serializable]
    public partial class TwowayOnlyException : LocalException
    {

        public string operation;

        private void _initDM()
        {
            this.operation = "";
        }

        public TwowayOnlyException()
        {
            _initDM();
        }

        public TwowayOnlyException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public TwowayOnlyException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.operation = info.GetString("operation");
        }

        private void _initDM(string operation)
        {
            this.operation = operation;
        }

        public TwowayOnlyException(string operation)
        {
            _initDM(operation);
        }

        public TwowayOnlyException(string operation, global::System.Exception ex) : base(ex)
        {
            _initDM(operation);
        }

        public override string ice_id()
        {
            return "::Ice::TwowayOnlyException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::TwowayOnlyException");
            global::IceInternal.HashUtil.hashAdd(ref h_, operation);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            TwowayOnlyException o = other as TwowayOnlyException;
            if (o == null)
            {
                return false;
            }
            if (this.operation == null)
            {
                if (o.operation != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.operation.Equals(o.operation))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("operation", this.operation == null ? "" : this.operation);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(TwowayOnlyException lhs, TwowayOnlyException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(TwowayOnlyException lhs, TwowayOnlyException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// An attempt was made to clone a class that does not support
    /// cloning.
    /// This exception is raised if ice_clone is called on
    /// a class that is derived from an abstract Slice class (that is,
    /// a class containing operations), and the derived class does not
    /// provide an implementation of the ice_clone operation (C++ only).
    /// </summary>

    [global::System.Serializable]
    public partial class CloneNotImplementedException : LocalException
    {

        public CloneNotImplementedException()
        {
        }

        public CloneNotImplementedException(global::System.Exception ex) : base(ex)
        {
        }

        public CloneNotImplementedException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::CloneNotImplementedException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::CloneNotImplementedException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            CloneNotImplementedException o = other as CloneNotImplementedException;
            if (o == null)
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(CloneNotImplementedException lhs, CloneNotImplementedException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(CloneNotImplementedException lhs, CloneNotImplementedException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if an operation call on a server raises an
    /// unknown exception.
    /// For example, for C++, this exception is raised
    /// if the server throws a C++ exception that is not directly or
    /// indirectly derived from Ice::LocalException or
    /// Ice::UserException.
    /// </summary>

    [global::System.Serializable]
    public partial class UnknownException : LocalException
    {

        public string unknown;

        private void _initDM()
        {
            this.unknown = "";
        }

        public UnknownException()
        {
            _initDM();
        }

        public UnknownException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public UnknownException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.unknown = info.GetString("unknown");
        }

        private void _initDM(string unknown)
        {
            this.unknown = unknown;
        }

        public UnknownException(string unknown)
        {
            _initDM(unknown);
        }

        public UnknownException(string unknown, global::System.Exception ex) : base(ex)
        {
            _initDM(unknown);
        }

        public override string ice_id()
        {
            return "::Ice::UnknownException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::UnknownException");
            global::IceInternal.HashUtil.hashAdd(ref h_, unknown);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            UnknownException o = other as UnknownException;
            if (o == null)
            {
                return false;
            }
            if (this.unknown == null)
            {
                if (o.unknown != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.unknown.Equals(o.unknown))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("unknown", this.unknown == null ? "" : this.unknown);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(UnknownException lhs, UnknownException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(UnknownException lhs, UnknownException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if an operation call on a server raises a
    /// local exception.
    /// Because local exceptions are not transmitted by
    /// the Ice protocol, the client receives all local exceptions raised
    /// by the server as UnknownLocalException. The only exception to this
    /// rule are all exceptions derived from RequestFailedException,
    /// which are transmitted by the Ice protocol even though they are
    /// declared local.
    /// </summary>

    [global::System.Serializable]
    public partial class UnknownLocalException : UnknownException
    {

        public UnknownLocalException()
        {
        }

        public UnknownLocalException(global::System.Exception ex) : base(ex)
        {
        }

        public UnknownLocalException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public UnknownLocalException(string unknown) : base(unknown)
        {
        }

        public UnknownLocalException(string unknown, global::System.Exception ex) : base(unknown, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::UnknownLocalException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::UnknownLocalException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            UnknownLocalException o = other as UnknownLocalException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(UnknownLocalException lhs, UnknownLocalException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(UnknownLocalException lhs, UnknownLocalException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// An operation raised an incorrect user exception.
    /// This exception is raised if an operation raises a
    /// user exception that is not declared in the exception's
    /// throws clause. Such undeclared exceptions are
    /// not transmitted from the server to the client by the Ice
    /// protocol, but instead the client just gets an
    /// UnknownUserException. This is necessary in order to not violate
    /// the contract established by an operation's signature: Only local
    /// exceptions and user exceptions declared in the
    /// throws clause can be raised.
    /// </summary>

    [global::System.Serializable]
    public partial class UnknownUserException : UnknownException
    {

        public UnknownUserException()
        {
        }

        public UnknownUserException(global::System.Exception ex) : base(ex)
        {
        }

        public UnknownUserException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public UnknownUserException(string unknown) : base(unknown)
        {
        }

        public UnknownUserException(string unknown, global::System.Exception ex) : base(unknown, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::UnknownUserException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::UnknownUserException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            UnknownUserException o = other as UnknownUserException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(UnknownUserException lhs, UnknownUserException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(UnknownUserException lhs, UnknownUserException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if the Ice library version does not match
    /// the version in the Ice header files.
    /// </summary>

    [global::System.Serializable]
    public partial class VersionMismatchException : LocalException
    {

        public VersionMismatchException()
        {
        }

        public VersionMismatchException(global::System.Exception ex) : base(ex)
        {
        }

        public VersionMismatchException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::VersionMismatchException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::VersionMismatchException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            VersionMismatchException o = other as VersionMismatchException;
            if (o == null)
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(VersionMismatchException lhs, VersionMismatchException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(VersionMismatchException lhs, VersionMismatchException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if the Communicator has been destroyed.
    /// </summary>

    [global::System.Serializable]
    public partial class CommunicatorDestroyedException : LocalException
    {

        public CommunicatorDestroyedException()
        {
        }

        public CommunicatorDestroyedException(global::System.Exception ex) : base(ex)
        {
        }

        public CommunicatorDestroyedException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::CommunicatorDestroyedException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::CommunicatorDestroyedException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            CommunicatorDestroyedException o = other as CommunicatorDestroyedException;
            if (o == null)
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(CommunicatorDestroyedException lhs, CommunicatorDestroyedException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(CommunicatorDestroyedException lhs, CommunicatorDestroyedException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if an attempt is made to use a deactivated
    /// ObjectAdapter.
    /// </summary>

    [global::System.Serializable]
    public partial class ObjectAdapterDeactivatedException : LocalException
    {

        public string name;

        private void _initDM()
        {
            this.name = "";
        }

        public ObjectAdapterDeactivatedException()
        {
            _initDM();
        }

        public ObjectAdapterDeactivatedException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public ObjectAdapterDeactivatedException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.name = info.GetString("name");
        }

        private void _initDM(string name)
        {
            this.name = name;
        }

        public ObjectAdapterDeactivatedException(string name)
        {
            _initDM(name);
        }

        public ObjectAdapterDeactivatedException(string name, global::System.Exception ex) : base(ex)
        {
            _initDM(name);
        }

        public override string ice_id()
        {
            return "::Ice::ObjectAdapterDeactivatedException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ObjectAdapterDeactivatedException");
            global::IceInternal.HashUtil.hashAdd(ref h_, name);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ObjectAdapterDeactivatedException o = other as ObjectAdapterDeactivatedException;
            if (o == null)
            {
                return false;
            }
            if (this.name == null)
            {
                if (o.name != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.name.Equals(o.name))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("name", this.name == null ? "" : this.name);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(ObjectAdapterDeactivatedException lhs, ObjectAdapterDeactivatedException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ObjectAdapterDeactivatedException lhs, ObjectAdapterDeactivatedException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if an ObjectAdapter cannot be activated.
    /// This happens if the Locator detects another active ObjectAdapter with
    /// the same adapter id.
    /// </summary>

    [global::System.Serializable]
    public partial class ObjectAdapterIdInUseException : LocalException
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

        public ObjectAdapterIdInUseException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public ObjectAdapterIdInUseException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.id = info.GetString("id");
        }

        private void _initDM(string id)
        {
            this.id = id;
        }

        public ObjectAdapterIdInUseException(string id)
        {
            _initDM(id);
        }

        public ObjectAdapterIdInUseException(string id, global::System.Exception ex) : base(ex)
        {
            _initDM(id);
        }

        public override string ice_id()
        {
            return "::Ice::ObjectAdapterIdInUseException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ObjectAdapterIdInUseException");
            global::IceInternal.HashUtil.hashAdd(ref h_, id);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ObjectAdapterIdInUseException o = other as ObjectAdapterIdInUseException;
            if (o == null)
            {
                return false;
            }
            if (this.id == null)
            {
                if (o.id != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.id.Equals(o.id))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("id", this.id == null ? "" : this.id);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(ObjectAdapterIdInUseException lhs, ObjectAdapterIdInUseException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ObjectAdapterIdInUseException lhs, ObjectAdapterIdInUseException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if no suitable endpoint is available.
    /// </summary>

    [global::System.Serializable]
    public partial class NoEndpointException : LocalException
    {

        public string proxy;

        private void _initDM()
        {
            this.proxy = "";
        }

        public NoEndpointException()
        {
            _initDM();
        }

        public NoEndpointException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public NoEndpointException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.proxy = info.GetString("proxy");
        }

        private void _initDM(string proxy)
        {
            this.proxy = proxy;
        }

        public NoEndpointException(string proxy)
        {
            _initDM(proxy);
        }

        public NoEndpointException(string proxy, global::System.Exception ex) : base(ex)
        {
            _initDM(proxy);
        }

        public override string ice_id()
        {
            return "::Ice::NoEndpointException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::NoEndpointException");
            global::IceInternal.HashUtil.hashAdd(ref h_, proxy);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            NoEndpointException o = other as NoEndpointException;
            if (o == null)
            {
                return false;
            }
            if (this.proxy == null)
            {
                if (o.proxy != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.proxy.Equals(o.proxy))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("proxy", this.proxy == null ? "" : this.proxy);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(NoEndpointException lhs, NoEndpointException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(NoEndpointException lhs, NoEndpointException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if there was an error while parsing an
    /// endpoint.
    /// </summary>

    [global::System.Serializable]
    public partial class EndpointParseException : LocalException
    {

        public string str;

        private void _initDM()
        {
            this.str = "";
        }

        public EndpointParseException()
        {
            _initDM();
        }

        public EndpointParseException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public EndpointParseException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.str = info.GetString("str");
        }

        private void _initDM(string str)
        {
            this.str = str;
        }

        public EndpointParseException(string str)
        {
            _initDM(str);
        }

        public EndpointParseException(string str, global::System.Exception ex) : base(ex)
        {
            _initDM(str);
        }

        public override string ice_id()
        {
            return "::Ice::EndpointParseException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::EndpointParseException");
            global::IceInternal.HashUtil.hashAdd(ref h_, str);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            EndpointParseException o = other as EndpointParseException;
            if (o == null)
            {
                return false;
            }
            if (this.str == null)
            {
                if (o.str != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.str.Equals(o.str))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("str", this.str == null ? "" : this.str);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(EndpointParseException lhs, EndpointParseException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(EndpointParseException lhs, EndpointParseException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if there was an error while parsing an
    /// endpoint selection type.
    /// </summary>

    [global::System.Serializable]
    public partial class EndpointSelectionTypeParseException : LocalException
    {

        public string str;

        private void _initDM()
        {
            this.str = "";
        }

        public EndpointSelectionTypeParseException()
        {
            _initDM();
        }

        public EndpointSelectionTypeParseException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public EndpointSelectionTypeParseException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.str = info.GetString("str");
        }

        private void _initDM(string str)
        {
            this.str = str;
        }

        public EndpointSelectionTypeParseException(string str)
        {
            _initDM(str);
        }

        public EndpointSelectionTypeParseException(string str, global::System.Exception ex) : base(ex)
        {
            _initDM(str);
        }

        public override string ice_id()
        {
            return "::Ice::EndpointSelectionTypeParseException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::EndpointSelectionTypeParseException");
            global::IceInternal.HashUtil.hashAdd(ref h_, str);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            EndpointSelectionTypeParseException o = other as EndpointSelectionTypeParseException;
            if (o == null)
            {
                return false;
            }
            if (this.str == null)
            {
                if (o.str != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.str.Equals(o.str))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("str", this.str == null ? "" : this.str);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(EndpointSelectionTypeParseException lhs, EndpointSelectionTypeParseException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(EndpointSelectionTypeParseException lhs, EndpointSelectionTypeParseException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if there was an error while parsing a
    /// version.
    /// </summary>

    [global::System.Serializable]
    public partial class VersionParseException : LocalException
    {

        public string str;

        private void _initDM()
        {
            this.str = "";
        }

        public VersionParseException()
        {
            _initDM();
        }

        public VersionParseException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public VersionParseException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.str = info.GetString("str");
        }

        private void _initDM(string str)
        {
            this.str = str;
        }

        public VersionParseException(string str)
        {
            _initDM(str);
        }

        public VersionParseException(string str, global::System.Exception ex) : base(ex)
        {
            _initDM(str);
        }

        public override string ice_id()
        {
            return "::Ice::VersionParseException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::VersionParseException");
            global::IceInternal.HashUtil.hashAdd(ref h_, str);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            VersionParseException o = other as VersionParseException;
            if (o == null)
            {
                return false;
            }
            if (this.str == null)
            {
                if (o.str != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.str.Equals(o.str))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("str", this.str == null ? "" : this.str);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(VersionParseException lhs, VersionParseException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(VersionParseException lhs, VersionParseException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if there was an error while parsing a
    /// stringified identity.
    /// </summary>

    [global::System.Serializable]
    public partial class IdentityParseException : LocalException
    {

        public string str;

        private void _initDM()
        {
            this.str = "";
        }

        public IdentityParseException()
        {
            _initDM();
        }

        public IdentityParseException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public IdentityParseException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.str = info.GetString("str");
        }

        private void _initDM(string str)
        {
            this.str = str;
        }

        public IdentityParseException(string str)
        {
            _initDM(str);
        }

        public IdentityParseException(string str, global::System.Exception ex) : base(ex)
        {
            _initDM(str);
        }

        public override string ice_id()
        {
            return "::Ice::IdentityParseException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::IdentityParseException");
            global::IceInternal.HashUtil.hashAdd(ref h_, str);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            IdentityParseException o = other as IdentityParseException;
            if (o == null)
            {
                return false;
            }
            if (this.str == null)
            {
                if (o.str != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.str.Equals(o.str))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("str", this.str == null ? "" : this.str);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(IdentityParseException lhs, IdentityParseException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(IdentityParseException lhs, IdentityParseException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if there was an error while parsing a
    /// stringified proxy.
    /// </summary>

    [global::System.Serializable]
    public partial class ProxyParseException : LocalException
    {

        public string str;

        private void _initDM()
        {
            this.str = "";
        }

        public ProxyParseException()
        {
            _initDM();
        }

        public ProxyParseException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public ProxyParseException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.str = info.GetString("str");
        }

        private void _initDM(string str)
        {
            this.str = str;
        }

        public ProxyParseException(string str)
        {
            _initDM(str);
        }

        public ProxyParseException(string str, global::System.Exception ex) : base(ex)
        {
            _initDM(str);
        }

        public override string ice_id()
        {
            return "::Ice::ProxyParseException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ProxyParseException");
            global::IceInternal.HashUtil.hashAdd(ref h_, str);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ProxyParseException o = other as ProxyParseException;
            if (o == null)
            {
                return false;
            }
            if (this.str == null)
            {
                if (o.str != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.str.Equals(o.str))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("str", this.str == null ? "" : this.str);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(ProxyParseException lhs, ProxyParseException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ProxyParseException lhs, ProxyParseException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if an illegal identity is encountered.
    /// </summary>

    [global::System.Serializable]
    public partial class IllegalIdentityException : LocalException
    {

        public Identity id;

        private void _initDM()
        {
            this.id = new Identity();
        }

        public IllegalIdentityException()
        {
            _initDM();
        }

        public IllegalIdentityException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public IllegalIdentityException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.id = (Identity)info.GetValue("id", typeof(Identity));
        }

        private void _initDM(Identity id)
        {
            this.id = id;
        }

        public IllegalIdentityException(Identity id)
        {
            _initDM(id);
        }

        public IllegalIdentityException(Identity id, global::System.Exception ex) : base(ex)
        {
            _initDM(id);
        }

        public override string ice_id()
        {
            return "::Ice::IllegalIdentityException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::IllegalIdentityException");
            global::IceInternal.HashUtil.hashAdd(ref h_, id);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            IllegalIdentityException o = other as IllegalIdentityException;
            if (o == null)
            {
                return false;
            }
            if (this.id == null)
            {
                if (o.id != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.id.Equals(o.id))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("id", this.id, typeof(Identity));

            base.GetObjectData(info, context);
        }

        public static bool operator ==(IllegalIdentityException lhs, IllegalIdentityException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(IllegalIdentityException lhs, IllegalIdentityException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised to reject an illegal servant (typically
    /// a null servant)
    /// </summary>

    [global::System.Serializable]
    public partial class IllegalServantException : LocalException
    {

        public string reason;

        private void _initDM()
        {
            this.reason = "";
        }

        public IllegalServantException()
        {
            _initDM();
        }

        public IllegalServantException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public IllegalServantException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.reason = info.GetString("reason");
        }

        private void _initDM(string reason)
        {
            this.reason = reason;
        }

        public IllegalServantException(string reason)
        {
            _initDM(reason);
        }

        public IllegalServantException(string reason, global::System.Exception ex) : base(ex)
        {
            _initDM(reason);
        }

        public override string ice_id()
        {
            return "::Ice::IllegalServantException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::IllegalServantException");
            global::IceInternal.HashUtil.hashAdd(ref h_, reason);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            IllegalServantException o = other as IllegalServantException;
            if (o == null)
            {
                return false;
            }
            if (this.reason == null)
            {
                if (o.reason != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.reason.Equals(o.reason))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("reason", this.reason == null ? "" : this.reason);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(IllegalServantException lhs, IllegalServantException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(IllegalServantException lhs, IllegalServantException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if a request failed.
    /// This exception, and
    /// all exceptions derived from RequestFailedException, are
    /// transmitted by the Ice protocol, even though they are declared
    /// local.
    /// </summary>

    [global::System.Serializable]
    public partial class RequestFailedException : LocalException
    {

        public Identity id;

        public string facet;

        public string operation;

        private void _initDM()
        {
            this.id = new Identity();
            this.facet = "";
            this.operation = "";
        }

        public RequestFailedException()
        {
            _initDM();
        }

        public RequestFailedException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public RequestFailedException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.id = (Identity)info.GetValue("id", typeof(Identity));
            this.facet = info.GetString("facet");
            this.operation = info.GetString("operation");
        }

        private void _initDM(Identity id, string facet, string operation)
        {
            this.id = id;
            this.facet = facet;
            this.operation = operation;
        }

        public RequestFailedException(Identity id, string facet, string operation)
        {
            _initDM(id, facet, operation);
        }

        public RequestFailedException(Identity id, string facet, string operation, global::System.Exception ex) : base(ex)
        {
            _initDM(id, facet, operation);
        }

        public override string ice_id()
        {
            return "::Ice::RequestFailedException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::RequestFailedException");
            global::IceInternal.HashUtil.hashAdd(ref h_, id);
            global::IceInternal.HashUtil.hashAdd(ref h_, facet);
            global::IceInternal.HashUtil.hashAdd(ref h_, operation);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            RequestFailedException o = other as RequestFailedException;
            if (o == null)
            {
                return false;
            }
            if (this.id == null)
            {
                if (o.id != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.id.Equals(o.id))
                {
                    return false;
                }
            }
            if (this.facet == null)
            {
                if (o.facet != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.facet.Equals(o.facet))
                {
                    return false;
                }
            }
            if (this.operation == null)
            {
                if (o.operation != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.operation.Equals(o.operation))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("id", this.id, typeof(Identity));
            info.AddValue("facet", this.facet == null ? "" : this.facet);
            info.AddValue("operation", this.operation == null ? "" : this.operation);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(RequestFailedException lhs, RequestFailedException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(RequestFailedException lhs, RequestFailedException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if an object does not exist on the server,
    /// that is, if no facets with the given identity exist.
    /// </summary>

    [global::System.Serializable]
    public partial class ObjectNotExistException : RequestFailedException
    {

        public ObjectNotExistException()
        {
        }

        public ObjectNotExistException(global::System.Exception ex) : base(ex)
        {
        }

        public ObjectNotExistException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public ObjectNotExistException(Identity id, string facet, string operation) : base(id, facet, operation)
        {
        }

        public ObjectNotExistException(Identity id, string facet, string operation, global::System.Exception ex) : base(id, facet, operation, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::ObjectNotExistException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ObjectNotExistException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ObjectNotExistException o = other as ObjectNotExistException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(ObjectNotExistException lhs, ObjectNotExistException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ObjectNotExistException lhs, ObjectNotExistException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if no facet with the given name exists,
    /// but at least one facet with the given identity exists.
    /// </summary>

    [global::System.Serializable]
    public partial class FacetNotExistException : RequestFailedException
    {

        public FacetNotExistException()
        {
        }

        public FacetNotExistException(global::System.Exception ex) : base(ex)
        {
        }

        public FacetNotExistException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public FacetNotExistException(Identity id, string facet, string operation) : base(id, facet, operation)
        {
        }

        public FacetNotExistException(Identity id, string facet, string operation, global::System.Exception ex) : base(id, facet, operation, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::FacetNotExistException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::FacetNotExistException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            FacetNotExistException o = other as FacetNotExistException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(FacetNotExistException lhs, FacetNotExistException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(FacetNotExistException lhs, FacetNotExistException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if an operation for a given object does
    /// not exist on the server.
    /// Typically this is caused by either the
    /// client or the server using an outdated Slice specification.
    /// </summary>

    [global::System.Serializable]
    public partial class OperationNotExistException : RequestFailedException
    {

        public OperationNotExistException()
        {
        }

        public OperationNotExistException(global::System.Exception ex) : base(ex)
        {
        }

        public OperationNotExistException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public OperationNotExistException(Identity id, string facet, string operation) : base(id, facet, operation)
        {
        }

        public OperationNotExistException(Identity id, string facet, string operation, global::System.Exception ex) : base(id, facet, operation, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::OperationNotExistException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::OperationNotExistException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            OperationNotExistException o = other as OperationNotExistException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(OperationNotExistException lhs, OperationNotExistException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(OperationNotExistException lhs, OperationNotExistException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if a system error occurred in the server
    /// or client process.
    /// There are many possible causes for such a system
    /// exception. For details on the cause, SyscallException.error
    /// should be inspected.
    /// </summary>

    [global::System.Serializable]
    public partial class SyscallException : LocalException
    {

        public int error;

        private void _initDM()
        {
            this.error = 0;
        }

        public SyscallException()
        {
            _initDM();
        }

        public SyscallException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public SyscallException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.error = info.GetInt32("error");
        }

        private void _initDM(int error)
        {
            this.error = error;
        }

        public SyscallException(int error)
        {
            _initDM(error);
        }

        public SyscallException(int error, global::System.Exception ex) : base(ex)
        {
            _initDM(error);
        }

        public override string ice_id()
        {
            return "::Ice::SyscallException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::SyscallException");
            global::IceInternal.HashUtil.hashAdd(ref h_, error);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            SyscallException o = other as SyscallException;
            if (o == null)
            {
                return false;
            }
            if (!this.error.Equals(o.error))
            {
                return false;
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("error", this.error);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(SyscallException lhs, SyscallException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(SyscallException lhs, SyscallException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates socket errors.
    /// </summary>

    [global::System.Serializable]
    public partial class SocketException : SyscallException
    {

        public SocketException()
        {
        }

        public SocketException(global::System.Exception ex) : base(ex)
        {
        }

        public SocketException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public SocketException(int error) : base(error)
        {
        }

        public SocketException(int error, global::System.Exception ex) : base(error, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::SocketException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::SocketException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            SocketException o = other as SocketException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(SocketException lhs, SocketException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(SocketException lhs, SocketException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates CFNetwork errors.
    /// </summary>

    [global::System.Serializable]
    public partial class CFNetworkException : SocketException
    {

        public string domain;

        private void _initDM()
        {
            this.domain = "";
        }

        public CFNetworkException()
        {
            _initDM();
        }

        public CFNetworkException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public CFNetworkException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.domain = info.GetString("domain");
        }

        private void _initDM(string domain)
        {
            this.domain = domain;
        }

        public CFNetworkException(int error, string domain) : base(error)
        {
            _initDM(domain);
        }

        public CFNetworkException(int error, string domain, global::System.Exception ex) : base(error, ex)
        {
            _initDM(domain);
        }

        public override string ice_id()
        {
            return "::Ice::CFNetworkException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::CFNetworkException");
            global::IceInternal.HashUtil.hashAdd(ref h_, domain);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            CFNetworkException o = other as CFNetworkException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            if (this.domain == null)
            {
                if (o.domain != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.domain.Equals(o.domain))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("domain", this.domain == null ? "" : this.domain);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(CFNetworkException lhs, CFNetworkException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(CFNetworkException lhs, CFNetworkException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates file errors.
    /// </summary>

    [global::System.Serializable]
    public partial class FileException : SyscallException
    {

        public string path;

        private void _initDM()
        {
            this.path = "";
        }

        public FileException()
        {
            _initDM();
        }

        public FileException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public FileException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.path = info.GetString("path");
        }

        private void _initDM(string path)
        {
            this.path = path;
        }

        public FileException(int error, string path) : base(error)
        {
            _initDM(path);
        }

        public FileException(int error, string path, global::System.Exception ex) : base(error, ex)
        {
            _initDM(path);
        }

        public override string ice_id()
        {
            return "::Ice::FileException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::FileException");
            global::IceInternal.HashUtil.hashAdd(ref h_, path);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            FileException o = other as FileException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            if (this.path == null)
            {
                if (o.path != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.path.Equals(o.path))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("path", this.path == null ? "" : this.path);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(FileException lhs, FileException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(FileException lhs, FileException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates connection failures.
    /// </summary>

    [global::System.Serializable]
    public partial class ConnectFailedException : SocketException
    {

        public ConnectFailedException()
        {
        }

        public ConnectFailedException(global::System.Exception ex) : base(ex)
        {
        }

        public ConnectFailedException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public ConnectFailedException(int error) : base(error)
        {
        }

        public ConnectFailedException(int error, global::System.Exception ex) : base(error, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::ConnectFailedException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ConnectFailedException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ConnectFailedException o = other as ConnectFailedException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(ConnectFailedException lhs, ConnectFailedException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ConnectFailedException lhs, ConnectFailedException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates a connection failure for which
    /// the server host actively refuses a connection.
    /// </summary>

    [global::System.Serializable]
    public partial class ConnectionRefusedException : ConnectFailedException
    {

        public ConnectionRefusedException()
        {
        }

        public ConnectionRefusedException(global::System.Exception ex) : base(ex)
        {
        }

        public ConnectionRefusedException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public ConnectionRefusedException(int error) : base(error)
        {
        }

        public ConnectionRefusedException(int error, global::System.Exception ex) : base(error, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::ConnectionRefusedException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ConnectionRefusedException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ConnectionRefusedException o = other as ConnectionRefusedException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(ConnectionRefusedException lhs, ConnectionRefusedException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ConnectionRefusedException lhs, ConnectionRefusedException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates a lost connection.
    /// </summary>

    [global::System.Serializable]
    public partial class ConnectionLostException : SocketException
    {

        public ConnectionLostException()
        {
        }

        public ConnectionLostException(global::System.Exception ex) : base(ex)
        {
        }

        public ConnectionLostException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public ConnectionLostException(int error) : base(error)
        {
        }

        public ConnectionLostException(int error, global::System.Exception ex) : base(error, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::ConnectionLostException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ConnectionLostException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ConnectionLostException o = other as ConnectionLostException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(ConnectionLostException lhs, ConnectionLostException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ConnectionLostException lhs, ConnectionLostException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates a DNS problem.
    /// For details on the cause,
    /// DNSException.error should be inspected.
    /// </summary>

    [global::System.Serializable]
    public partial class DNSException : LocalException
    {

        public int error;

        public string host;

        private void _initDM()
        {
            this.error = 0;
            this.host = "";
        }

        public DNSException()
        {
            _initDM();
        }

        public DNSException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public DNSException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.error = info.GetInt32("error");
            this.host = info.GetString("host");
        }

        private void _initDM(int error, string host)
        {
            this.error = error;
            this.host = host;
        }

        public DNSException(int error, string host)
        {
            _initDM(error, host);
        }

        public DNSException(int error, string host, global::System.Exception ex) : base(ex)
        {
            _initDM(error, host);
        }

        public override string ice_id()
        {
            return "::Ice::DNSException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::DNSException");
            global::IceInternal.HashUtil.hashAdd(ref h_, error);
            global::IceInternal.HashUtil.hashAdd(ref h_, host);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            DNSException o = other as DNSException;
            if (o == null)
            {
                return false;
            }
            if (!this.error.Equals(o.error))
            {
                return false;
            }
            if (this.host == null)
            {
                if (o.host != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.host.Equals(o.host))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("error", this.error);
            info.AddValue("host", this.host == null ? "" : this.host);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(DNSException lhs, DNSException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(DNSException lhs, DNSException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates a request was interrupted.
    /// </summary>

    [global::System.Serializable]
    public partial class OperationInterruptedException : LocalException
    {

        public OperationInterruptedException()
        {
        }

        public OperationInterruptedException(global::System.Exception ex) : base(ex)
        {
        }

        public OperationInterruptedException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::OperationInterruptedException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::OperationInterruptedException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            OperationInterruptedException o = other as OperationInterruptedException;
            if (o == null)
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(OperationInterruptedException lhs, OperationInterruptedException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(OperationInterruptedException lhs, OperationInterruptedException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates a timeout condition.
    /// </summary>

    [global::System.Serializable]
    public partial class TimeoutException : LocalException
    {

        public TimeoutException()
        {
        }

        public TimeoutException(global::System.Exception ex) : base(ex)
        {
        }

        public TimeoutException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::TimeoutException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::TimeoutException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            TimeoutException o = other as TimeoutException;
            if (o == null)
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(TimeoutException lhs, TimeoutException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(TimeoutException lhs, TimeoutException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates a connection establishment timeout condition.
    /// </summary>

    [global::System.Serializable]
    public partial class ConnectTimeoutException : TimeoutException
    {

        public ConnectTimeoutException()
        {
        }

        public ConnectTimeoutException(global::System.Exception ex) : base(ex)
        {
        }

        public ConnectTimeoutException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::ConnectTimeoutException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ConnectTimeoutException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ConnectTimeoutException o = other as ConnectTimeoutException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(ConnectTimeoutException lhs, ConnectTimeoutException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ConnectTimeoutException lhs, ConnectTimeoutException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates a connection closure timeout condition.
    /// </summary>

    [global::System.Serializable]
    public partial class CloseTimeoutException : TimeoutException
    {

        public CloseTimeoutException()
        {
        }

        public CloseTimeoutException(global::System.Exception ex) : base(ex)
        {
        }

        public CloseTimeoutException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::CloseTimeoutException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::CloseTimeoutException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            CloseTimeoutException o = other as CloseTimeoutException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(CloseTimeoutException lhs, CloseTimeoutException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(CloseTimeoutException lhs, CloseTimeoutException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates that a connection has been shut down because it has been
    /// idle for some time.
    /// </summary>

    [global::System.Serializable]
    public partial class ConnectionTimeoutException : TimeoutException
    {

        public ConnectionTimeoutException()
        {
        }

        public ConnectionTimeoutException(global::System.Exception ex) : base(ex)
        {
        }

        public ConnectionTimeoutException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::ConnectionTimeoutException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ConnectionTimeoutException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ConnectionTimeoutException o = other as ConnectionTimeoutException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(ConnectionTimeoutException lhs, ConnectionTimeoutException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ConnectionTimeoutException lhs, ConnectionTimeoutException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates that an invocation failed because it timed
    /// out.
    /// </summary>

    [global::System.Serializable]
    public partial class InvocationTimeoutException : TimeoutException
    {

        public InvocationTimeoutException()
        {
        }

        public InvocationTimeoutException(global::System.Exception ex) : base(ex)
        {
        }

        public InvocationTimeoutException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::InvocationTimeoutException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::InvocationTimeoutException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            InvocationTimeoutException o = other as InvocationTimeoutException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(InvocationTimeoutException lhs, InvocationTimeoutException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(InvocationTimeoutException lhs, InvocationTimeoutException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates that an asynchronous invocation failed
    /// because it was canceled explicitly by the user.
    /// </summary>

    [global::System.Serializable]
    public partial class InvocationCanceledException : LocalException
    {

        public InvocationCanceledException()
        {
        }

        public InvocationCanceledException(global::System.Exception ex) : base(ex)
        {
        }

        public InvocationCanceledException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::InvocationCanceledException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::InvocationCanceledException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            InvocationCanceledException o = other as InvocationCanceledException;
            if (o == null)
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(InvocationCanceledException lhs, InvocationCanceledException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(InvocationCanceledException lhs, InvocationCanceledException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// A generic exception base for all kinds of protocol error
    /// conditions.
    /// </summary>

    [global::System.Serializable]
    public partial class ProtocolException : LocalException
    {

        public string reason;

        private void _initDM()
        {
            this.reason = "";
        }

        public ProtocolException()
        {
            _initDM();
        }

        public ProtocolException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public ProtocolException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.reason = info.GetString("reason");
        }

        private void _initDM(string reason)
        {
            this.reason = reason;
        }

        public ProtocolException(string reason)
        {
            _initDM(reason);
        }

        public ProtocolException(string reason, global::System.Exception ex) : base(ex)
        {
            _initDM(reason);
        }

        public override string ice_id()
        {
            return "::Ice::ProtocolException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ProtocolException");
            global::IceInternal.HashUtil.hashAdd(ref h_, reason);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ProtocolException o = other as ProtocolException;
            if (o == null)
            {
                return false;
            }
            if (this.reason == null)
            {
                if (o.reason != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.reason.Equals(o.reason))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("reason", this.reason == null ? "" : this.reason);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(ProtocolException lhs, ProtocolException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ProtocolException lhs, ProtocolException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates that a message did not start with the expected
    /// magic number ('I', 'c', 'e', 'P').
    /// </summary>

    [global::System.Serializable]
    public partial class BadMagicException : ProtocolException
    {

        public byte[] badMagic;

        public BadMagicException()
        {
        }

        public BadMagicException(global::System.Exception ex) : base(ex)
        {
        }

        public BadMagicException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.badMagic = (byte[])info.GetValue("badMagic", typeof(byte[]));
        }

        private void _initDM(byte[] badMagic)
        {
            this.badMagic = badMagic;
        }

        public BadMagicException(string reason, byte[] badMagic) : base(reason)
        {
            _initDM(badMagic);
        }

        public BadMagicException(string reason, byte[] badMagic, global::System.Exception ex) : base(reason, ex)
        {
            _initDM(badMagic);
        }

        public override string ice_id()
        {
            return "::Ice::BadMagicException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::BadMagicException");
            global::IceInternal.HashUtil.hashAdd(ref h_, badMagic);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            BadMagicException o = other as BadMagicException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            if (this.badMagic == null)
            {
                if (o.badMagic != null)
                {
                    return false;
                }
            }
            else
            {
                if (!IceUtilInternal.Arrays.Equals(this.badMagic, o.badMagic))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("badMagic", this.badMagic, typeof(byte[]));

            base.GetObjectData(info, context);
        }

        public static bool operator ==(BadMagicException lhs, BadMagicException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(BadMagicException lhs, BadMagicException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates an unsupported protocol version.
    /// </summary>

    [global::System.Serializable]
    public partial class UnsupportedProtocolException : ProtocolException
    {

        public ProtocolVersion bad;

        public ProtocolVersion supported;

        private void _initDM()
        {
            this.bad = new ProtocolVersion();
            this.supported = new ProtocolVersion();
        }

        public UnsupportedProtocolException()
        {
            _initDM();
        }

        public UnsupportedProtocolException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public UnsupportedProtocolException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.bad = (ProtocolVersion)info.GetValue("bad", typeof(ProtocolVersion));
            this.supported = (ProtocolVersion)info.GetValue("supported", typeof(ProtocolVersion));
        }

        private void _initDM(ProtocolVersion bad, ProtocolVersion supported)
        {
            this.bad = bad;
            this.supported = supported;
        }

        public UnsupportedProtocolException(string reason, ProtocolVersion bad, ProtocolVersion supported) : base(reason)
        {
            _initDM(bad, supported);
        }

        public UnsupportedProtocolException(string reason, ProtocolVersion bad, ProtocolVersion supported, global::System.Exception ex) : base(reason, ex)
        {
            _initDM(bad, supported);
        }

        public override string ice_id()
        {
            return "::Ice::UnsupportedProtocolException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::UnsupportedProtocolException");
            global::IceInternal.HashUtil.hashAdd(ref h_, bad);
            global::IceInternal.HashUtil.hashAdd(ref h_, supported);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            UnsupportedProtocolException o = other as UnsupportedProtocolException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            if (!this.bad.Equals(o.bad))
            {
                return false;
            }
            if (!this.supported.Equals(o.supported))
            {
                return false;
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("bad", this.bad, typeof(ProtocolVersion));
            info.AddValue("supported", this.supported, typeof(ProtocolVersion));

            base.GetObjectData(info, context);
        }

        public static bool operator ==(UnsupportedProtocolException lhs, UnsupportedProtocolException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(UnsupportedProtocolException lhs, UnsupportedProtocolException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates an unsupported data encoding version.
    /// </summary>

    [global::System.Serializable]
    public partial class UnsupportedEncodingException : ProtocolException
    {

        public EncodingVersion bad;

        public EncodingVersion supported;

        private void _initDM()
        {
            this.bad = new EncodingVersion();
            this.supported = new EncodingVersion();
        }

        public UnsupportedEncodingException()
        {
            _initDM();
        }

        public UnsupportedEncodingException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public UnsupportedEncodingException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.bad = (EncodingVersion)info.GetValue("bad", typeof(EncodingVersion));
            this.supported = (EncodingVersion)info.GetValue("supported", typeof(EncodingVersion));
        }

        private void _initDM(EncodingVersion bad, EncodingVersion supported)
        {
            this.bad = bad;
            this.supported = supported;
        }

        public UnsupportedEncodingException(string reason, EncodingVersion bad, EncodingVersion supported) : base(reason)
        {
            _initDM(bad, supported);
        }

        public UnsupportedEncodingException(string reason, EncodingVersion bad, EncodingVersion supported, global::System.Exception ex) : base(reason, ex)
        {
            _initDM(bad, supported);
        }

        public override string ice_id()
        {
            return "::Ice::UnsupportedEncodingException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::UnsupportedEncodingException");
            global::IceInternal.HashUtil.hashAdd(ref h_, bad);
            global::IceInternal.HashUtil.hashAdd(ref h_, supported);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            UnsupportedEncodingException o = other as UnsupportedEncodingException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            if (!this.bad.Equals(o.bad))
            {
                return false;
            }
            if (!this.supported.Equals(o.supported))
            {
                return false;
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("bad", this.bad, typeof(EncodingVersion));
            info.AddValue("supported", this.supported, typeof(EncodingVersion));

            base.GetObjectData(info, context);
        }

        public static bool operator ==(UnsupportedEncodingException lhs, UnsupportedEncodingException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(UnsupportedEncodingException lhs, UnsupportedEncodingException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates that an unknown protocol message has been received.
    /// </summary>

    [global::System.Serializable]
    public partial class UnknownMessageException : ProtocolException
    {

        public UnknownMessageException()
        {
        }

        public UnknownMessageException(global::System.Exception ex) : base(ex)
        {
        }

        public UnknownMessageException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public UnknownMessageException(string reason) : base(reason)
        {
        }

        public UnknownMessageException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::UnknownMessageException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::UnknownMessageException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            UnknownMessageException o = other as UnknownMessageException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(UnknownMessageException lhs, UnknownMessageException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(UnknownMessageException lhs, UnknownMessageException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if a message is received over a connection
    /// that is not yet validated.
    /// </summary>

    [global::System.Serializable]
    public partial class ConnectionNotValidatedException : ProtocolException
    {

        public ConnectionNotValidatedException()
        {
        }

        public ConnectionNotValidatedException(global::System.Exception ex) : base(ex)
        {
        }

        public ConnectionNotValidatedException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public ConnectionNotValidatedException(string reason) : base(reason)
        {
        }

        public ConnectionNotValidatedException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::ConnectionNotValidatedException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ConnectionNotValidatedException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ConnectionNotValidatedException o = other as ConnectionNotValidatedException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(ConnectionNotValidatedException lhs, ConnectionNotValidatedException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ConnectionNotValidatedException lhs, ConnectionNotValidatedException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates that a response for an unknown request ID has been
    /// received.
    /// </summary>

    [global::System.Serializable]
    public partial class UnknownRequestIdException : ProtocolException
    {

        public UnknownRequestIdException()
        {
        }

        public UnknownRequestIdException(global::System.Exception ex) : base(ex)
        {
        }

        public UnknownRequestIdException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public UnknownRequestIdException(string reason) : base(reason)
        {
        }

        public UnknownRequestIdException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::UnknownRequestIdException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::UnknownRequestIdException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            UnknownRequestIdException o = other as UnknownRequestIdException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(UnknownRequestIdException lhs, UnknownRequestIdException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(UnknownRequestIdException lhs, UnknownRequestIdException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates that an unknown reply status has been received.
    /// </summary>

    [global::System.Serializable]
    public partial class UnknownReplyStatusException : ProtocolException
    {

        public UnknownReplyStatusException()
        {
        }

        public UnknownReplyStatusException(global::System.Exception ex) : base(ex)
        {
        }

        public UnknownReplyStatusException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public UnknownReplyStatusException(string reason) : base(reason)
        {
        }

        public UnknownReplyStatusException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::UnknownReplyStatusException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::UnknownReplyStatusException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            UnknownReplyStatusException o = other as UnknownReplyStatusException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(UnknownReplyStatusException lhs, UnknownReplyStatusException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(UnknownReplyStatusException lhs, UnknownReplyStatusException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates that the connection has been gracefully shut down by the
    /// server.
    /// The operation call that caused this exception has not been
    /// executed by the server. In most cases you will not get this
    /// exception, because the client will automatically retry the
    /// operation call in case the server shut down the connection. However,
    /// if upon retry the server shuts down the connection again, and the
    /// retry limit has been reached, then this exception is propagated to
    /// the application code.
    /// </summary>

    [global::System.Serializable]
    public partial class CloseConnectionException : ProtocolException
    {

        public CloseConnectionException()
        {
        }

        public CloseConnectionException(global::System.Exception ex) : base(ex)
        {
        }

        public CloseConnectionException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public CloseConnectionException(string reason) : base(reason)
        {
        }

        public CloseConnectionException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::CloseConnectionException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::CloseConnectionException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            CloseConnectionException o = other as CloseConnectionException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(CloseConnectionException lhs, CloseConnectionException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(CloseConnectionException lhs, CloseConnectionException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// This exception is raised by an operation call if the application
    /// closes the connection locally using Connection.close.
    /// </summary>
    [global::System.Serializable]
    public partial class ConnectionManuallyClosedException : LocalException
    {
        public bool graceful;
        private void _initDM()
        {
        }

        public ConnectionManuallyClosedException()
        {
            _initDM();
        }

        public ConnectionManuallyClosedException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public ConnectionManuallyClosedException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.graceful = info.GetBoolean("graceful");
        }

        private void _initDM(bool graceful)
        {
            this.graceful = graceful;
        }

        public ConnectionManuallyClosedException(bool graceful)
        {
            _initDM(graceful);
        }

        public ConnectionManuallyClosedException(bool graceful, global::System.Exception ex) : base(ex)
        {
            _initDM(graceful);
        }

        public override string ice_id()
        {
            return "::Ice::ConnectionManuallyClosedException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ConnectionManuallyClosedException");
            global::IceInternal.HashUtil.hashAdd(ref h_, graceful);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ConnectionManuallyClosedException o = other as ConnectionManuallyClosedException;
            if (o == null)
            {
                return false;
            }
            if (!this.graceful.Equals(o.graceful))
            {
                return false;
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("graceful", this.graceful);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(ConnectionManuallyClosedException lhs, ConnectionManuallyClosedException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ConnectionManuallyClosedException lhs, ConnectionManuallyClosedException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// This exception indicates that a message size is less
    /// than the minimum required size.
    /// </summary>
    [global::System.Serializable]
    public partial class IllegalMessageSizeException : ProtocolException
    {
        public IllegalMessageSizeException()
        {
        }

        public IllegalMessageSizeException(global::System.Exception ex) : base(ex)
        {
        }

        public IllegalMessageSizeException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public IllegalMessageSizeException(string reason) : base(reason)
        {
        }

        public IllegalMessageSizeException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::IllegalMessageSizeException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::IllegalMessageSizeException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            IllegalMessageSizeException o = other as IllegalMessageSizeException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(IllegalMessageSizeException lhs, IllegalMessageSizeException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(IllegalMessageSizeException lhs, IllegalMessageSizeException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// This exception indicates a problem with compressing or uncompressing data.
    /// </summary>
    [global::System.Serializable]
    public partial class CompressionException : ProtocolException
    {
        public CompressionException()
        {
        }

        public CompressionException(global::System.Exception ex) : base(ex)
        {
        }

        public CompressionException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public CompressionException(string reason) : base(reason)
        {
        }

        public CompressionException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::CompressionException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::CompressionException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            CompressionException o = other as CompressionException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(CompressionException lhs, CompressionException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(CompressionException lhs, CompressionException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// A datagram exceeds the configured size.
    /// This exception is raised if a datagram exceeds the configured send or receive buffer
    /// size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
    /// </summary>
    [global::System.Serializable]
    public partial class DatagramLimitException : ProtocolException
    {
        public DatagramLimitException()
        {
        }

        public DatagramLimitException(global::System.Exception ex) : base(ex)
        {
        }

        public DatagramLimitException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public DatagramLimitException(string reason) : base(reason)
        {
        }

        public DatagramLimitException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::DatagramLimitException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::DatagramLimitException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            DatagramLimitException o = other as DatagramLimitException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(DatagramLimitException lhs, DatagramLimitException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(DatagramLimitException lhs, DatagramLimitException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// This exception is raised for errors during marshaling or unmarshaling data.
    /// </summary>
    [global::System.Serializable]
    public partial class MarshalException : ProtocolException
    {
        public MarshalException()
        {
        }

        public MarshalException(global::System.Exception ex) : base(ex)
        {
        }

        public MarshalException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public MarshalException(string reason) : base(reason)
        {
        }

        public MarshalException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::MarshalException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::MarshalException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            MarshalException o = other as MarshalException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(MarshalException lhs, MarshalException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(MarshalException lhs, MarshalException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// This exception is raised if inconsistent data is received while unmarshaling a proxy.
    /// </summary>
    [global::System.Serializable]
    public partial class ProxyUnmarshalException : MarshalException
    {
        public ProxyUnmarshalException()
        {
        }

        public ProxyUnmarshalException(global::System.Exception ex) : base(ex)
        {
        }

        public ProxyUnmarshalException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public ProxyUnmarshalException(string reason) : base(reason)
        {
        }

        public ProxyUnmarshalException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::ProxyUnmarshalException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ProxyUnmarshalException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ProxyUnmarshalException o = other as ProxyUnmarshalException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(ProxyUnmarshalException lhs, ProxyUnmarshalException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ProxyUnmarshalException lhs, ProxyUnmarshalException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// This exception is raised if an out-of-bounds condition occurs during unmarshaling.
    /// </summary>
    [global::System.Serializable]
    public partial class UnmarshalOutOfBoundsException : MarshalException
    {
        public UnmarshalOutOfBoundsException()
        {
        }

        public UnmarshalOutOfBoundsException(global::System.Exception ex) : base(ex)
        {
        }

        public UnmarshalOutOfBoundsException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public UnmarshalOutOfBoundsException(string reason) : base(reason)
        {
        }

        public UnmarshalOutOfBoundsException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::UnmarshalOutOfBoundsException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::UnmarshalOutOfBoundsException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            UnmarshalOutOfBoundsException o = other as UnmarshalOutOfBoundsException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(UnmarshalOutOfBoundsException lhs, UnmarshalOutOfBoundsException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(UnmarshalOutOfBoundsException lhs, UnmarshalOutOfBoundsException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// This exception is raised if no suitable value factory was found during
    /// unmarshaling of a Slice class instance.
    /// </summary>
    [global::System.Serializable]
    public partial class NoValueFactoryException : MarshalException
    {
        public string type;

        private void _initDM()
        {
            this.type = "";
        }

        public NoValueFactoryException()
        {
            _initDM();
        }

        public NoValueFactoryException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public NoValueFactoryException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.type = info.GetString("type");
        }

        private void _initDM(string type)
        {
            this.type = type;
        }

        public NoValueFactoryException(string reason, string type) : base(reason)
        {
            _initDM(type);
        }

        public NoValueFactoryException(string reason, string type, global::System.Exception ex) : base(reason, ex)
        {
            _initDM(type);
        }

        public override string ice_id()
        {
            return "::Ice::NoValueFactoryException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::NoValueFactoryException");
            global::IceInternal.HashUtil.hashAdd(ref h_, type);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            NoValueFactoryException o = other as NoValueFactoryException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            if (this.type == null)
            {
                if (o.type != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.type.Equals(o.type))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("type", this.type == null ? "" : this.type);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(NoValueFactoryException lhs, NoValueFactoryException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(NoValueFactoryException lhs, NoValueFactoryException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// This exception is raised if the type of an unmarshaled Slice class instance does
    /// not match its expected type.
    /// This can happen if client and server are compiled with mismatched Slice
    /// definitions or if a class of the wrong type is passed as a parameter
    /// or return value using dynamic invocation. This exception can also be
    /// raised if IceStorm is used to send Slice class instances and
    /// an operation is subscribed to the wrong topic.
    /// </summary>
    [global::System.Serializable]
    public partial class UnexpectedObjectException : MarshalException
    {
        public string type;
        public string expectedType;

        private void _initDM()
        {
            this.type = "";
            this.expectedType = "";
        }

        public UnexpectedObjectException()
        {
            _initDM();
        }

        public UnexpectedObjectException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public UnexpectedObjectException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.type = info.GetString("type");
            this.expectedType = info.GetString("expectedType");
        }

        private void _initDM(string type, string expectedType)
        {
            this.type = type;
            this.expectedType = expectedType;
        }

        public UnexpectedObjectException(string reason, string type, string expectedType) : base(reason)
        {
            _initDM(type, expectedType);
        }

        public UnexpectedObjectException(string reason, string type, string expectedType, global::System.Exception ex) : base(reason, ex)
        {
            _initDM(type, expectedType);
        }

        public override string ice_id()
        {
            return "::Ice::UnexpectedObjectException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::UnexpectedObjectException");
            global::IceInternal.HashUtil.hashAdd(ref h_, type);
            global::IceInternal.HashUtil.hashAdd(ref h_, expectedType);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            UnexpectedObjectException o = other as UnexpectedObjectException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            if (this.type == null)
            {
                if (o.type != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.type.Equals(o.type))
                {
                    return false;
                }
            }
            if (this.expectedType == null)
            {
                if (o.expectedType != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.expectedType.Equals(o.expectedType))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("type", this.type == null ? "" : this.type);
            info.AddValue("expectedType", this.expectedType == null ? "" : this.expectedType);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(UnexpectedObjectException lhs, UnexpectedObjectException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(UnexpectedObjectException lhs, UnexpectedObjectException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// This exception is raised when Ice receives a request or reply
    /// message whose size exceeds the limit specified by the
    /// Ice.MessageSizeMax property.
    /// </summary>
    [global::System.Serializable]
    public partial class MemoryLimitException : MarshalException
    {
        public MemoryLimitException()
        {
        }

        public MemoryLimitException(global::System.Exception ex) : base(ex)
        {
        }

        public MemoryLimitException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public MemoryLimitException(string reason) : base(reason)
        {
        }

        public MemoryLimitException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::MemoryLimitException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::MemoryLimitException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            MemoryLimitException o = other as MemoryLimitException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(MemoryLimitException lhs, MemoryLimitException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(MemoryLimitException lhs, MemoryLimitException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised when a string conversion to or from UTF-8
    /// fails during marshaling or unmarshaling.
    /// </summary>

    [global::System.Serializable]
    public partial class StringConversionException : MarshalException
    {

        public StringConversionException()
        {
        }

        public StringConversionException(global::System.Exception ex) : base(ex)
        {
        }

        public StringConversionException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public StringConversionException(string reason) : base(reason)
        {
        }

        public StringConversionException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::StringConversionException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::StringConversionException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            StringConversionException o = other as StringConversionException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(StringConversionException lhs, StringConversionException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(StringConversionException lhs, StringConversionException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates a malformed data encapsulation.
    /// </summary>

    [global::System.Serializable]
    public partial class EncapsulationException : MarshalException
    {

        public EncapsulationException()
        {
        }

        public EncapsulationException(global::System.Exception ex) : base(ex)
        {
        }

        public EncapsulationException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public EncapsulationException(string reason) : base(reason)
        {
        }

        public EncapsulationException(string reason, global::System.Exception ex) : base(reason, ex)
        {
        }

        public override string ice_id()
        {
            return "::Ice::EncapsulationException";
        }

        public override int GetHashCode()
        {
            int h_ = base.GetHashCode();
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::EncapsulationException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            EncapsulationException o = other as EncapsulationException;
            if (o == null)
            {
                return false;
            }
            if (!base.Equals(other))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(EncapsulationException lhs, EncapsulationException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(EncapsulationException lhs, EncapsulationException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception is raised if an unsupported feature is used.
    /// The
    /// unsupported feature string contains the name of the unsupported
    /// feature
    /// </summary>

    [global::System.Serializable]
    public partial class FeatureNotSupportedException : LocalException
    {

        public string unsupportedFeature;

        private void _initDM()
        {
            this.unsupportedFeature = "";
        }

        public FeatureNotSupportedException()
        {
            _initDM();
        }

        public FeatureNotSupportedException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public FeatureNotSupportedException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.unsupportedFeature = info.GetString("unsupportedFeature");
        }

        private void _initDM(string unsupportedFeature)
        {
            this.unsupportedFeature = unsupportedFeature;
        }

        public FeatureNotSupportedException(string unsupportedFeature)
        {
            _initDM(unsupportedFeature);
        }

        public FeatureNotSupportedException(string unsupportedFeature, global::System.Exception ex) : base(ex)
        {
            _initDM(unsupportedFeature);
        }

        public override string ice_id()
        {
            return "::Ice::FeatureNotSupportedException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::FeatureNotSupportedException");
            global::IceInternal.HashUtil.hashAdd(ref h_, unsupportedFeature);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            FeatureNotSupportedException o = other as FeatureNotSupportedException;
            if (o == null)
            {
                return false;
            }
            if (this.unsupportedFeature == null)
            {
                if (o.unsupportedFeature != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.unsupportedFeature.Equals(o.unsupportedFeature))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("unsupportedFeature", this.unsupportedFeature == null ? "" : this.unsupportedFeature);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(FeatureNotSupportedException lhs, FeatureNotSupportedException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(FeatureNotSupportedException lhs, FeatureNotSupportedException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates a failure in a security subsystem,
    /// such as the IceSSL plug-in.
    /// </summary>

    [global::System.Serializable]
    public partial class SecurityException : LocalException
    {

        public string reason;

        private void _initDM()
        {
            this.reason = "";
        }

        public SecurityException()
        {
            _initDM();
        }

        public SecurityException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public SecurityException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.reason = info.GetString("reason");
        }

        private void _initDM(string reason)
        {
            this.reason = reason;
        }

        public SecurityException(string reason)
        {
            _initDM(reason);
        }

        public SecurityException(string reason, global::System.Exception ex) : base(ex)
        {
            _initDM(reason);
        }

        public override string ice_id()
        {
            return "::Ice::SecurityException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::SecurityException");
            global::IceInternal.HashUtil.hashAdd(ref h_, reason);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            SecurityException o = other as SecurityException;
            if (o == null)
            {
                return false;
            }
            if (this.reason == null)
            {
                if (o.reason != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.reason.Equals(o.reason))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("reason", this.reason == null ? "" : this.reason);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(SecurityException lhs, SecurityException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(SecurityException lhs, SecurityException rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    /// <summary>
    /// This exception indicates that an attempt has been made to
    /// change the connection properties of a fixed proxy.
    /// </summary>
    [global::System.Serializable]
    public partial class FixedProxyException : LocalException
    {
        public FixedProxyException()
        {
        }

        public FixedProxyException(global::System.Exception ex) : base(ex)
        {
        }

        public FixedProxyException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::FixedProxyException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::FixedProxyException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            FixedProxyException o = other as FixedProxyException;
            if (o == null)
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(FixedProxyException lhs, FixedProxyException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(FixedProxyException lhs, FixedProxyException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    /// <summary>
    /// Indicates that the response to a request has already been sent;
    /// re-dispatching such a request is not possible.
    /// </summary>
    [global::System.Serializable]
    public partial class ResponseSentException : LocalException
    {
        public ResponseSentException()
        {
        }

        public ResponseSentException(global::System.Exception ex) : base(ex)
        {
        }

        public ResponseSentException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
        }

        public override string ice_id()
        {
            return "::Ice::ResponseSentException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ResponseSentException");
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            ResponseSentException o = other as ResponseSentException;
            if (o == null)
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(ResponseSentException lhs, ResponseSentException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ResponseSentException lhs, ResponseSentException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }
}

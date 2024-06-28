<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace
{
    require_once 'Ice/Identity.php';
    require_once 'Ice/Version.php';
    require_once 'Ice/BuiltinSequences.php';
}

namespace Ice
{
    global $Ice__t_InitializationException;
    class InitializationException extends \Ice\LocalException
    {
        public function __construct($reason='')
        {
            $this->reason = $reason;
        }

        public function ice_id()
        {
            return '::Ice::InitializationException';
        }

        public function __toString(): string
        {
            global $Ice__t_InitializationException;
            return IcePHP_stringifyException($this, $Ice__t_InitializationException);
        }

        public $reason;
    }
    global $IcePHP__t_string;

    $Ice__t_InitializationException = IcePHP_defineException('::Ice::InitializationException', '\\Ice\\InitializationException', null, array(
        array('reason', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_PluginInitializationException;
    class PluginInitializationException extends \Ice\LocalException
    {
        public function __construct($reason='')
        {
            $this->reason = $reason;
        }

        public function ice_id()
        {
            return '::Ice::PluginInitializationException';
        }

        public function __toString(): string
        {
            global $Ice__t_PluginInitializationException;
            return IcePHP_stringifyException($this, $Ice__t_PluginInitializationException);
        }

        public $reason;
    }
    global $IcePHP__t_string;

    $Ice__t_PluginInitializationException = IcePHP_defineException('::Ice::PluginInitializationException', '\\Ice\\PluginInitializationException', null, array(
        array('reason', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_AlreadyRegisteredException;
    class AlreadyRegisteredException extends \Ice\LocalException
    {
        public function __construct($kindOfObject='', $id='')
        {
            $this->kindOfObject = $kindOfObject;
            $this->id = $id;
        }

        public function ice_id()
        {
            return '::Ice::AlreadyRegisteredException';
        }

        public function __toString(): string
        {
            global $Ice__t_AlreadyRegisteredException;
            return IcePHP_stringifyException($this, $Ice__t_AlreadyRegisteredException);
        }

        public $kindOfObject;
        public $id;
    }
    global $IcePHP__t_string;

    $Ice__t_AlreadyRegisteredException = IcePHP_defineException('::Ice::AlreadyRegisteredException', '\\Ice\\AlreadyRegisteredException', null, array(
        array('kindOfObject', $IcePHP__t_string, false, 0),
        array('id', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_NotRegisteredException;
    class NotRegisteredException extends \Ice\LocalException
    {
        public function __construct($kindOfObject='', $id='')
        {
            $this->kindOfObject = $kindOfObject;
            $this->id = $id;
        }

        public function ice_id()
        {
            return '::Ice::NotRegisteredException';
        }

        public function __toString(): string
        {
            global $Ice__t_NotRegisteredException;
            return IcePHP_stringifyException($this, $Ice__t_NotRegisteredException);
        }

        public $kindOfObject;
        public $id;
    }
    global $IcePHP__t_string;

    $Ice__t_NotRegisteredException = IcePHP_defineException('::Ice::NotRegisteredException', '\\Ice\\NotRegisteredException', null, array(
        array('kindOfObject', $IcePHP__t_string, false, 0),
        array('id', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_TwowayOnlyException;
    class TwowayOnlyException extends \Ice\LocalException
    {
        public function __construct($operation='')
        {
            $this->operation = $operation;
        }

        public function ice_id()
        {
            return '::Ice::TwowayOnlyException';
        }

        public function __toString(): string
        {
            global $Ice__t_TwowayOnlyException;
            return IcePHP_stringifyException($this, $Ice__t_TwowayOnlyException);
        }

        public $operation;
    }
    global $IcePHP__t_string;

    $Ice__t_TwowayOnlyException = IcePHP_defineException('::Ice::TwowayOnlyException', '\\Ice\\TwowayOnlyException', null, array(
        array('operation', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_UnknownException;
    class UnknownException extends \Ice\LocalException
    {
        public function __construct($unknown='')
        {
            $this->unknown = $unknown;
        }

        public function ice_id()
        {
            return '::Ice::UnknownException';
        }

        public function __toString(): string
        {
            global $Ice__t_UnknownException;
            return IcePHP_stringifyException($this, $Ice__t_UnknownException);
        }

        public $unknown;
    }
    global $IcePHP__t_string;

    $Ice__t_UnknownException = IcePHP_defineException('::Ice::UnknownException', '\\Ice\\UnknownException', null, array(
        array('unknown', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_UnknownLocalException;
    class UnknownLocalException extends \Ice\UnknownException
    {
        public function __construct($unknown='')
        {
            parent::__construct($unknown);
        }

        public function ice_id()
        {
            return '::Ice::UnknownLocalException';
        }

        public function __toString(): string
        {
            global $Ice__t_UnknownLocalException;
            return IcePHP_stringifyException($this, $Ice__t_UnknownLocalException);
        }
    }

    $Ice__t_UnknownLocalException = IcePHP_defineException('::Ice::UnknownLocalException', '\\Ice\\UnknownLocalException', $Ice__t_UnknownException, null);
}

namespace Ice
{
    global $Ice__t_UnknownUserException;
    class UnknownUserException extends \Ice\UnknownException
    {
        public function __construct($unknown='')
        {
            parent::__construct($unknown);
        }

        public function ice_id()
        {
            return '::Ice::UnknownUserException';
        }

        public function __toString(): string
        {
            global $Ice__t_UnknownUserException;
            return IcePHP_stringifyException($this, $Ice__t_UnknownUserException);
        }
    }

    $Ice__t_UnknownUserException = IcePHP_defineException('::Ice::UnknownUserException', '\\Ice\\UnknownUserException', $Ice__t_UnknownException, null);
}

namespace Ice
{
    global $Ice__t_CommunicatorDestroyedException;
    class CommunicatorDestroyedException extends \Ice\LocalException
    {
        public function __construct()
        {
        }

        public function ice_id()
        {
            return '::Ice::CommunicatorDestroyedException';
        }

        public function __toString(): string
        {
            global $Ice__t_CommunicatorDestroyedException;
            return IcePHP_stringifyException($this, $Ice__t_CommunicatorDestroyedException);
        }
    }

    $Ice__t_CommunicatorDestroyedException = IcePHP_defineException('::Ice::CommunicatorDestroyedException', '\\Ice\\CommunicatorDestroyedException', null, null);
}

namespace Ice
{
    global $Ice__t_ObjectAdapterDeactivatedException;
    class ObjectAdapterDeactivatedException extends \Ice\LocalException
    {
        public function __construct($name='')
        {
            $this->name = $name;
        }

        public function ice_id()
        {
            return '::Ice::ObjectAdapterDeactivatedException';
        }

        public function __toString(): string
        {
            global $Ice__t_ObjectAdapterDeactivatedException;
            return IcePHP_stringifyException($this, $Ice__t_ObjectAdapterDeactivatedException);
        }

        public $name;
    }
    global $IcePHP__t_string;

    $Ice__t_ObjectAdapterDeactivatedException = IcePHP_defineException('::Ice::ObjectAdapterDeactivatedException', '\\Ice\\ObjectAdapterDeactivatedException', null, array(
        array('name', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_ObjectAdapterIdInUseException;
    class ObjectAdapterIdInUseException extends \Ice\LocalException
    {
        public function __construct($id='')
        {
            $this->id = $id;
        }

        public function ice_id()
        {
            return '::Ice::ObjectAdapterIdInUseException';
        }

        public function __toString(): string
        {
            global $Ice__t_ObjectAdapterIdInUseException;
            return IcePHP_stringifyException($this, $Ice__t_ObjectAdapterIdInUseException);
        }

        public $id;
    }
    global $IcePHP__t_string;

    $Ice__t_ObjectAdapterIdInUseException = IcePHP_defineException('::Ice::ObjectAdapterIdInUseException', '\\Ice\\ObjectAdapterIdInUseException', null, array(
        array('id', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_NoEndpointException;
    class NoEndpointException extends \Ice\LocalException
    {
        public function __construct($proxy='')
        {
            $this->proxy = $proxy;
        }

        public function ice_id()
        {
            return '::Ice::NoEndpointException';
        }

        public function __toString(): string
        {
            global $Ice__t_NoEndpointException;
            return IcePHP_stringifyException($this, $Ice__t_NoEndpointException);
        }

        public $proxy;
    }
    global $IcePHP__t_string;

    $Ice__t_NoEndpointException = IcePHP_defineException('::Ice::NoEndpointException', '\\Ice\\NoEndpointException', null, array(
        array('proxy', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_ParseException;
    class ParseException extends \Ice\LocalException
    {
        public function __construct($str='')
        {
            $this->str = $str;
        }

        public function ice_id()
        {
            return '::Ice::ParseException';
        }

        public function __toString(): string
        {
            global $Ice__t_ParseException;
            return IcePHP_stringifyException($this, $Ice__t_ParseException);
        }

        public $str;
    }
    global $IcePHP__t_string;

    $Ice__t_ParseException = IcePHP_defineException('::Ice::ParseException', '\\Ice\\ParseException', null, array(
        array('str', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_IllegalIdentityException;
    class IllegalIdentityException extends \Ice\LocalException
    {
        public function __construct($id=null)
        {
        }

        public function ice_id()
        {
            return '::Ice::IllegalIdentityException';
        }

        public function __toString(): string
        {
            global $Ice__t_IllegalIdentityException;
            return IcePHP_stringifyException($this, $Ice__t_IllegalIdentityException);
        }
    }

    $Ice__t_IllegalIdentityException = IcePHP_defineException('::Ice::IllegalIdentityException', '\\Ice\\IllegalIdentityException', null, null);
}

namespace Ice
{
    global $Ice__t_IllegalServantException;
    class IllegalServantException extends \Ice\LocalException
    {
        public function __construct($reason='')
        {
            $this->reason = $reason;
        }

        public function ice_id()
        {
            return '::Ice::IllegalServantException';
        }

        public function __toString(): string
        {
            global $Ice__t_IllegalServantException;
            return IcePHP_stringifyException($this, $Ice__t_IllegalServantException);
        }

        public $reason;
    }
    global $IcePHP__t_string;

    $Ice__t_IllegalServantException = IcePHP_defineException('::Ice::IllegalServantException', '\\Ice\\IllegalServantException', null, array(
        array('reason', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_RequestFailedException;
    class RequestFailedException extends \Ice\LocalException
    {
        public function __construct($id=null, $facet='', $operation='')
        {
            $this->id = is_null($id) ? new \Ice\Identity : $id;
            $this->facet = $facet;
            $this->operation = $operation;
        }

        public function ice_id()
        {
            return '::Ice::RequestFailedException';
        }

        public function __toString(): string
        {
            global $Ice__t_RequestFailedException;
            return IcePHP_stringifyException($this, $Ice__t_RequestFailedException);
        }

        public $id;
        public $facet;
        public $operation;
    }
    global $Ice__t_Identity;
    global $IcePHP__t_string;

    $Ice__t_RequestFailedException = IcePHP_defineException('::Ice::RequestFailedException', '\\Ice\\RequestFailedException', null, array(
        array('id', $Ice__t_Identity, false, 0),
        array('facet', $IcePHP__t_string, false, 0),
        array('operation', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_ObjectNotExistException;
    class ObjectNotExistException extends \Ice\RequestFailedException
    {
        public function __construct($id=null, $facet='', $operation='')
        {
            parent::__construct($id, $facet, $operation);
        }

        public function ice_id()
        {
            return '::Ice::ObjectNotExistException';
        }

        public function __toString(): string
        {
            global $Ice__t_ObjectNotExistException;
            return IcePHP_stringifyException($this, $Ice__t_ObjectNotExistException);
        }
    }

    $Ice__t_ObjectNotExistException = IcePHP_defineException('::Ice::ObjectNotExistException', '\\Ice\\ObjectNotExistException', $Ice__t_RequestFailedException, null);
}

namespace Ice
{
    global $Ice__t_FacetNotExistException;
    class FacetNotExistException extends \Ice\RequestFailedException
    {
        public function __construct($id=null, $facet='', $operation='')
        {
            parent::__construct($id, $facet, $operation);
        }

        public function ice_id()
        {
            return '::Ice::FacetNotExistException';
        }

        public function __toString(): string
        {
            global $Ice__t_FacetNotExistException;
            return IcePHP_stringifyException($this, $Ice__t_FacetNotExistException);
        }
    }

    $Ice__t_FacetNotExistException = IcePHP_defineException('::Ice::FacetNotExistException', '\\Ice\\FacetNotExistException', $Ice__t_RequestFailedException, null);
}

namespace Ice
{
    global $Ice__t_OperationNotExistException;
    class OperationNotExistException extends \Ice\RequestFailedException
    {
        public function __construct($id=null, $facet='', $operation='')
        {
            parent::__construct($id, $facet, $operation);
        }

        public function ice_id()
        {
            return '::Ice::OperationNotExistException';
        }

        public function __toString(): string
        {
            global $Ice__t_OperationNotExistException;
            return IcePHP_stringifyException($this, $Ice__t_OperationNotExistException);
        }
    }

    $Ice__t_OperationNotExistException = IcePHP_defineException('::Ice::OperationNotExistException', '\\Ice\\OperationNotExistException', $Ice__t_RequestFailedException, null);
}

namespace Ice
{
    global $Ice__t_SyscallException;
    class SyscallException extends \Ice\LocalException
    {
        public function __construct($error=0)
        {
            $this->error = $error;
        }

        public function ice_id()
        {
            return '::Ice::SyscallException';
        }

        public function __toString(): string
        {
            global $Ice__t_SyscallException;
            return IcePHP_stringifyException($this, $Ice__t_SyscallException);
        }

        public $error;
    }
    global $IcePHP__t_int;

    $Ice__t_SyscallException = IcePHP_defineException('::Ice::SyscallException', '\\Ice\\SyscallException', null, array(
        array('error', $IcePHP__t_int, false, 0)));
}

namespace Ice
{
    global $Ice__t_SocketException;
    class SocketException extends \Ice\SyscallException
    {
        public function __construct($error=0)
        {
            parent::__construct($error);
        }

        public function ice_id()
        {
            return '::Ice::SocketException';
        }

        public function __toString(): string
        {
            global $Ice__t_SocketException;
            return IcePHP_stringifyException($this, $Ice__t_SocketException);
        }
    }

    $Ice__t_SocketException = IcePHP_defineException('::Ice::SocketException', '\\Ice\\SocketException', $Ice__t_SyscallException, null);
}

namespace Ice
{
    global $Ice__t_CFNetworkException;
    class CFNetworkException extends \Ice\SocketException
    {
        public function __construct($error=0, $domain='')
        {
            parent::__construct($error);
            $this->domain = $domain;
        }

        public function ice_id()
        {
            return '::Ice::CFNetworkException';
        }

        public function __toString(): string
        {
            global $Ice__t_CFNetworkException;
            return IcePHP_stringifyException($this, $Ice__t_CFNetworkException);
        }

        public $domain;
    }
    global $IcePHP__t_string;

    $Ice__t_CFNetworkException = IcePHP_defineException('::Ice::CFNetworkException', '\\Ice\\CFNetworkException', $Ice__t_SocketException, array(
        array('domain', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_FileException;
    class FileException extends \Ice\SyscallException
    {
        public function __construct($error=0, $path='')
        {
            parent::__construct($error);
            $this->path = $path;
        }

        public function ice_id()
        {
            return '::Ice::FileException';
        }

        public function __toString(): string
        {
            global $Ice__t_FileException;
            return IcePHP_stringifyException($this, $Ice__t_FileException);
        }

        public $path;
    }
    global $IcePHP__t_string;

    $Ice__t_FileException = IcePHP_defineException('::Ice::FileException', '\\Ice\\FileException', $Ice__t_SyscallException, array(
        array('path', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_ConnectFailedException;
    class ConnectFailedException extends \Ice\SocketException
    {
        public function __construct($error=0)
        {
            parent::__construct($error);
        }

        public function ice_id()
        {
            return '::Ice::ConnectFailedException';
        }

        public function __toString(): string
        {
            global $Ice__t_ConnectFailedException;
            return IcePHP_stringifyException($this, $Ice__t_ConnectFailedException);
        }
    }

    $Ice__t_ConnectFailedException = IcePHP_defineException('::Ice::ConnectFailedException', '\\Ice\\ConnectFailedException', $Ice__t_SocketException, null);
}

namespace Ice
{
    global $Ice__t_ConnectionRefusedException;
    class ConnectionRefusedException extends \Ice\ConnectFailedException
    {
        public function __construct($error=0)
        {
            parent::__construct($error);
        }

        public function ice_id()
        {
            return '::Ice::ConnectionRefusedException';
        }

        public function __toString(): string
        {
            global $Ice__t_ConnectionRefusedException;
            return IcePHP_stringifyException($this, $Ice__t_ConnectionRefusedException);
        }
    }

    $Ice__t_ConnectionRefusedException = IcePHP_defineException('::Ice::ConnectionRefusedException', '\\Ice\\ConnectionRefusedException', $Ice__t_ConnectFailedException, null);
}

namespace Ice
{
    global $Ice__t_ConnectionLostException;
    class ConnectionLostException extends \Ice\SocketException
    {
        public function __construct($error=0)
        {
            parent::__construct($error);
        }

        public function ice_id()
        {
            return '::Ice::ConnectionLostException';
        }

        public function __toString(): string
        {
            global $Ice__t_ConnectionLostException;
            return IcePHP_stringifyException($this, $Ice__t_ConnectionLostException);
        }
    }

    $Ice__t_ConnectionLostException = IcePHP_defineException('::Ice::ConnectionLostException', '\\Ice\\ConnectionLostException', $Ice__t_SocketException, null);
}

namespace Ice
{
    global $Ice__t_DNSException;
    class DNSException extends \Ice\LocalException
    {
        public function __construct($error=0, $host='')
        {
            $this->error = $error;
            $this->host = $host;
        }

        public function ice_id()
        {
            return '::Ice::DNSException';
        }

        public function __toString(): string
        {
            global $Ice__t_DNSException;
            return IcePHP_stringifyException($this, $Ice__t_DNSException);
        }

        public $error;
        public $host;
    }
    global $IcePHP__t_int;
    global $IcePHP__t_string;

    $Ice__t_DNSException = IcePHP_defineException('::Ice::DNSException', '\\Ice\\DNSException', null, array(
        array('error', $IcePHP__t_int, false, 0),
        array('host', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_ConnectionIdleException;
    class ConnectionIdleException extends \Ice\LocalException
    {
        public function __construct()
        {
            parent::__construct();
        }

        public function ice_id()
        {
            return '::Ice::ConnectionIdleException';
        }

        public function __toString(): string
        {
            global $Ice__t_ConnectionIdleException;
            return IcePHP_stringifyException($this, $Ice__t_ConnectionIdleException);
        }
    }

    $Ice__t_ConnectionIdleException = IcePHP_defineException('::Ice::ConnectionIdleException', '\\Ice\\ConnectionIdleException', null, null);
}

namespace Ice
{
    global $Ice__t_TimeoutException;
    class TimeoutException extends \Ice\LocalException
    {
        public function __construct()
        {
        }

        public function ice_id()
        {
            return '::Ice::TimeoutException';
        }

        public function __toString(): string
        {
            global $Ice__t_TimeoutException;
            return IcePHP_stringifyException($this, $Ice__t_TimeoutException);
        }
    }

    $Ice__t_TimeoutException = IcePHP_defineException('::Ice::TimeoutException', '\\Ice\\TimeoutException', null, null);
}

namespace Ice
{
    global $Ice__t_ConnectTimeoutException;
    class ConnectTimeoutException extends \Ice\TimeoutException
    {
        public function __construct()
        {
            parent::__construct();
        }

        public function ice_id()
        {
            return '::Ice::ConnectTimeoutException';
        }

        public function __toString(): string
        {
            global $Ice__t_ConnectTimeoutException;
            return IcePHP_stringifyException($this, $Ice__t_ConnectTimeoutException);
        }
    }

    $Ice__t_ConnectTimeoutException = IcePHP_defineException('::Ice::ConnectTimeoutException', '\\Ice\\ConnectTimeoutException', $Ice__t_TimeoutException, null);
}

namespace Ice
{
    global $Ice__t_CloseTimeoutException;
    class CloseTimeoutException extends \Ice\TimeoutException
    {
        public function __construct()
        {
            parent::__construct();
        }

        public function ice_id()
        {
            return '::Ice::CloseTimeoutException';
        }

        public function __toString(): string
        {
            global $Ice__t_CloseTimeoutException;
            return IcePHP_stringifyException($this, $Ice__t_CloseTimeoutException);
        }
    }

    $Ice__t_CloseTimeoutException = IcePHP_defineException('::Ice::CloseTimeoutException', '\\Ice\\CloseTimeoutException', $Ice__t_TimeoutException, null);
}

namespace Ice
{
    global $Ice__t_InvocationTimeoutException;
    class InvocationTimeoutException extends \Ice\TimeoutException
    {
        public function __construct()
        {
            parent::__construct();
        }

        public function ice_id()
        {
            return '::Ice::InvocationTimeoutException';
        }

        public function __toString(): string
        {
            global $Ice__t_InvocationTimeoutException;
            return IcePHP_stringifyException($this, $Ice__t_InvocationTimeoutException);
        }
    }

    $Ice__t_InvocationTimeoutException = IcePHP_defineException('::Ice::InvocationTimeoutException', '\\Ice\\InvocationTimeoutException', $Ice__t_TimeoutException, null);
}

namespace Ice
{
    global $Ice__t_InvocationCanceledException;
    class InvocationCanceledException extends \Ice\LocalException
    {
        public function __construct()
        {
        }

        public function ice_id()
        {
            return '::Ice::InvocationCanceledException';
        }

        public function __toString(): string
        {
            global $Ice__t_InvocationCanceledException;
            return IcePHP_stringifyException($this, $Ice__t_InvocationCanceledException);
        }
    }

    $Ice__t_InvocationCanceledException = IcePHP_defineException('::Ice::InvocationCanceledException', '\\Ice\\InvocationCanceledException', null, null);
}

namespace Ice
{
    global $Ice__t_ProtocolException;
    class ProtocolException extends \Ice\LocalException
    {
        public function __construct($reason='')
        {
            $this->reason = $reason;
        }

        public function ice_id()
        {
            return '::Ice::ProtocolException';
        }

        public function __toString(): string
        {
            global $Ice__t_ProtocolException;
            return IcePHP_stringifyException($this, $Ice__t_ProtocolException);
        }

        public $reason;
    }
    global $IcePHP__t_string;

    $Ice__t_ProtocolException = IcePHP_defineException('::Ice::ProtocolException', '\\Ice\\ProtocolException', null, array(
        array('reason', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_CloseConnectionException;
    class CloseConnectionException extends \Ice\ProtocolException
    {
        public function __construct($reason='')
        {
            parent::__construct($reason);
        }

        public function ice_id()
        {
            return '::Ice::CloseConnectionException';
        }

        public function __toString(): string
        {
            global $Ice__t_CloseConnectionException;
            return IcePHP_stringifyException($this, $Ice__t_CloseConnectionException);
        }
    }

    $Ice__t_CloseConnectionException = IcePHP_defineException('::Ice::CloseConnectionException', '\\Ice\\CloseConnectionException', $Ice__t_ProtocolException, null);
}

namespace Ice
{
    global $Ice__t_ConnectionManuallyClosedException;
    class ConnectionManuallyClosedException extends \Ice\LocalException
    {
        public function __construct($graceful=false)
        {
            $this->graceful = $graceful;
        }

        public function ice_id()
        {
            return '::Ice::ConnectionManuallyClosedException';
        }

        public function __toString(): string
        {
            global $Ice__t_ConnectionManuallyClosedException;
            return IcePHP_stringifyException($this, $Ice__t_ConnectionManuallyClosedException);
        }

        public $graceful;
    }
    global $IcePHP__t_bool;

    $Ice__t_ConnectionManuallyClosedException = IcePHP_defineException('::Ice::ConnectionManuallyClosedException', '\\Ice\\ConnectionManuallyClosedException', null, array(
        array('graceful', $IcePHP__t_bool, false, 0)));
}

namespace Ice
{
    global $Ice__t_DatagramLimitException;
    class DatagramLimitException extends \Ice\ProtocolException
    {
        public function __construct($reason='')
        {
            parent::__construct($reason);
        }

        public function ice_id()
        {
            return '::Ice::DatagramLimitException';
        }

        public function __toString(): string
        {
            global $Ice__t_DatagramLimitException;
            return IcePHP_stringifyException($this, $Ice__t_DatagramLimitException);
        }
    }

    $Ice__t_DatagramLimitException = IcePHP_defineException('::Ice::DatagramLimitException', '\\Ice\\DatagramLimitException', $Ice__t_ProtocolException, null);
}

namespace Ice
{
    global $Ice__t_MarshalException;
    class MarshalException extends \Ice\ProtocolException
    {
        public function __construct($reason='')
        {
            parent::__construct($reason);
        }

        public function ice_id()
        {
            return '::Ice::MarshalException';
        }

        public function __toString(): string
        {
            global $Ice__t_MarshalException;
            return IcePHP_stringifyException($this, $Ice__t_MarshalException);
        }
    }

    $Ice__t_MarshalException = IcePHP_defineException('::Ice::MarshalException', '\\Ice\\MarshalException', $Ice__t_ProtocolException, null);
}

namespace Ice
{
    global $Ice__t_SecurityException;
    class SecurityException extends \Ice\LocalException
    {
        public function __construct($reason='')
        {
            $this->reason = $reason;
        }

        public function ice_id()
        {
            return '::Ice::SecurityException';
        }

        public function __toString(): string
        {
            global $Ice__t_SecurityException;
            return IcePHP_stringifyException($this, $Ice__t_SecurityException);
        }

        public $reason;
    }
    global $IcePHP__t_string;

    $Ice__t_SecurityException = IcePHP_defineException('::Ice::SecurityException', '\\Ice\\SecurityException', null, array(
        array('reason', $IcePHP__t_string, false, 0)));
}

namespace Ice
{
    global $Ice__t_FixedProxyException;
    class FixedProxyException extends \Ice\LocalException
    {
        public function __construct()
        {
        }

        public function ice_id()
        {
            return '::Ice::FixedProxyException';
        }

        public function __toString(): string
        {
            global $Ice__t_FixedProxyException;
            return IcePHP_stringifyException($this, $Ice__t_FixedProxyException);
        }
    }

    $Ice__t_FixedProxyException = IcePHP_defineException('::Ice::FixedProxyException', '\\Ice\\FixedProxyException', null, null);
}

?>

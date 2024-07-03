<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace
{
    require_once 'Ice/Identity.php';
}

namespace Ice
{
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
            return $this->ice_id();
        }

        public $reason;
    }

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
            return $this->ice_id();
        }

        public $reason;
    }

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
            return $this->ice_id();
        }

        public $kindOfObject;
        public $id;
    }    class NotRegisteredException extends \Ice\LocalException
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
            return $this->ice_id();
        }

        public $kindOfObject;
        public $id;
    }    class TwowayOnlyException extends \Ice\LocalException
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
            return $this->ice_id();
        }

        public $operation;
    }

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
            return $this->ice_id();
        }

        public $unknown;
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }

        public $name;
    }

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
            return $this->ice_id();
        }

        public $id;
    }

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
            return $this->ice_id();
        }

        public $proxy;
    }

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
            return $this->ice_id();
        }

        public $str;
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }

        public $reason;
    }

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
            return $this->ice_id();
        }

        public $id;
        public $facet;
        public $operation;
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }

        public $error;
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }

        public $domain;
    }

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
            return $this->ice_id();
        }

        public $path;
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }

        public $error;
        public $host;
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }

        public $reason;
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }

        public $graceful;
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }
    }

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
            return $this->ice_id();
        }

        public $reason;
    }

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
            return $this->ice_id();
        }
    }
}

?>

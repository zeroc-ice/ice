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
    class RequestFailedException extends LocalException
    {
        public Identity $id;
        public string $facet;
        public string $operation;
    }

    final class ObjectNotExistException extends RequestFailedException
    {
    }

    final class FacetNotExistException extends RequestFailedException
    {
    }

    final class OperationNotExistException extends RequestFailedException
    {
    }

    class InitializationException extends LocalException
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

    class PluginInitializationException extends LocalException
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

    class AlreadyRegisteredException extends LocalException
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
    }

    class NotRegisteredException extends LocalException
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
    }

    class TwowayOnlyException extends LocalException
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

    class UnknownException extends LocalException
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

    class UnknownLocalException extends UnknownException
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

    class UnknownUserException extends UnknownException
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

    class CommunicatorDestroyedException extends LocalException
    {
        public function ice_id()
        {
            return '::Ice::CommunicatorDestroyedException';
        }

        public function __toString(): string
        {
            return $this->ice_id();
        }
    }

    class ObjectAdapterDeactivatedException extends LocalException
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

    class ObjectAdapterIdInUseException extends LocalException
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

    class NoEndpointException extends LocalException
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

    class ParseException extends LocalException
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

    class IllegalIdentityException extends LocalException
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

    class IllegalServantException extends LocalException
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

    class SyscallException extends LocalException
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

    class SocketException extends SyscallException
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

    class FileException extends SyscallException
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

    class ConnectFailedException extends SocketException
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

    class ConnectionRefusedException extends ConnectFailedException
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

    class ConnectionLostException extends SocketException
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

    class DNSException extends LocalException
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

    class ConnectionIdleException extends LocalException
    {
        public function ice_id()
        {
            return '::Ice::ConnectionIdleException';
        }

        public function __toString(): string
        {
            return $this->ice_id();
        }
    }

    class TimeoutException extends LocalException
    {
        public function ice_id()
        {
            return '::Ice::TimeoutException';
        }

        public function __toString(): string
        {
            return $this->ice_id();
        }
    }

    class ConnectTimeoutException extends TimeoutException
    {
        public function ice_id()
        {
            return '::Ice::ConnectTimeoutException';
        }

        public function __toString(): string
        {
            return $this->ice_id();
        }
    }

    class CloseTimeoutException extends TimeoutException
    {
        public function ice_id()
        {
            return '::Ice::CloseTimeoutException';
        }

        public function __toString(): string
        {
            return $this->ice_id();
        }
    }

    class InvocationTimeoutException extends TimeoutException
    {
        public function ice_id()
        {
            return '::Ice::InvocationTimeoutException';
        }

        public function __toString(): string
        {
            return $this->ice_id();
        }
    }

    class InvocationCanceledException extends LocalException
    {
        public function ice_id()
        {
            return '::Ice::InvocationCanceledException';
        }

        public function __toString(): string
        {
            return $this->ice_id();
        }
    }

    class ProtocolException extends LocalException
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

    class CloseConnectionException extends ProtocolException
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

    class ConnectionManuallyClosedException extends LocalException
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

    class DatagramLimitException extends ProtocolException
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

    class MarshalException extends ProtocolException
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

    class SecurityException extends LocalException
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

    class FixedProxyException extends LocalException
    {
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

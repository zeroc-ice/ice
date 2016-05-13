<?php
// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace
{
//
// These symbols are defined by the extension but must be declared global
// here to ensure they are visible to scripts regardless of the scope in
// which we are currently executing.
//
global $IcePHP__t_bool;
global $IcePHP__t_byte;
global $IcePHP__t_short;
global $IcePHP__t_int;
global $IcePHP__t_long;
global $IcePHP__t_float;
global $IcePHP__t_double;
global $IcePHP__t_string;
}

namespace Ice
{
    //
    // Exceptions.
    //
    abstract class Exception extends \Exception
    {
        public function __construct($message = '')
        {
            parent::__construct($message);
        }

        abstract public function ice_id();
        public function ice_name()
        {
            trigger_error('ice_name() is deprecated use ice_id() instead.', E_DEPRECATED);
            return substr($this->ice_id(), 2);
        }
    }

    abstract class UserException extends Exception
    {
        public function __construct($message = '')
        {
            parent::__construct($message);
        }
    }

    abstract class LocalException extends Exception
    {
        public function __construct($message = '')
        {
            parent::__construct($message);
        }
    }

    interface Object
    {
        public function ice_isA($id);
        public function ice_ping();
        public function ice_ids();
        public function ice_id();

        //
        // No need to define these here; the marshaling code will invoke them if defined by a subclass.
        //
        //public function ice_preMarshal();
        //public function ice_postUnmarshal();
    }

    abstract class ObjectImpl implements Object
    {
        public function ice_isA($id)
        {
            return array_search($id, ice_ids());
        }

        public function ice_ping()
        {
        }

        public function ice_ids()
        {
            return array(ice_id());
        }

        public function ice_id()
        {
            return "::Ice::Object";
        }
    }

    class ObjectPrxHelper
    {
        public static function checkedCast($proxy, $facetOrCtx=null, $ctx=null)
        {
            return $proxy->ice_checkedCast('::Ice::Object', $facetOrCtx, $ctx);
        }

        public static function uncheckedCast($proxy, $facet=null)
        {
            return $proxy->ice_uncheckedCast('::Ice::Object', $facet);
        }

        public static function ice_staticId()
        {
            return '::Ice::Object';
        }
    }

    $Ice__t_Object = IcePHP_defineClass('::Ice::Object', "\\Ice\\Object", -1, true, false, null, null, null);
    $Ice__t_ObjectSeq = IcePHP_defineSequence('::Ice::ObjectSeq', $Ice__t_Object);
    $Ice__t_LocalObject = IcePHP_defineClass('::Ice::LocalObject', "\\Ice\\LocalObject", -1, true, false, null, null, null);
    $Ice__t_ObjectPrx = IcePHP_defineProxy($Ice__t_Object);
    $Ice__t_ObjectProxySeq = IcePHP_defineSequence('::Ice::ObjectProxySeq', $Ice__t_ObjectPrx);

    class UnknownSlicedObject extends ObjectImpl
    {
        public function __construct()
        {
        }

        public $unknownTypeId;
    }

    $Ice__t_UnknownSlicedObject = IcePHP_defineClass('::Ice::UnknownSlicedObject', "\\Ice\\UnknownSlicedObject", -1, false, true, $Ice__t_Object, null, null);

    interface ObjectFactory
    {
        public function create($id);
        public function destroy();
    }

    interface VaueFactory
    {
        public function create($id);
    }

    class InitializationData
    {
        public function __construct($properties=null, $logger=null)
        {
            $this->properties = $properties;
            $this->logger = $logger;
        }

        public $properties;
        public $logger;
    }

    class SlicedData
    {
        public $slices;
    }

    class SliceInfo
    {
        public $typeId;
        public $bytes;
        public $instances;
    }

    class FormatType
    {
        const DefaultFormat = 0;
        const CompactFormat = 1;
        const SlicedFormat = 2;
    }

    $Ice_sliceChecksums = array();
}

namespace
{
//
// Include certain generated files.
//
require_once 'Ice/BuiltinSequences.php';
require_once 'Ice/Connection.php';
require_once 'Ice/EndpointTypes.php';
require_once 'Ice/LocalException.php';
require_once 'Ice/Locator.php';
require_once 'Ice/ObjectFactory.php';
require_once 'Ice/ValueFactory.php';
require_once 'Ice/Process.php';
require_once 'Ice/Router.php';
require_once 'Ice/Version.php';
require_once 'Ice/Instrumentation.php';
require_once 'Ice/Metrics.php';
require_once 'Ice/RemoteLogger.php';

$Ice_Protocol_1_0 = new Ice\ProtocolVersion(1, 0);
$Ice_Encoding_1_0 = new Ice\EncodingVersion(1, 0);
$Ice_Encoding_1_1 = new Ice\EncodingVersion(1, 1);

IcePHP_defineOperation($Ice__t_Object, 'ice_isA', 2, 1, 0, array(array($IcePHP__t_string, false, 0)), null, array($IcePHP__t_bool, false, 0), null);
IcePHP_defineOperation($Ice__t_Object, 'ice_ping', 2, 1, 0, null, null, null, null);
IcePHP_defineOperation($Ice__t_Object, 'ice_id', 2, 1, 0, null, null, array($IcePHP__t_string, false, 0), null);
IcePHP_defineOperation($Ice__t_Object, 'ice_ids', 2, 1, 0, null, null, array($Ice__t_StringSeq, false, 0), null);
}

namespace Ice
{
    $Protocol_1_0 = new ProtocolVersion(1, 0);
    $Encoding_1_0 = new EncodingVersion(1, 0);
    $Encoding_1_1 = new EncodingVersion(1, 1);

    //
    // Proxy comparison functions.
    //
    function proxyIdentityCompare($lhs, $rhs)
    {
        if(($lhs != null && !($lhs instanceof ObjectPrx)) || ($rhs != null && !($rhs instanceof ObjectPrx)))
        {
            throw new InvalidArgumentException('argument is not a proxy');
        }
        if($lhs == null && $rhs == null)
        {
            return 0;
        }
        elseif($lhs == null && $rhs != null)
        {
            return -1;
        }
        elseif($lhs != null && $rhs == null)
        {
            return 1;
        }
        else
        {
            $lid = $lhs->ice_getIdentity();
            $rid = $rhs->ice_getIdentity();
            if($lid < $rid)
            {
                return -1;
            }
            elseif($lid > $rid)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }

    function proxyIdentityEqual($lhs, $rhs)
    {
        return proxyIdentityCompare($lhs, $rhs) == 0;
    }

    function proxyIdentityAndFacetCompare($lhs, $rhs)
    {
        $n = proxyIdentityCompare($lhs, $rhs);
        if($n == 0 && $lhs != null && $rhs != null)
        {
            $n = strcmp($lhs->ice_getFacet(), $rhs->ice_getFacet());
        }
        return $n;
    }

    function proxyIdentityAndFacetEqual($lhs, $rhs)
    {
        return proxyIdentityAndFacetCompare($lhs, $rhs) == 0;
    }
}

namespace
{
    function Ice_proxyIdentityCompare($lhs, $rhs)
    {
        return Ice\proxyIdentityCompare($lhs, $rhs);
    }

    function Ice_proxyIdentityEqual($lhs, $rhs)
    {
        return Ice\proxyIdentityEqual($lhs, $rhs);
    }

    function Ice_proxyIdentityAndFacetCompare($lhs, $rhs)
    {
        return Ice\proxyIdentityAndFacetCompare($lhs, $rhs);
    }

    function Ice_proxyIdentityAndFacetEqual($lhs, $rhs)
    {
        return Ice\proxyIdentityAndFacetEqual($lhs, $rhs);
    }
}
?>

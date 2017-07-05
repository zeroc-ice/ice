<?php
// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// Definitions for IcePHP with namespaces enabled
//

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

//
// Exceptions.
//
abstract class Ice_Exception extends Exception
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

abstract class Ice_UserException extends Ice_Exception
{
    public function __construct($message = '')
    {
        parent::__construct($message);
    }
}

abstract class Ice_LocalException extends Ice_Exception
{
    public function __construct($message = '')
    {
        parent::__construct($message);
    }
}

class Ice_Value
{
    public static function ice_staticId()
    {
        return "::Ice::Object";
    }

    public function ice_id()
    {
        return "::Ice::Object";
    }

    public function ice_getSlicedData()
    {
        if(property_exists($this, '_ice_slicedData'))
        {
            return $this->_ice_slicedData;
        }
        else
        {
            return null;
        }
    }
}

class Ice_InterfaceByValue extends Ice_Value
{
    public function __construct($id)
    {
        $this->id =$id;
    }

    public function ice_id()
    {
        return $this->id;
    }
}

class Ice_ObjectPrxHelper
{
    public static function checkedCast($proxy, $facetOrContext=null, $context=null)
    {
        return $proxy->ice_checkedCast('::Ice::Object', $facetOrContext, $context);
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

$Ice__t_Value = IcePHP_defineClass('::Ice::Object', "Ice_Value", -1, false, false, null, null);
$Ice__t_ObjectSeq = IcePHP_defineSequence('::Ice::ObjectSeq', $Ice__t_Value);
$Ice__t_LocalObject = IcePHP_defineClass('::Ice::LocalObject', "Ice_LocalObject", -1, false, false, null, null);
$Ice__t_ObjectPrx = IcePHP_defineProxy('::Ice::Object', null, null);
$Ice__t_ObjectProxySeq = IcePHP_defineSequence('::Ice::ObjectProxySeq', $Ice__t_ObjectPrx);

class Ice_UnknownSlicedValue extends Ice_Value
{
    public function __construct()
    {
    }

    public $unknownTypeId;
}

$Ice__t_UnknownSlicedValue = IcePHP_defineClass('::Ice::UnknownSlicedValue', 'Ice_UnknownSlicedValue', -1, true, false, $Ice__t_Value, null);

interface Ice_ObjectFactory
{
    public function create($id);
    public function destroy();
}

interface Ice_ValueFactory
{
    public function create($id);
}

class Ice_InitializationData
{
    public function __construct($properties=null, $logger=null)
    {
        $this->properties = $properties;
        $this->logger = $logger;
    }

    public $properties;
    public $logger;
}

class Ice_SlicedData
{
    public $slices;
}

class Ice_SliceInfo
{
    public $typeId;
    public $bytes;
    public $instances;
}

class Ice_FormatType
{
    const DefaultFormat = 0;
    const CompactFormat = 1;
    const SlicedFormat = 2;
}

$Ice_sliceChecksums = array();

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
require_once 'Ice/Communicator.php';

$Ice_Protocol_1_0 = new Ice_ProtocolVersion(1, 0);
$Ice_Encoding_1_0 = new Ice_EncodingVersion(1, 0);
$Ice_Encoding_1_1 = new Ice_EncodingVersion(1, 1);

IcePHP_defineOperation($Ice__t_ObjectPrx, 'ice_isA', 2, 1, 0, array(array($IcePHP__t_string)), null, array($IcePHP__t_bool), null);
IcePHP_defineOperation($Ice__t_ObjectPrx, 'ice_ping', 2, 1, 0, null, null, null, null);
IcePHP_defineOperation($Ice__t_ObjectPrx, 'ice_id', 2, 1, 0, null, null, array($IcePHP__t_string), null);
IcePHP_defineOperation($Ice__t_ObjectPrx, 'ice_ids', 2, 1, 0, null, null, array($Ice__t_StringSeq), null);

//
// Proxy comparison functions.
//
function Ice_proxyIdentityCompare($lhs, $rhs)
{
    if(($lhs != null && !($lhs instanceof Ice_ObjectPrx)) || ($rhs != null && !($rhs instanceof Ice_ObjectPrx)))
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

function Ice_proxyIdentityEqual($lhs, $rhs)
{
    return Ice_proxyIdentityCompare($lhs, $rhs) == 0;
}

function Ice_proxyIdentityAndFacetCompare($lhs, $rhs)
{
    $n = Ice_proxyIdentityCompare($lhs, $rhs);
    if($n == 0 && $lhs != null && $rhs != null)
    {
        $n = strcmp($lhs->ice_getFacet(), $rhs->ice_getFacet());
    }
    return $n;
}

function Ice_proxyIdentityAndFacetEqual($lhs, $rhs)
{
    return Ice_proxyIdentityAndFacetCompare($lhs, $rhs) == 0;
}

?>

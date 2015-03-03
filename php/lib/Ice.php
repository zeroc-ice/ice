<?php
// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

    abstract public function ice_name();
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

interface Ice_Object
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

abstract class Ice_ObjectImpl implements Ice_Object
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

$Ice__t_Object = IcePHP_defineClass('::Ice::Object', "Ice_Object", true, null, null, null);
$Ice__t_ObjectSeq = IcePHP_defineSequence('::Ice::ObjectSeq', $Ice__t_Object);
$Ice__t_LocalObject = IcePHP_defineClass('::Ice::LocalObject', "Ice_LocalObject", true, null, null, null);
$Ice__t_ObjectPrx = IcePHP_defineProxy($Ice__t_Object);
$Ice__t_ObjectProxySeq = IcePHP_defineSequence('::Ice::ObjectProxySeq', $Ice__t_ObjectPrx);

interface Ice_ObjectFactory
{
    public function create($id);
    public function destroy();
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

$Ice_sliceChecksums = array();

//
// Include certain generated files.
//
require_once 'Ice/BuiltinSequences.php';
require_once 'Ice/Endpoint.php';
require_once 'Ice/EndpointTypes.php';
require_once 'Ice/LocalException.php';
require_once 'Ice/Locator.php';
require_once 'Ice/ObjectFactory.php';
require_once 'Ice/Process.php';
require_once 'Ice/Router.php';

IcePHP_defineOperation($Ice__t_Object, 'ice_isA', 2, 1, array($IcePHP__t_string), null, $IcePHP__t_bool, null);
IcePHP_defineOperation($Ice__t_Object, 'ice_ping', 2, 1, null, null, null, null);
IcePHP_defineOperation($Ice__t_Object, 'ice_id', 2, 1, null, null, $IcePHP__t_string, null);
IcePHP_defineOperation($Ice__t_Object, 'ice_ids', 2, 1, null, null, $Ice__t_StringSeq, null);

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

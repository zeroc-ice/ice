<?php
// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

        abstract public function ice_name();
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

    $Ice__t_Object = IcePHP_defineClass('::Ice::Object', "\\Ice\\Object", true, null, null, null);
    $Ice__t_ObjectSeq = IcePHP_defineSequence('::Ice::ObjectSeq', $Ice__t_Object);
    $Ice__t_LocalObject = IcePHP_defineClass('::Ice::LocalObject', "\\Ice\\LocalObject", true, null, null, null);
    $Ice__t_ObjectPrx = IcePHP_defineProxy($Ice__t_Object);
    $Ice__t_ObjectProxySeq = IcePHP_defineSequence('::Ice::ObjectProxySeq', $Ice__t_ObjectPrx);

    interface ObjectFactory
    {
        public function create($id);
        public function destroy();
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

    $Ice_sliceChecksums = array();
}

namespace
{
//
// Include certain generated files.
//
require 'Ice/BuiltinSequences.php';
require 'Ice/Endpoint.php';
require 'Ice/EndpointTypes.php';
require 'Ice/LocalException.php';
require 'Ice/Locator.php';
require 'Ice/ObjectFactory.php';
require 'Ice/Process.php';
require 'Ice/Router.php';

IcePHP_defineOperation($Ice__t_Object, 'ice_isA', 0, 0, array($IcePHP__t_string), array(), $IcePHP__t_bool, null);
IcePHP_defineOperation($Ice__t_Object, 'ice_ping', 0, 0, null, null, null, null);
IcePHP_defineOperation($Ice__t_Object, 'ice_id', 0, 0, null, null, $IcePHP__t_string, null);
IcePHP_defineOperation($Ice__t_Object, 'ice_ids', 0, 0, null, null, $Ice__t_StringSeq, null);
}

namespace Ice
{
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

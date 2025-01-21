<?php
// Copyright (c) ZeroC, Inc.

namespace Ice;

class ObjectPrxHelper
{
    public static function createProxy($communicator, $proxyString)
    {
        return $communicator->stringToProxy($proxyString, '::Ice::Object');
    }

    public static function checkedCast($proxy, ...$args)
    {
        return $proxy->ice_checkedCast('::Ice::Object', ...$args);
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

global $Ice__t_ObjectPrx;
$Ice__t_ObjectPrx = IcePHP_defineProxy('::Ice::Object', null, null);

//
// Proxy comparison functions.
//
function proxyIdentityCompare($lhs, $rhs)
{
    if (($lhs != null && !($lhs instanceof ObjectPrx)) || ($rhs != null && !($rhs instanceof ObjectPrx))) {
        throw new InvalidArgumentException('argument is not a proxy');
    }
    if ($lhs == null && $rhs == null) {
        return 0;
    } elseif ($lhs == null && $rhs != null) {
        return -1;
    } elseif ($lhs != null && $rhs == null) {
        return 1;
    } else {
        $lid = $lhs->ice_getIdentity();
        $rid = $rhs->ice_getIdentity();
        if ($lid < $rid) {
            return -1;
        } elseif ($lid > $rid) {
            return 1;
        } else {
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
    if ($n == 0 && $lhs != null && $rhs != null) {
        $n = strcmp($lhs->ice_getFacet(), $rhs->ice_getFacet());
    }
    return $n;
}

function proxyIdentityAndFacetEqual($lhs, $rhs)
{
    return proxyIdentityAndFacetCompare($lhs, $rhs) == 0;
}

?>

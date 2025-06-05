<?php
// Copyright (c) ZeroC, Inc.

// These symbols are defined by the extension but must be declared global here to ensure they are visible to scripts
// regardless of the scope in which we are currently executing.
global $IcePHP__t_bool;
global $IcePHP__t_byte;
global $IcePHP__t_short;
global $IcePHP__t_int;
global $IcePHP__t_long;
global $IcePHP__t_float;
global $IcePHP__t_double;
global $IcePHP__t_string;

global $Ice_Protocol_1_0;
global $Ice_Encoding_1_0;
global $Ice_Encoding_1_1;

require_once 'Ice/Exception.php';
require_once 'Ice/Proxy.php';
require_once 'Ice/Value.php';

require_once 'Ice/BuiltinSequences.php';
require_once 'Ice/CompressBatch.php';
require_once 'Ice/EndpointSelectionType.php';
require_once 'Ice/EndpointTypes.php';
require_once 'Ice/Identity.php';
require_once 'Ice/InitializationData.php';
require_once 'Ice/LocalExceptions.php';
require_once 'Ice/Locator.php';
require_once 'Ice/LocatorRegistry.php';
require_once 'Ice/Metrics.php';
require_once 'Ice/Process.php';
require_once 'Ice/RemoteLogger.php';
require_once 'Ice/ReplyStatus.php';
require_once 'Ice/Router.php';
require_once 'Ice/ToStringMode.php';
require_once 'Ice/Version.php';

$Ice_Protocol_1_0 = new Ice\ProtocolVersion(1, 0);
$Ice_Encoding_1_0 = new Ice\EncodingVersion(1, 0);
$Ice_Encoding_1_1 = new Ice\EncodingVersion(1, 1);

IcePHP_defineOperation($Ice__t_ObjectPrx, 'ice_isA', 'ice_isA', 2, 0, array(array($IcePHP__t_string)), null,
    array($IcePHP__t_bool), null);
IcePHP_defineOperation($Ice__t_ObjectPrx, 'ice_ping', 'ice_ping', 2, 0, null, null, null, null);
IcePHP_defineOperation($Ice__t_ObjectPrx, 'ice_id', 'ice_id', 2, 0, null, null, array($IcePHP__t_string), null);
IcePHP_defineOperation($Ice__t_ObjectPrx, 'ice_ids', 'ice_ids', 2, 0, null, null, array($Ice__t_StringSeq), null);

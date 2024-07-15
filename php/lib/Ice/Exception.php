<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice;

class Exception extends \Exception {}

class LocalException extends Exception {}

abstract class UserException extends Exception
{
    abstract public function ice_id();
}

?>

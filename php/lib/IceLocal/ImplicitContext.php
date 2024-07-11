<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice;

interface ImplicitContext
{
    public function getContext();
    public function setContext($newContext);
    public function containsKey($key);
    public function get($key);
    public function put($key, $value);
    public function remove($key);
}

?>

<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
namespace Ice;

interface Logger
{
    public function _print($message);
    public function trace($category, $message);
    public function warning($message);
    public function error($message);
    public function getPrefix();
    public function cloneWithPrefix($prefix);
}

?>

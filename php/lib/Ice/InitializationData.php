<?php
// Copyright (c) ZeroC, Inc.

namespace Ice;

class InitializationData
{
    public function __construct($properties=null, $logger=null, $sliceLoader=null)
    {
        $this->properties = $properties;
        $this->logger = $logger;
        $this->sliceLoader = $sliceLoader;
    }

    public $properties;
    public $logger;
    public $sliceLoader;
}

?>

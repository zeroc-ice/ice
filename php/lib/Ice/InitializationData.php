<?php
// Copyright (c) ZeroC, Inc.

namespace Ice;

interface SliceLoader
{
    public function newInstance($typeId);
}

class InitializationData
{
    public function __construct($properties=null, $logger=null, ?SliceLoader $sliceLoader=null)
    {
        $this->properties = $properties;
        $this->logger = $logger;
        $this->sliceLoader = $sliceLoader;
    }

    public $properties;
    public $logger;
    public ?SliceLoader $sliceLoader;
}

?>

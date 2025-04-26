<?php
// Copyright (c) ZeroC, Inc.

namespace Ice;

interface SliceLoader
{
    public function newInstance($typeId);
}

class InitializationData
{
    public function __construct(
        ?\IcePHP_Properties $properties=null,
        ?\IcePHP_Logger $logger=null,
        ?SliceLoader $sliceLoader=null)
    {
        $this->properties = $properties;
        $this->logger = $logger;
        $this->sliceLoader = $sliceLoader;
    }

    public ?\IcePHP_Properties $properties;
    public ?\IcePHP_Logger $logger;
    public ?SliceLoader $sliceLoader;
}

?>

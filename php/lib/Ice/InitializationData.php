<?php
// Copyright (c) ZeroC, Inc.

namespace Ice;

class InitializationData
{
    public function __construct(
        ?\IcePHP_Properties $properties=null,
        ?\IcePHP_Logger $logger=null)
    {
        $this->properties = $properties;
        $this->logger = $logger;
    }

    public ?\IcePHP_Properties $properties;
    public ?\IcePHP_Logger $logger;
}

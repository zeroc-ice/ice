<?php
// Copyright (c) ZeroC, Inc.

require_once('Test.php');

function allTests($helper)
{
    global $Ice_Encoding_1_0;

    $communicator = $helper->communicator();
    $test = Test\TestIntfPrxHelper::createProxy($communicator, sprintf("Test:%s", $helper->getTestEndpoint()));

    echo "base... ";
    flush();
    {
        try
        {
            $test->baseAsBase();
            test(false);
        }
        catch(Test\Base $b)
        {
            test($b->b == "Base.b");
            test(get_class($b) == "Test\\Base");
        }
    }
    echo "ok\n";

    echo "unknown derived... ";
    flush();
    {
        try
        {
            $test->unknownDerivedAsBase();
            test(false);
        }
        catch(Test\Base $b)
        {
            test($b->b == "UnknownDerived.b");
            test(get_class($b) == "Test\\Base");
        }
    }
    echo "ok\n";

    echo "non-slicing of known derived as base... ";
    flush();
    {
        try
        {
            $test->knownDerivedAsBase();
            test(false);
        }
        catch(Test\KnownDerived $k)
        {
            test($k->b == "KnownDerived.b");
            test($k->kd == "KnownDerived.kd");
            test(get_class($k) == "Test\\KnownDerived");
        }
    }
    echo "ok\n";

    echo "non-slicing of known derived as derived... ";
    flush();
    {
        try
        {
            $test->knownDerivedAsKnownDerived();
            test(false);
        }
        catch(Test\KnownDerived $k)
        {
            test($k->b == "KnownDerived.b");
            test($k->kd == "KnownDerived.kd");
            test(get_class($k) == "Test\\KnownDerived");
        }
    }
    echo "ok\n";

    echo "slicing of unknown intermediate as base... ";
    flush();
    {
        try
        {
            $test->unknownIntermediateAsBase();
            test(false);
        }
        catch(Test\Base $b)
        {
            test($b->b == "UnknownIntermediate.b");
            test(get_class($b) == "Test\\Base");
        }
    }
    echo "ok\n";

    echo "slicing of known intermediate as base... ";
    flush();
    {
        try
        {
            $test->knownIntermediateAsBase();
            test(false);
        }
        catch(Test\KnownIntermediate $ki)
        {
            test($ki->b == "KnownIntermediate.b");
            test($ki->ki == "KnownIntermediate.ki");
            test(get_class($ki) == "Test\\KnownIntermediate");
        }
    }
    echo "ok\n";

    echo "slicing of known most derived as base... ";
    flush();
    {
        try
        {
            $test->knownMostDerivedAsBase();
            test(false);
        }
        catch(Test\KnownMostDerived $kmd)
        {
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == "Test\\KnownMostDerived");
        }
    }
    echo "ok\n";

    echo "non-slicing of known intermediate as intermediate... ";
    flush();
    {
        try
        {
            $test->knownIntermediateAsKnownIntermediate();
            test(false);
        }
        catch(Test\KnownIntermediate $ki)
        {
            test($ki->b == "KnownIntermediate.b");
            test($ki->ki == "KnownIntermediate.ki");
            test(get_class($ki) == "Test\\KnownIntermediate");
        }
    }
    echo "ok\n";

    echo "non-slicing of known most derived as intermediate... ";
    flush();
    {
        try
        {
            $test->knownMostDerivedAsKnownIntermediate();
            test(false);
        }
        catch(Test\KnownMostDerived $kmd)
        {
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == "Test\\KnownMostDerived");
        }
    }
    echo "ok\n";

    echo "non-slicing of known most derived as most derived... ";
    flush();
    {
        try
        {
            $test->knownMostDerivedAsKnownMostDerived();
            test(false);
        }
        catch(Test\KnownMostDerived $kmd)
        {
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == "Test\\KnownMostDerived");
        }
    }
    echo "ok\n";

    echo "slicing of unknown most derived, known intermediate as base... ";
    flush();
    {
        try
        {
            $test->unknownMostDerived1AsBase();
            test(false);
        }
        catch(Test\KnownIntermediate $ki)
        {
            test($ki->b == "UnknownMostDerived1.b");
            test($ki->ki == "UnknownMostDerived1.ki");
            test(get_class($ki) == "Test\KnownIntermediate");
        }
    }
    echo "ok\n";

    echo "slicing of unknown most derived, known intermediate as intermediate... ";
    flush();
    {
        try
        {
            $test->unknownMostDerived1AsKnownIntermediate();
            test(false);
        }
        catch(Test\KnownIntermediate $ki)
        {
            test($ki->b == "UnknownMostDerived1.b");
            test($ki->ki == "UnknownMostDerived1.ki");
            test(get_class($ki) == "Test\\KnownIntermediate");
        }
    }
    echo "ok\n";

    echo "slicing of unknown most derived, unknown intermediate as base... ";
    flush();
    {
        try
        {
            $test->unknownMostDerived2AsBase();
            test(false);
        }
        catch(Test\Base $b)
        {
            test($b->b == "UnknownMostDerived2.b");
            test(get_class($b) == "Test\\Base");
        }
    }
    echo "ok\n";

    return $test;
}

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            $communicator = $this->initialize($args);
            $proxy = allTests($this);
            $proxy->shutdown();
            $communicator->destroy();
        }
        catch(Exception $ex)
        {
            $communicator->destroy();
            throw $ex;
       }
    }
}

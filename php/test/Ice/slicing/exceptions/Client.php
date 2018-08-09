<?php
// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

require_once('Test.php');

function allTests($helper)
{
    global $NS;
    global $Ice_Encoding_1_0;

    $communicator = $helper->communicator();
    $obj = $communicator->stringToProxy(sprintf("Test:%s", $helper->getTestEndpoint()));
    $test = $obj->ice_checkedCast("::Test::TestIntf");

    echo "base... ";
    flush();
    {
        try
        {
            $test->baseAsBase();
            test(false);
        }
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\Base" : "Test_Base";
            if(!($b instanceof $excls))
            {
                throw $b;
            }
            test($b->b == "Base.b");
            test(get_class($b) == ($NS ? "Test\\Base" : "Test_Base"));
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
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\Base" : "Test_Base";
            if(!($b instanceof $excls))
            {
                throw $b;
            }
            test($b->b == "UnknownDerived.b");
            test(get_class($b) == ($NS ? "Test\\Base" : "Test_Base"));
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
        catch(Exception $k)
        {
            $excls = $NS ? "Test\\KnownDerived" : "Test_KnownDerived";
            if(!($k instanceof $excls))
            {
                throw $k;
            }
            test($k->b == "KnownDerived.b");
            test($k->kd == "KnownDerived.kd");
            test(get_class($k) == ($NS ? "Test\\KnownDerived" : "Test_KnownDerived"));
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
        catch(Exception $k)
        {
            $excls = $NS ? "Test\\KnownDerived" : "Test_KnownDerived";
            if(!($k instanceof $excls))
            {
                throw $k;
            }
            test($k->b == "KnownDerived.b");
            test($k->kd == "KnownDerived.kd");
            test(get_class($k) == ($NS ? "Test\\KnownDerived" : "Test_KnownDerived"));
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
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\Base" : "Test_Base";
            if(!($b instanceof $excls))
            {
                throw $b;
            }
            test($b->b == "UnknownIntermediate.b");
            test(get_class($b) == ($NS ? "Test\\Base" : "Test_Base"));
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
        catch(Exception $ki)
        {
            $excls = $NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate";
            if(!($ki instanceof $excls))
            {
                throw $ki;
            }
            test($ki->b == "KnownIntermediate.b");
            test($ki->ki == "KnownIntermediate.ki");
            test(get_class($ki) == ($NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate"));
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
        catch(Exception $kmd)
        {
            $excls = $NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived";
            if(!($kmd instanceof $excls))
            {
                throw $kmd;
            }
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == ($NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived"));
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
        catch(Exception $ki)
        {
            $excls = $NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate";
            if(!($ki instanceof $excls))
            {
                throw $ki;
            }
            test($ki->b == "KnownIntermediate.b");
            test($ki->ki == "KnownIntermediate.ki");
            test(get_class($ki) == ($NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate"));
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
        catch(Exception $kmd)
        {
            $excls = $NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived";
            if(!($kmd instanceof $excls))
            {
                throw $kmd;
            }
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == ($NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived"));
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
        catch(Exception $kmd)
        {
            $excls = $NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived";
            if(!($kmd instanceof $excls))
            {
                throw $kmd;
            }
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == ($NS ? "Test\\KnownMostDerived" : "Test_KnownMostDerived"));
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
        catch(Exception $ki)
        {
            $excls = $NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate";
            if(!($ki instanceof $excls))
            {
                throw $ki;
            }
            test($ki->b == "UnknownMostDerived1.b");
            test($ki->ki == "UnknownMostDerived1.ki");
            test(get_class($ki) == ($NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate"));
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
        catch(Exception $ki)
        {
            $excls = $NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate";
            if(!($ki instanceof $excls))
            {
                throw $ki;
            }
            test($ki->b == "UnknownMostDerived1.b");
            test($ki->ki == "UnknownMostDerived1.ki");
            test(get_class($ki) == ($NS ? "Test\\KnownIntermediate" : "Test_KnownIntermediate"));
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
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\Base" : "Test_Base";
            if(!($b instanceof $excls))
            {
                throw $b;
            }
            test($b->b == "UnknownMostDerived2.b");
            test(get_class($b) == ($NS ? "Test\\Base" : "Test_Base"));
        }
    }
    echo "ok\n";

    echo "unknown most derived in compact format... ";
    flush();
    {
        try
        {
            $test->unknownMostDerived2AsBaseCompact();
            test(false);
        }
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\Base" : "Test_Base";
            if(($b instanceof $excls))
            if(get_class($b) == ($NS ? "Test\\Base" : "Test_Base"))
            {
                //
                // For the 1.0 encoding, the unknown exception is sliced to Base.
                //
                test($test->ice_getEncodingVersion() == $Ice_Encoding_1_0);
            }
            else if(get_class($b) == ($NS ? "Ice\\UnkownUserException" : "Ice_UnknownUserException"))
            {
                //
                // An UnkonwnUserException is raised for the compact format because the
                // most-derived type is unknown and the exception cannot be sliced.
                //
                test($test->ice_getEncodingVersion() != $Ice_Encoding_1_0);
            }
            else
            {
                throw $b;
            }
        }
    }
    echo "ok\n";

    echo "preserved exceptions... ";
    flush();
    {
        try
        {
            $test->knownPreservedAsBase();
            test(false);
        }
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\KnownPreservedDerived" : "Test_KnownPreservedDerived";
            if(!($b instanceof $excls))
            {
                throw $b;
            }
            test($b->b == "base");
            test($b->kp == "preserved");
            test($b->kpd == "derived");
        }

        try
        {
            $test->knownPreservedAsKnownPreserved();
            test(false);
        }
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\KnownPreservedDerived" : "Test_KnownPreservedDerived";
            if(!($b instanceof $excls))
            {
                throw $b;
            }
            test($b->b == "base");
            test($b->kp == "preserved");
            test($b->kpd == "derived");
        }

        try
        {
            $test->unknownPreservedAsBase();
            test(false);
        }
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\KnownPreservedDerived" : "Test_KnownPreservedDerived";
            if(!($b instanceof $excls))
            {
                throw $b;
            }
            test($b->b == "base");
            test($b->kp == "preserved");
            test($b->kpd == "derived");
        }

        try
        {
            $test->unknownPreservedAsKnownPreserved();
            test(false);
        }
        catch(Exception $b)
        {
            $excls = $NS ? "Test\\KnownPreservedDerived" : "Test_KnownPreservedDerived";
            if(!($b instanceof $excls))
            {
                throw $b;
            }
            test($b->b == "base");
            test($b->kp == "preserved");
            test($b->kpd == "derived");
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
?>

<?php
// Copyright (c) ZeroC, Inc.

require_once('Test.php');

function allTests($helper)
{
    echo "testing stringToProxy... ";
    flush();
    $ref = sprintf("d:%s", $helper->getTestEndpoint());
    $communicator = $helper->communicator();
    $db = $communicator->stringToProxy($ref);
    test($db != null);
    echo "ok\n";

    echo "testing unchecked cast... ";
    flush();
    $prx = Ice\ObjectPrxHelper::uncheckedCast($db);
    test($prx->ice_getFacet() == "");
    $prx = Ice\ObjectPrxHelper::uncheckedCast($db, "facetABCD");
    test($prx->ice_getFacet() == "facetABCD");
    $prx2 = Ice\ObjectPrxHelper::uncheckedCast($prx);
    test($prx2->ice_getFacet() == "facetABCD");
    $prx3 = Ice\ObjectPrxHelper::uncheckedCast($prx, "");
    test($prx3->ice_getFacet() == "");
    $d = Test\DPrxHelper::uncheckedCast($db);
    test($d->ice_getFacet() == "");
    $df = Test\DPrxHelper::uncheckedCast($db, "facetABCD");
    test($df->ice_getFacet() == "facetABCD");
    $df2 = Test\DPrxHelper::uncheckedCast($df);
    test($df2->ice_getFacet() == "facetABCD");
    $df3 = Test\DPrxHelper::uncheckedCast($df, "");
    test($df3->ice_getFacet() == "");
    echo "ok\n";

    echo "testing checked cast... ";
    flush();
    $prx = Ice\ObjectPrxHelper::checkedCast($db);
    test($prx->ice_getFacet() == "");
    $prx = Ice\ObjectPrxHelper::checkedCast($db, "facetABCD");
    test($prx->ice_getFacet() == "facetABCD");
    $prx2 = Ice\ObjectPrxHelper::checkedCast($prx);
    test($prx2->ice_getFacet() == "facetABCD");
    $prx3 = Ice\ObjectPrxHelper::checkedCast($prx, "");
    test($prx3->ice_getFacet() == "");
    $d = Test\DPrxHelper::checkedCast($db);
    test($d->ice_getFacet() == "");
    $df = Test\DPrxHelper::checkedCast($db, "facetABCD");
    test($df->ice_getFacet() == "facetABCD");
    $df2 = Test\DPrxHelper::checkedCast($df);
    test($df2->ice_getFacet() == "facetABCD");
    $df3 = Test\DPrxHelper::checkedCast($df, "");
    test($df3->ice_getFacet() == "");
    echo "ok\n";

    echo "testing non-facets A, B, C, and D... ";
    flush();
    test($d->callA() == "A");
    test($d->callB() == "B");
    test($d->callC() == "C");
    test($d->callD() == "D");
    echo "ok\n";

    echo "testing facets A, B, C, and D... ";
    flush();
    $df = $d->ice_checkedCast("::Test::D", "facetABCD");
    test($df != null);
    test($df->callA() == "A");
    test($df->callB() == "B");
    test($df->callC() == "C");
    test($df->callD() == "D");
    echo "ok\n";

    echo "testing facets E and F... ";
    flush();
    $ff = $d->ice_checkedCast("::Test::F", "facetEF");
    test($ff != null);
    test($ff->callE() == "E");
    test($ff->callF() == "F");
    echo "ok\n";

    echo "testing facet G... ";
    flush();
    $gf = $ff->ice_checkedCast("::Test::G", "facetGH");
    test($gf != null);
    test($gf->callG() == "G");
    echo "ok\n";

    echo "testing whether casting preserves the facet... ";
    flush();
    $hf = $gf->ice_checkedCast("::Test::H");
    test($hf != null);
    test($hf->callG() == "G");
    test($hf->callH() == "H");
    echo "ok\n";

    return $gf;
}

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            $communicator = $this->initialize($args);
            $proxy= allTests($this);
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

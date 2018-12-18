<?php
// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

require_once('Test.php');

function allTests($helper)
{
    global $NS;

    $communicator = $helper->communicator();
    {
        $base = $communicator->stringToProxy(sprintf("i1:%s", $helper->getTestEndpoint()));
        $i = $base->ice_checkedCast("::Test::I");

        $s1 = $NS ? eval("return new Test\\S(0);") :
                    eval("return new Test_S(0);");
        $s2 = null;
        $s3 = $i->opS($s1, $s2);
        test($s1 == $s3);
        test($s2 == $s3);

        $sseq1 = array($s1);
        $sseq2 = null;
        $sseq3 = $i->opSSeq($sseq1, $sseq2);
        test($sseq2[0] == $s1);
        test($sseq3[0] == $s1);

        $smap1 = array("a" => $s1);
        $smap2 = null;
        $smap3 = $i->opSMap($smap1, $smap2);
        test($smap2["a"] == $s1);
        test($smap3["a"] == $s1);
    }

    {
        $base = $communicator->stringToProxy(sprintf("i2:%s", $helper->getTestEndpoint()));
        $i = $base->ice_checkedCast("::Test::Inner::Inner2::I");

        $s1 = $NS ? eval("return new Test\\Inner\\Inner2\\S(0);") :
                    eval("return new Test_Inner_Inner2_S(0);");
        $s2 = null;
        $s3 = $i->opS($s1, $s2);
        test($s1 == $s3);
        test($s2 == $s3);

        $sseq1 = array($s1);
        $sseq2 = null;
        $sseq3 = $i->opSSeq($sseq1, $sseq2);
        test($sseq2[0] == $s1);
        test($sseq3[0] == $s1);

        $smap1 = array("a" => $s1);
        $smap2 = null;
        $smap3 = $i->opSMap($smap1, $smap2);
        test($smap2["a"] == $s1);
        test($smap3["a"] == $s1);
    }

    {
        $base = $communicator->stringToProxy(sprintf("i3:%s", $helper->getTestEndpoint()));
        $i = $base->ice_checkedCast("::Test::Inner::I");

        $s1 = $NS ? eval("return new Test\\Inner\\Inner2\\S(0);") :
                    eval("return new Test_Inner_Inner2_S(0);");
        $s2 = null;
        $s3 = $i->opS($s1, $s2);
        test($s1 == $s3);
        test($s2 == $s3);

        $sseq1 = array($s1);
        $sseq2 = null;
        $sseq3 = $i->opSSeq($sseq1, $sseq2);
        test($sseq2[0] == $s1);
        test($sseq3[0] == $s1);

        $smap1 = array("a" => $s1);
        $smap2 = null;
        $smap3 = $i->opSMap($smap1, $smap2);
        test($smap2["a"] == $s1);
        test($smap3["a"] == $s1);
    }

    {
        $base = $communicator->stringToProxy(sprintf("i4:%s", $helper->getTestEndpoint()));
        $i = $base->ice_checkedCast("::Inner::Test::Inner2::I");

        $s1 = $NS ? eval("return new Test\\S(0);") :
                    eval("return new Test_S(0);");
        $s2 = null;
        $s3 = $i->opS($s1, $s2);
        test($s1 == $s3);
        test($s2 == $s3);

        $sseq1 = array($s1);
        $sseq2 = null;
        $sseq3 = $i->opSSeq($sseq1, $sseq2);
        test($sseq2[0] == $s1);
        test($sseq3[0] == $s1);

        $smap1 = array("a" => $s1);
        $smap2 = null;
        $smap3 = $i->opSMap($smap1, $smap2);
        test($smap2["a"] == $s1);
        test($smap3["a"] == $s1);
    }

    {
        $base = $communicator->stringToProxy(sprintf("i1:%s", $helper->getTestEndpoint()));
        $i = $base->ice_checkedCast("::Test::I");
        $i->shutdown();
    }
}

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            echo "test same Slice type name in different scopes... ";
            flush();
            $communicator = $this->initialize($args);
            allTests($this);
            echo "ok\n";
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

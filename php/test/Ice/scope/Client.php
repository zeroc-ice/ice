<?php
// Copyright (c) ZeroC, Inc.

require_once('Test.php');

function allTests($helper)
{
    $communicator = $helper->communicator();
    {
        $i = Test\IPrxHelper::createProxy($communicator, sprintf("i1:%s", $helper->getTestEndpoint()));

        $s1 = new Test\MyStruct(0);
        $s2 = null;
        $s3 = $i->opMyStruct($s1, $s2);
        test($s1 == $s3);
        test($s2 == $s3);

        $sseq1 = array($s1);
        $sseq2 = null;
        $sseq3 = $i->opMyStructSeq($sseq1, $sseq2);
        test($sseq2[0] == $s1);
        test($sseq3[0] == $s1);

        $smap1 = array("a" => $s1);
        $smap2 = null;
        $smap3 = $i->opMyStructMap($smap1, $smap2);
        test($smap2["a"] == $s1);
        test($smap3["a"] == $s1);

        $v1 = Test\E1::v1;
        $e = $i->opE1($v1);
        test($e == $v1);

        $s = $i->opS1(new Test\S1("S1"));
        test($s->s == "S1");

        $c = $i->opC1(new Test\C1("C1"));
        test($c->s == "C1");
    }

    {
        $i = Test\Inner\Inner2\IPrxHelper::createProxy($communicator, sprintf("i2:%s", $helper->getTestEndpoint()));

        $s1 = new Test\Inner\Inner2\MyStruct(0);
        $s2 = null;
        $s3 = $i->opMyStruct($s1, $s2);
        test($s1 == $s3);
        test($s2 == $s3);

        $sseq1 = array($s1);
        $sseq2 = null;
        $sseq3 = $i->opMyStructSeq($sseq1, $sseq2);
        test($sseq2[0] == $s1);
        test($sseq3[0] == $s1);

        $smap1 = array("a" => $s1);
        $smap2 = null;
        $smap3 = $i->opMyStructMap($smap1, $smap2);
        test($smap2["a"] == $s1);
        test($smap3["a"] == $s1);
    }

    {
        $i = Test\Inner\IPrxHelper::createProxy($communicator, sprintf("i3:%s", $helper->getTestEndpoint()));

        $s1 = new Test\Inner\Inner2\MyStruct(0);
        $s2 = null;
        $s3 = $i->opMyStruct($s1, $s2);
        test($s1 == $s3);
        test($s2 == $s3);

        $sseq1 = array($s1);
        $sseq2 = null;
        $sseq3 = $i->opMyStructSeq($sseq1, $sseq2);
        test($sseq2[0] == $s1);
        test($sseq3[0] == $s1);

        $smap1 = array("a" => $s1);
        $smap2 = null;
        $smap3 = $i->opMyStructMap($smap1, $smap2);
        test($smap2["a"] == $s1);
        test($smap3["a"] == $s1);
    }

    {
        $i = Inner\Test\Inner2\IPrxHelper::createProxy($communicator, sprintf("i4:%s", $helper->getTestEndpoint()));

        $s1 = new Test\MyStruct(0);
        $s2 = null;
        $s3 = $i->opMyStruct($s1, $s2);
        test($s1 == $s3);
        test($s2 == $s3);

        $sseq1 = array($s1);
        $sseq2 = null;
        $sseq3 = $i->opMyStructSeq($sseq1, $sseq2);
        test($sseq2[0] == $s1);
        test($sseq3[0] == $s1);

        $smap1 = array("a" => $s1);
        $smap2 = null;
        $smap3 = $i->opMyStructMap($smap1, $smap2);
        test($smap2["a"] == $s1);
        test($smap3["a"] == $s1);
    }

    {
        $i = Test\IPrxHelper::createProxy($communicator, sprintf("i1:%s", $helper->getTestEndpoint()));
        $i->shutdown();
    }
}

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            echo "test using same type name in different Slice modules... ";
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

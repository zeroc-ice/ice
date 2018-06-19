<?php
// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

error_reporting(E_ALL | E_STRICT);

if(!extension_loaded("ice"))
{
    echo "\nerror: Ice extension is not loaded.\n\n";
    exit(1);
}

$NS = function_exists("Ice\\initialize");
require_once('Ice.php');
require_once('Test.php');

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        echo "\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n";
        exit(1);
    }
}

function allTests($communicator)
{
    global $NS;

    {
        $base = $communicator->stringToProxy("i1:default -p 12010");
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
        $base = $communicator->stringToProxy("i2:default -p 12010");
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
        $base = $communicator->stringToProxy("i3:default -p 12010");
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
        $base = $communicator->stringToProxy("i1:default -p 12010");
        $i = $base->ice_checkedCast("::Test::I");
        $i->shutdown();
    }
}

echo "test same Slice type name in different scopes... ";
flush();

$communicator = $NS ? eval("return Ice\\initialize(\$argv);") :
                      eval("return Ice_initialize(\$argv);");

allTests($communicator);

echo "ok\n";

?>

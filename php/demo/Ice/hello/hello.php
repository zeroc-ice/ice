<HTML>
<HEAD>
    <TITLE>Hello Demo</TITLE>
</HEAD>
<BODY>

    <H1>Hello Demo</H1>

<?php
// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

Ice_loadProfile();

//
// Change this to true if SSL is configured for the PHP extension.
//
$have_ssl = false;

if(isset($_POST["submitted"]))
{
    echo "<HR>\n";
    echo "<P>Status:<BR><B>\n";
    try
    {
        if($have_ssl)
        {
            $p = $ICE->stringToProxy("hello:tcp -p 10000:udp -p 10000:ssl -p 10001");
        }
        else
        {
            $p = $ICE->stringToProxy("hello:tcp -p 10000:udp -p 10000");
        }

        if(isset($_POST["mode"]))
        {
            if($_POST["mode"] == "oneway")
            {
                $p = $p->ice_oneway();
            }
            elseif($_POST["mode"] == "datagram")
            {
                $p = $p->ice_datagram();
            }
        }

        if(isset($_POST["secure"]) and $_POST["secure"] == "yes")
        {
            $p = $p->ice_secure(true);
        }
        if(isset($_POST["timeout"]) and $_POST["timeout"] == "yes")
        {
            $p = $p->ice_timeout(2000);
        }

        if($p->ice_isTwoway())
        {
            $hello = $p->ice_checkedCast("::Demo::Hello");
        }
        else
        {
            $hello = $p->ice_uncheckedCast("::Demo::Hello");
        }

        if(isset($_POST["sayHello"]))
        {
            $hello->sayHello(0);
        }
        elseif(isset($_POST["shutdown"]))
        {
            $hello->shutdown();
        }

        echo "OK\n";
    }
    catch(Ice_LocalException $ex)
    {
        echo "<pre>\n";
        print_r($ex);
        echo "</pre>\n";
    }
    echo "</B></P>\n";
    echo "<HR>\n";
}
?>

    <P>
    <FORM method="POST" action="<?php echo basename($_SERVER["PHP_SELF"]); ?>">
        <P>Mode: 
        <INPUT type="radio" name="mode" value="twoway"
            <?php if(!isset($_POST["mode"]) or $_POST["mode"] == "twoway") echo " checked "; ?>
        > Twoway
        <INPUT type="radio" name="mode" value="oneway"
            <?php if(isset($_POST["mode"]) and $_POST["mode"] == "oneway") echo " checked "; ?>
        > Oneway
        <INPUT type="radio" name="mode" value="datagram"
            <?php if(isset($_POST["mode"]) and $_POST["mode"] == "datagram") echo " checked "; ?>
        > Datagram
        </P>
        <P>Options: 
        <INPUT type="checkbox" name="secure" value="yes"
            <?php if(isset($_POST["secure"]) and $_POST["secure"] == "yes") echo " checked "; ?>
            <?php if(!$have_ssl) echo " disabled "; ?>
        > Secure
        <INPUT type="checkbox" name="timeout" value="yes"
            <?php if(isset($_POST["timeout"]) and $_POST["timeout"] == "yes") echo " checked "; ?>
        > Timeout
        </P>
        <P>
        <INPUT type="hidden" name="submitted" value="yes">
        <INPUT type="submit" name="sayHello" value="Say Hello">
        <INPUT type="submit" name="shutdown" value="Shutdown">
    </FORM>
</BODY>
</HTML>

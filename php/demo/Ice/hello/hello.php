<HTML>
<HEAD>
    <TITLE>Hello Demo</TITLE>
</HEAD>
<BODY>

    <H1>Hello Demo</H1>

<?php
Ice_loadProfile();

if(isset($_POST["submitted"]))
{
    echo "<HR>\n";
    echo "<P>Status:<BR><B>\n";
    try
    {
        $p = $ICE->stringToProxy("hello:tcp -p 10000:udp -p 10000:ssl -p 10001");

        if($_POST["mode"] == "oneway")
        {
            $p = $p->ice_oneway();
        }
        elseif($_POST["mode"] == "datagram")
        {
            $p = $p->ice_datagram();
        }

        if($_POST["secure"] == "yes")
        {
            $p = $p->ice_secure(true);
        }
        if($_POST["timeout"] == "yes")
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
            $hello->sayHello();
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
            <?php if(!isset($_POST["mode"]) or $_POST["mode"] == "twoway") echo "checked"; ?>
        > Twoway
        <INPUT type="radio" name="mode" value="oneway"
            <?php if($_POST["mode"] == "oneway") echo "checked"; ?>
        > Oneway
        <INPUT type="radio" name="mode" value="datagram"
            <?php if($_POST["mode"] == "datagram") echo "checked"; ?>
        > Datagram
        </P>
        <P>Options: 
        <INPUT type="checkbox" name="secure" value="yes"
            <?php if($_POST["secure"] == "yes") echo "checked"; ?>
        > Secure
        <INPUT type="checkbox" name="timeout" value="yes"
            <?php if($_POST["timeout"] == "yes") echo "checked"; ?>
        > Timeout
        </P>
        <P>
        <INPUT type="hidden" name="submitted" value="yes">
        <INPUT type="submit" name="sayHello" value="Say Hello">
        <INPUT type="submit" name="shutdown" value="Shutdown">
    </FORM>
</BODY>
</HTML>

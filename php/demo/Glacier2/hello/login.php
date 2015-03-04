<HTML>
<HEAD>
<TITLE>Session Demo - Login</TITLE>
</HEAD>

<BODY>
<H1>Session Demo - Login</H1>

<?php
require_once 'Ice.php';
require_once 'Glacier2.php';

session_start();

try
{
    //
    // Check for an existing session.
    //
    $ICE = Ice_find(session_id());
    if(isset($_SESSION['authenticated']) || $ICE != null)
    {
        unset($_SESSION['authenticated']);
        Ice_unregister(session_id());
        echo "<P><HR><I>Destroyed previous session.</I><HR></P>\n";
    }

    //
    // Generate a new session ID.
    //
    session_regenerate_id();

    if(isset($_POST["submitted"]))
    {
        //
        // Initialize a communicator using the default properties.
        //
        $initData = new Ice_InitializationData;
        $initData->properties = Ice_createProperties();
        $initData->properties->setProperty("Ice.Default.Router", "DemoGlacier2/router:tcp -p 4063 -h localhost");
        $ICE = Ice_initialize($initData);

        try
        {
            //
            // Verify that we are using a Glacier2 router.
            //
            $router = Glacier2_RouterPrxHelper::checkedCast($ICE->getDefaultRouter());
            if($router == null)
            {
                echo "<P><HR><B>Configured router is not a Glacier2 router.</B><HR></P>\n";
                die();
            }

            $user = "";
            $password = "";

            if(isset($_POST["user"]))
            {
                $user = $_POST["user"];
            }
            if(isset($_POST["password"]))
            {
                $password = $_POST["password"];
            }

            if(isset($_POST["login"]))
            {
                $router->createSession($user, $password);
                $_SESSION['authenticated'] = 'true';
                Ice_register($ICE, session_id(), session_cache_expire());
                header("Location: session.php"); // Redirect.
                exit();
            }
        }
        catch(Glacier2_PermissionDeniedException $ex)
        {
            echo "<P><HR><B>Login failure: " . $ex->reason . "</B><HR></P>\n";
        }
    }
}
catch(Ice_LocalException $ex)
{
    echo "<P><HR><B>\n";
    echo "<PRE>\n";
    print_r($ex);
    echo "</PRE>\n";
    echo "</B><HR></P>\n";
}
?>

    <P>
    <FORM method="POST" action="<?php echo basename($_SERVER["PHP_SELF"]); ?>">
        <P>This demo accepts any username / password combination.</P>
        <P>
        Username: <INPUT type="text" size="12" name="user"><BR>
        <BR>
        Password: <INPUT type="text" size="12" name="password"><BR>
        </P>
        <P>
        <INPUT type="hidden" name="submitted" value="yes">
        <INPUT type="submit" name="login" value="Login">
    </FORM>
</BODY>
</HTML>

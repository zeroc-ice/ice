This test demonstrates the sharing of a single Ice object adapter for
multiple EJBs.

It setups 2 EJBs which both implement a similar service but with
slightly different Slice types: the Account class in the first version
has 1 string member and the second version has 2 string members:

// Version 1
class Account extends Base
{
    string id;
};

// Version 2
class Account extends Base
{
    string id;
    string foo;
};

The Base class is defined in a Jar shared by both EJBs and install in
the lib directory of the application server.

Each EJB registers servants using the IceAdapter helper class. This
helper manages the shared Ice object adapter.

The helper registers the servant with the object adapter and uses a
dispatch interceptor to setup the thread context class loader to
ensure the servant invocations are un-marhsalled with the EJB class
loader.

With Ice 3.5, this interceptor will no longer be necessary, the Ice
core will provide a property to enable the setting of the thread
context class loader from the Ice core directly.

Building the test
-----------------

Set the JBOSS_HOME environment variable to your jboss-eap-5.1/jboss-as
directory, for example:

  $ export JBOSS_HOME=$HOME/EnterprisePlatform-5.1.2/jboss-eap-5.1/jboss-as/

If Ice isn't installed in the default location set the ICE_HOME
environment variable to your Ice 3.7.1 installation directory.

Run ant to build and deploy the application:

  $ ant deploy

This will deploy the following Jars to your JBoss server default
configuration:

  $JBOSS_HOME/server/default/lib/Ice.jar
  $JBOSS_HOME/server/default/lib/ice-ejb-test-common.jar
  $JBOSS_HOME/server/default/deploy/ice-ejb-test-ejb1.jar
  $JBOSS_HOME/server/default/deploy/ice-ejb-test-ejb2.jar

Run the clients
---------------

To run the client for the first EJB version:

  $ ant run.client1

For the second EJB version:

  $ ant run.client2

Note that the clients uses the Java naming provier specified in
src/resources/jndi.properties (jnp://127.0.0.1:1099) to obtain a
reference on the initialization EJB of each service. The Ice object
adapter listen on the port 10000 of the loopback interface.

Oracle OCCI demo
================

This demo shows how to implement an Ice server that uses Oracle
through its Oracle C++ Call Interface (OCCI) API.

It is a fairly simple demo that illustrates how to:

 - Map relational data to Ice objects, in particular convert between
   Ice and OCCI types.
 - Use an OCCI pool to provide Oracle connections to Ice requests.
 - Use an Ice servant locator.


Building the demo
-----------------

OCCI is only available for some C++ compilers. Make sure to select a
platform and C++ compiler supported by both Ice and OCCI.

OCCI Oracle drivers are available at:

 http://www.oracle.com/technetwork/database/features/instant-client/index-097480.html

- Setup an Oracle database with the traditional EMP/DEPT schema.
  With Oracle Server 12c, the corresponding SQL script is
  $ORACLE_HOME/rdbms/admin/utlsampl.sql.

- Ensure that your user (by default, scott) has the CREATE VIEW
  privilege.

- Create object types and views using the provided createTypes.sql
  script. For example:

     % sqlplus /nolog
     SQL> @createTypes.sql

  This script assumes that you can connect to your database with
  'scott/tiger@orcl'. If you need another connect-string, edit
  createTypes.sql.

- Oracle's ott utility needs to connect to the database in order to
  generate code from the file 'DbTypes.typ'. The default connect-
  string is "scott/tiger@orcl". If this is not appropriate, edit the
  corresponding Makefile target, or the Custom build rule for
  Visual Studio projects.

- Set the environment variable ORACLE_HOME to point to your Oracle
  installation home directory.

- On Windows with Visual Studio Project Files:

  - Oracle settings are configured using a property sheet named oracle
    that is attached to the server project configurations. Open the
    Visual Studio Property Manager from "View > Property Manager" menu.

  - Expand the server configuration and double click the oracle
    property sheet to edit it.

  - Click "Common Properties > User Macros" and set the ORACLE_HOME
    value to match your Oracle deployment.

  - Occi libraries and drivers for Visual Studio 2012 x64 are included
    with Oracle 12c. If you are using a different compiler or arch you
    will need to install the corresponding Oracle instant client
    package from:

    http://www.oracle.com/technetwork/database/features/instant-client/index-097480.html

    Follow the link corresponding to the platform you want to use and
    setup both:

    - instantclient-basic-nt-12.1.0.2.0.zip
    - instantclient-sdk-nt-12.1.0.2.0.zip

    After that set the ORACLE_INSTANTCLIENT_HOME value in the oracle
    property sheet to match your setup.

  - Make sure to save the changes to the property sheet using the save
    button in the Property Manager windows so changes take effect.

- On Windows, when using nmake Makefiles, set the ORACLE_HOME and
  ORACLE_INSTANTCLIENT_HOME variables in your enviroment to match your
  Oracle setup and review Makefile.mak.

- Then build as usual.


Running the demo
----------------

- Review the Oracle properties in the config.server file. You may need
  to change them to connect to your Oracle instance.

- Start the server:

  $ server

- Start the client in a separate window:

  $ client

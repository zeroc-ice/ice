' **********************************************************************
'
' Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports System.Diagnostics
Imports IcePackHelloDemo

Public Class HelloFactoryI
    Inherits _HelloFactoryDisp

    Public Overloads Overrides Function create(ByVal name As String, ByVal current As Ice.Current) As IcePackHelloDemo.HelloPrx
        Dim adapter As Ice.ObjectAdapter = current.adapter
        Dim communicator As Ice.Communicator = adapter.getCommunicator()

        '
        ' Create the servant and add it to the object adapter using the
        ' given name as the identity.
        '
        Dim hello As Ice.Object = New IcePackHelloI(name)
        Dim [object] As Ice.ObjectPrx = adapter.add(hello, Ice.Util.stringToIdentity(name))

        '
        ' Get the IcePack Admin interface and register the newly created
        ' Hello object with the IcePack object registry.
        ' 
        Try
            Dim admin As IcePack.AdminPrx = IcePack.AdminPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Admin"))
            admin.addObject([object])
        Catch ex As IcePack.ObjectExistsException
            '
            ' An object with the same identity is already registered with
            ' the registry. Remove the object from the object adapter and
            ' throw.
            '
            adapter.remove([object].ice_getIdentity())
            Throw New NameExistsException
        Catch ex As IcePack.DeploymentException
            Debug.Assert(False)
        End Try

        Dim id As String = communicator.getProperties().getProperty("Identity")

        Console.WriteLine("HelloFactory-" & id & ": created Hello object named '" & name & "'")

        Return HelloPrxHelper.uncheckedCast([object])
    End Function

    Public Overloads Overrides Function find(ByVal name As String, ByVal current As Ice.Current) As IcePackHelloDemo.HelloPrx
        Dim communicator As Ice.Communicator = current.adapter.getCommunicator()

        '
        ' The object is registered with the IcePack object registry so we
        ' just return a proxy containing the identity.
        '
        Try
            Return HelloPrxHelper.checkedCast(communicator.stringToProxy(name))
        Catch ex As Ice.NoEndpointException
            '
            ' The object couldn't be activated. Ignore.
            ' 
            Return HelloPrxHelper.uncheckedCast(communicator.stringToProxy(name))
        Catch ex As Ice.NotRegisteredException
            '
            ' The object is not registered.
            '
            Throw New NameNotExistException
        Catch ex As Ice.ObjectNotExistException
            '
            ' The object doesn't exist anymore. This can occur if the
            ' server has been restarted and the server objects haven't
            ' been removed from the object registry.
            ' 
            Dim admin As IcePack.AdminPrx = IcePack.AdminPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Admin"))
            Try
                admin.removeObject(Ice.Util.stringToIdentity(name))
            Catch e As IcePack.ObjectNotExistException
                Debug.Assert(False)
            End Try
            Throw New NameNotExistException
        End Try
    End Function

End Class

Imports Employees
Imports System.Collections.Generic

Public Class Sheet1

    Private Sub Sheet1_Startup(ByVal sender As Object, ByVal e As System.EventArgs) Handles Me.Startup
        Try
            communicator = Ice.Util.initialize()
            Ice.Util.initialize()

            Dim prx As Ice.ObjectPrx = _
                communicator.stringToProxy("Finder:tcp -p 10000")
            Dim finder As EmployeeFinderPrx = _
                EmployeeFinderPrxHelper.uncheckedCast(prx)

            Dim row = firstRow
            employees = finder.list()
            For Each emp As EmployeePrx In employees
                Dim details As EmployeeDetails = emp.getDetails()
                Me.Cells(row, 1) = details.name
                Me.Cells(row, 2) = details.address
                row += 1
            Next
        Catch ex As Exception
            If Not communicator Is Nothing Then
                Try
                    communicator.destroy()
                Catch
                End Try
                communicator = Nothing
            End If
        End Try
    End Sub

    Private Sub Sheet1_Shutdown(ByVal sender As Object, ByVal e As System.EventArgs) Handles Me.Shutdown
        If Not communicator Is Nothing Then
            Try
                Dim row As Integer = firstRow
                For Each emp As EmployeePrx In employees
                    emp.setAnnualSalary(Double.Parse(Me.Cells(row, 3).Value))
                    emp.setTaxPayable(Double.Parse(Me.Cells(row, 4).Value))
                    row += 1
                Next
                communicator.destroy()
            Catch
            End Try
            communicator = Nothing
        End If
    End Sub

    Private communicator As Ice.Communicator
    Private employees As List(Of EmployeePrx)
    Private Const firstRow = 10 ' Employees start on row 10 of worksheet
End Class

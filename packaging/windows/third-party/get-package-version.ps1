# Print the version of a package, as recorded in packages.json.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Package
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$packages = Get-Content -LiteralPath "$PSScriptRoot/packages.json" -Raw | ConvertFrom-Json
$entry = @($packages | Where-Object { $_.package -eq $Package })
if ($entry.Count -ne 1) {
    throw "Expected one entry for $Package in packages.json, found $($entry.Count)."
}
Write-Output $entry[0].version

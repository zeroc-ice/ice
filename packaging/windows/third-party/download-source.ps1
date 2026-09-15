# Download and verify a pinned archive before extracting or applying any patches.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('bzip2', 'libexpat', 'lmdb')]
    [string]$Dependency
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
Set-StrictMode -Version Latest

$manifest = Get-Content -LiteralPath "$PSScriptRoot/sources.json" -Raw | ConvertFrom-Json
$source = $manifest.$Dependency
if ($source.sha256 -notmatch '^[0-9a-f]{64}$' -or $source.url -notlike 'https://*') {
    throw "Invalid source URL or SHA-256 for $Dependency in sources.json."
}

$downloadDir = Join-Path $PSScriptRoot 'downloads'
$sourcesDir = Join-Path $PSScriptRoot 'sources'
$destination = Join-Path $sourcesDir $Dependency
$archive = Join-Path $downloadDir "$Dependency-$($source.sha256).tar.gz"
$stampFile = Join-Path $destination '.source-stamp'
New-Item -ItemType Directory -Force -Path $downloadDir, $sourcesDir | Out-Null

function Get-SHA256([string]$Path) {
    $stream = [IO.File]::OpenRead($Path)
    $sha256 = [Security.Cryptography.SHA256]::Create()
    try {
        $hash = $sha256.ComputeHash($stream)
        return [BitConverter]::ToString($hash).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $sha256.Dispose()
        $stream.Dispose()
    }
}

function Assert-Checksum([string]$Path) {
    $actual = Get-SHA256 $Path
    if ($actual -ne $source.sha256) {
        throw "SHA-256 mismatch for $Dependency. Expected $($source.sha256), got $actual ($Path)."
    }
}

if (!(Test-Path -LiteralPath $archive)) {
    $partial = "$archive.$([Guid]::NewGuid().ToString('N')).partial"
    try {
        Write-Host "Downloading $($source.url)"
        # Windows PowerShell 5.1 can otherwise default to older TLS protocols.
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
        Invoke-WebRequest -Uri $source.url -OutFile $partial -UseBasicParsing
        Assert-Checksum $partial
        Move-Item -LiteralPath $partial -Destination $archive -Force
    }
    finally {
        if (Test-Path -LiteralPath $partial) { Remove-Item -LiteralPath $partial -Force }
    }
}
else {
    # Recheck cached downloads too. A corrupt cache must never be extracted, even on a repeat build.
    Assert-Checksum $archive
}
Write-Host "Verified SHA-256 for $Dependency $($source.version)."

# Updating either the source pin or a patch invalidates the extracted tree. Record the stamp only after success.
$stamp = $source.sha256
foreach ($patch in $source.patches) {
    $stamp += ':' + (Get-SHA256 (Join-Path $PSScriptRoot $patch))
}
if ((Test-Path -LiteralPath $stampFile) -and
    (Test-Path -LiteralPath (Join-Path $destination $source.required_file)) -and
    ((Get-Content -LiteralPath $stampFile -Raw).Trim() -eq $stamp)) {
    Write-Host "Sources for $Dependency are ready."
    return
}

$temporary = Join-Path $sourcesDir "$Dependency-$([Guid]::NewGuid().ToString('N'))"
New-Item -ItemType Directory -Path $temporary | Out-Null
try {
    & tar -xzf $archive -C $temporary
    if ($LASTEXITCODE -ne 0) { throw "Extracting $archive failed with exit code $LASTEXITCODE." }

    $extracted = Join-Path $temporary $source.root
    if (!(Test-Path -LiteralPath (Join-Path $extracted $source.required_file))) {
        throw "The $Dependency archive does not contain $($source.root)/$($source.required_file)."
    }
    foreach ($patch in $source.patches) {
        # Treat the extracted tree as the worktree, including when invoked from a subdirectory of Ice.
        $patchPath = Join-Path $PSScriptRoot $patch
        & git -C $extracted "--work-tree=$extracted" apply $patchPath
        if ($LASTEXITCODE -ne 0) { throw "Applying $patch failed with exit code $LASTEXITCODE." }
        & git -C $extracted "--work-tree=$extracted" apply --reverse --check $patchPath
        if ($LASTEXITCODE -ne 0) { throw "Verifying $patch failed with exit code $LASTEXITCODE." }
    }

    Set-Content -LiteralPath (Join-Path $extracted '.source-stamp') -Value $stamp -Encoding ascii
    if (Test-Path -LiteralPath $destination) { Remove-Item -LiteralPath $destination -Recurse -Force }
    Move-Item -LiteralPath $extracted -Destination $destination
}
finally {
    Remove-Item -LiteralPath $temporary -Recurse -Force
}

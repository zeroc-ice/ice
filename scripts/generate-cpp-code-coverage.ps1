<#
.SYNOPSIS
Generates a code coverage report for the C++ modules and executables with Microsoft.CodeCoverage.Console.

.DESCRIPTION
Builds the C++ source tree in the current working directory with coverage instrumentation, runs the tests, and writes
the coverage data in Cobertura format to coverage\coverage.cobertura.xml. Of the Slice compilers, the report includes
only slice2cpp: the others are exercised by the builds and tests of their own language mappings, which the script
doesn't run. The report also counts the header code that only the tests use, but not the test code itself.

Run it from a Developer PowerShell for Visual Studio, so msbuild is on the PATH. Microsoft.CodeCoverage.Console ships
with Visual Studio Enterprise. Visual Studio opens the Cobertura file, and ReportGenerator turns it into HTML.

.EXAMPLE
cd cpp; ..\scripts\generate-cpp-code-coverage.ps1
#>

$ErrorActionPreference = "Stop"

if (-not (Test-Path msbuild\ice.proj)) { throw "msbuild\ice.proj not found" }
if (-not (Test-Path allTests.py)) { throw "allTests.py not found" }

$vsPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
$coverageConsole = "$vsPath\Common7\IDE\Extensions\Microsoft\CodeCoverage.Console"
if (-not (Test-Path $coverageConsole)) { throw "Microsoft.CodeCoverage.Console not found in $coverageConsole" }
$settings = "msbuild\coverage.runsettings"
$session = "cpp-coverage"

Remove-Item coverage -Recurse -Force -ErrorAction SilentlyContinue
New-Item coverage -ItemType Directory | Out-Null

# The script changes PATH and LINK for its own steps, and puts them back for the shell that runs it.
$path = $env:PATH
$link = $env:LINK
try {
    Write-Host "Building with code coverage..."
    # Instrumentation rewrites the modules in place and needs them linked with /PROFILE, so the build starts clean.
    # link.exe reads extra options from the LINK environment variable.
    msbuild /m /p:Platform=x64 /p:Configuration=Debug /t:Clean msbuild\ice.proj
    if ($LASTEXITCODE -ne 0) { throw "msbuild clean failed" }
    $env:LINK = "/PROFILE"
    msbuild /m /p:Platform=x64 /p:Configuration=Debug msbuild\ice.proj
    if ($LASTEXITCODE -ne 0) { throw "msbuild build failed" }
    $env:LINK = $link

    Write-Host "Instrumenting..."
    # The instrumented modules load static_covrun64.dll from the PATH.
    $env:PATH = "$coverageConsole;$env:PATH"
    # bzip2 and libexpat are third-party DLLs, and the other Slice compilers are exercised by the builds and tests of
    # their own language mappings.
    $modules = @(Get-ChildItem bin\x64\Debug\* -Include *.dll, *.exe -Exclude bzip2*.dll, libexpat*.dll |
        Where-Object { $_.Name -notlike "slice2*" -or $_.Name -eq "slice2cpp.exe" })
    # The test executables and plugins instantiate header templates and inline functions that the Ice modules don't.
    # The runsettings leave out the test code itself.
    $modules += Get-ChildItem test -Recurse -Include *.dll, *.exe -Exclude bzip2*.dll, libexpat*.dll |
        Where-Object { $_.DirectoryName -like "*\x64\Debug" }
    foreach ($module in $modules) {
        Microsoft.CodeCoverage.Console instrument --session-id $session --settings $settings --nologo $module.FullName
        if ($LASTEXITCODE -ne 0) { throw "instrumenting $($module.FullName) failed" }
    }

    Write-Host "Running tests..."
    # Instrumented modules report to the collector of their session, so the tests run as usual.
    Microsoft.CodeCoverage.Console collect --session-id $session --server-mode --background --settings $settings `
        --output coverage\coverage.cobertura.xml --output-format cobertura --nologo
    if ($LASTEXITCODE -ne 0) { throw "starting the coverage collector failed" }
    python allTests.py --all --workers=$env:NUMBER_OF_PROCESSORS --platform=x64 --config=Debug
    $testsExitCode = $LASTEXITCODE
    # Stop the collector before reporting a test failure, so it doesn't outlive the script.
    Microsoft.CodeCoverage.Console shutdown $session --nologo
    if ($LASTEXITCODE -ne 0) { throw "stopping the coverage collector failed" }
    if ($testsExitCode -ne 0) { throw "tests failed" }
}
finally {
    $env:PATH = $path
    $env:LINK = $link
}
Write-Host "Coverage data written to coverage\coverage.cobertura.xml"

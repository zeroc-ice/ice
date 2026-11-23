<#
.SYNOPSIS
Generates a code coverage report for the C++ modules and executables with Microsoft.CodeCoverage.Console.

.DESCRIPTION
Builds the C++ source tree in the current working directory with coverage instrumentation, runs the tests, writes
the coverage data in Cobertura format to coverage\coverage.cobertura.xml and in lcov format to coverage\lcov.info, and
cleans the build. Of the Slice compilers, the report includes only slice2cpp: the others are exercised by the builds and
tests of their own language mappings, which the script doesn't run. The report also counts the header code that only
the tests use, but not the test code itself.

The Cobertura file reports the code of each module apart, and of each template instantiation in it. The lcov file
merges these copies, so each source line counts once, and is covered when any copy of it runs.

Run it from a Developer PowerShell for Visual Studio, so msbuild is on the PATH. Microsoft.CodeCoverage.Console ships
with Visual Studio Enterprise. Visual Studio opens the Cobertura file, and ReportGenerator turns the lcov file into
HTML.

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
    # The instrumented modules don't load without the coverage runtime, and MSBuild sees them as up to date.
    msbuild /m /p:Platform=x64 /p:Configuration=Debug /t:Clean msbuild\ice.proj
    if ($LASTEXITCODE -ne 0) { throw "msbuild clean failed" }
    if ($testsExitCode -ne 0) { throw "tests failed" }
}
finally {
    $env:PATH = $path
    $env:LINK = $link
}

Write-Host "Merging coverage data..."
# The Cobertura file has a class for each template instantiation in each module that compiles it, and ReportGenerator
# counts the lines of each class apart. The lcov file has a record for each source file instead, where a line counts
# once, as with llvm-cov. A method's instantiations share its first line, which keys it. Microsoft.CodeCoverage records
# whether a line ran, not how often, so each count is the number of copies that ran.
$files = @{}
foreach ($class in ([xml](Get-Content coverage\coverage.cobertura.xml -Raw)).SelectNodes("//class")) {
    $filename = $class.GetAttribute("filename")
    if (-not $files.ContainsKey($filename)) {
        $files[$filename] = @{ Hits = @{}; Methods = @{}; MethodHits = @{} }
    }
    $file = $files[$filename]
    foreach ($line in $class.SelectNodes("lines/line")) {
        $file.Hits[[int]$line.GetAttribute("number")] += [long]$line.GetAttribute("hits")
    }
    foreach ($method in $class.SelectNodes("methods/method")) {
        $lines = $method.SelectNodes("lines/line")
        $first = [int]$lines[0].GetAttribute("number")
        $file.Methods[$first] = $class.GetAttribute("name") + "::" + $method.GetAttribute("name") +
            $method.GetAttribute("signature")
        # The first line of a flex or bison function can be a rule that didn't run, so any line counts.
        $ran = $method.SelectNodes("lines/line[@hits != '0']").Count -gt 0
        $file.MethodHits[$first] += [int]$ran
    }
}
$lcov = foreach ($filename in $files.Keys | Sort-Object) {
    $file = $files[$filename]
    $firstLines = $file.Methods.Keys | Sort-Object
    "SF:$filename"
    foreach ($first in $firstLines) { "FN:$first,$($file.Methods[$first])" }
    foreach ($first in $firstLines) { "FNDA:$($file.MethodHits[$first]),$($file.Methods[$first])" }
    foreach ($number in $file.Hits.Keys | Sort-Object) { "DA:$number,$($file.Hits[$number])" }
    "end_of_record"
}
Set-Content coverage\lcov.info $lcov
Write-Host "Coverage data written to coverage\coverage.cobertura.xml and coverage\lcov.info"

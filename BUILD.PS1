param (
  [Parameter(Mandatory = $true)]
  [ValidateSet("debug", "release")]
  $config
)

$common_flags = @("/std:c++latest", "/utf-8", "/Iinclude/mttlib", "/c", "/W4")
$source_files = Get-ChildItem "src\*" -Name | ForEach { "src\$_" }
$library_name = "mttlib"

switch ($config) {
  "debug" {
    $binary_directory = "bin\debug\"
    $flags = @("/Fo:$binary_directory", "/Fd:$binary_directory", "/Zi", "/Od", "/MDd")
  }
  "release" {
    $binary_directory = "bin\release\"
    $flags = @("/Fo:$binary_directory", "/Fd:$binary_directory", "/O2", "/MD")
  }
}

if (-not (Test-Path $binary_directory)) { New-Item -ItemType Directory -Path $binary_directory -Force | Out-Null }

& cl @common_flags @flags @source_files
$object_files = Get-ChildItem "$binary_directory*.obj" | ForEach-Object { $_.FullName }
& lib /OUT:"$binary_directory$library_name.lib" $object_files
@echo off

if not exist "nuget\nuget.exe" (
    if not exist "nuget" mkdir nuget
    curl --silent https://dist.nuget.org/win-x86-commandline/latest/nuget.exe -o nuget\nuget.exe
)

nuget\nuget.exe install Microsoft.Direct3D.DXC -ExcludeVersion -OutputDirectory nuget\packages
nuget\nuget.exe install Microsoft.Direct3D.D3D12 -ExcludeVersion -OutputDirectory nuget\packages

call shader.bat
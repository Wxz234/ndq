@echo off

set COMPILER_PATH=.\nuget\packages\Microsoft.Direct3D.DXC\build\native\bin\x86\dxc.exe

%COMPILER_PATH% -T vs_6_6 -E main -O3 .\examples\02-triangle\vertex.hlsl -Fh .\examples\02-triangle\vertex.h -Vn VertexBlob
%COMPILER_PATH% -T ps_6_6 -E main -O3 .\examples\02-triangle\pixel.hlsl -Fh .\examples\02-triangle\pixel.h -Vn PixelBlob
@echo off

set COMPILER_PATH=.\nuget\packages\Microsoft.Direct3D.DXC\build\native\bin\x86\dxc.exe

%COMPILER_PATH% -T vs_6_6 -E main -O3 .\examples\Triangle\Vertex.hlsl -Fh .\examples\Triangle\Vertex.h -Vn vertexBlob
%COMPILER_PATH% -T ps_6_6 -E main -O3 .\examples\Triangle\Pixel.hlsl -Fh .\examples\Triangle\Pixel.h -Vn pixelBlob
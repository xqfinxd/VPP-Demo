@echo off

setlocal enabledelayedexpansion

for %%i in (
  simple.vert,
  simple.frag,
  vertexbuffer.vert,
  vertexbuffer.frag,
) do (
  echo compile %%i to %%i.spv
  glslc.exe %%i -o %%i.spv
)

pause
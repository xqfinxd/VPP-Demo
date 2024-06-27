@echo off

setlocal enabledelayedexpansion

for %%i in (
  basic.vert,
  basic.frag
) do (
  echo compile %%i to %%i.spv
  glslc.exe %%i -o %%i.spv
)

pause
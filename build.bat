@echo off

if "%~1" == "clean" (
  del *.exe
) else if "%~1" == "build" (
  if "%~2" == "debug" (
    gcc -g -DDEBUG_MODE main.c heap.c -o program_debug.exe
  ) else (
    gcc -o0 main.c heap.c -o program_o0.exe
    gcc -o1 main.c heap.c -o program_o1.exe
    gcc -o2 main.c heap.c -o program_o2.exe
    gcc -os main.c heap.c -o program_os.exe
    gcc -os main.c heap.c -o program_oz.exe
  )
) else (
  echo Usage: build.bat build [debug|release]
)
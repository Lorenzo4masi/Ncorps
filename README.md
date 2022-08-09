# Ncorps
Ncorps (this program should be executed only via prompt without resizing the window)

If you want to compile this program the syntax is simple: gcc ncorps.c InOut.c basCom.c dbgFun.c Fnt.c gfxstdfont.c -o ncorps.exe

I've put all the infos in the help of ncorps: "ncorps /?"

I've also included an example file (example.txt) to try with the /F switch.

You probably wont be able to set the pixel dimension to 1 because of a bug, here is the source:

https://www.reddit.com/r/C_Programming/comments/s6463m/error_with_windows_api/?utm_source=share&utm_medium=web2x&context=3

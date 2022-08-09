#include "basgfx6.h"

int _p_err(int err_c, char *fmt,...)
{
    #ifdef GFX_DBG
        printf("GFX_DBG: ");
        va_list arg;
        va_start(arg,fmt);
            vprintf(fmt,arg);
                    char c[100];
        vsprintf(c,fmt,arg);
        SetConsoleTitle(c);
        va_end(arg);
        putchar('\n');

    #endif
    
    return err_c;
}

void _g_ferr(int new_c, int *err_c)
{    *err_c = *err_c == 0 && new_c != 0 ? new_c : *err_c;   }
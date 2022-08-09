#include "basgfx6.h"



font *gfxLoadFont(char *path)
{
    FILE *buff = fopen(path,"r");
    font *ret = (font *) malloc(sizeof(font));

    char tmp[100];
    int acqdim = 0;
    int ln = 0; // line of letter
    
    for (int i = 0; i < 256;++i)
        ret->letter[i].char_c = '\0'; 
    
    
    while (fgets(tmp, 100, buff) != NULL)  // parse the text
    {
        tmp[strlen(tmp) - 1] = '\0';

        if (tmp[0] == '!') // comando speciale
        {
            if (strncmp(tmp + 1,"TITLE:",sizeof("TITLE:") - 1) == 0) // title
                ret->name = tmp + 1 + sizeof("TITLE:") - 1;
            else if (strncmp(tmp + 1,"X:",sizeof("X:") - 1) == 0)
            {
                // acquire the dimension
                ret->Xdim = atoi(tmp + 1 + sizeof("X:") - 1);
                ++acqdim;                
            }
            else if (strncmp(tmp + 1,"Y:",sizeof("Y:") - 1) == 0)
            {
                // acquire the dimension
                ret->Ydim = atoi(tmp + 1 + sizeof("Y:") - 1);
                ++acqdim;                
            }
            else if (strncmp(tmp + 1,"NOTEXIST:",sizeof("NOTEXIST:") - 1) == 0)
                ++ln;

            else if (tmp[1] != ' ' && tmp[1] != '\t')
            {
                if (acqdim != 2)
                {
                    free(ret);
                    return NULL;
                }

                ret->letter[ln].char_c = tmp[1];

                char **tmps = (char **) malloc(sizeof(char) * ret->Ydim);

                if (tmps == NULL)
                {
                    free(tmps);
                    free(ret);
                    return NULL;
                }
                for (int i = 0;i < ret->Ydim;++i)
                {
                    char *ps;

                    if ((ps = (char *) malloc(sizeof(char) * (ret->Xdim + 1))) == NULL)  // + '\0'
                    {
                        free(tmps);
                        free(ret);
                        return NULL;
                    }
                    
                    fgets(ps, 100, buff);
                    ps[strlen(ps) - 1] = '\0';
                    tmps[i] = ps;
                }
                ret->letter[ln++].s = tmps; 
            }
        }
    }

    freopen(path,"r",buff);
    return ret;
}




int gfxFontWrite(window t,font *fnt,int y, int x, int k, char *fmt, ...)
{
    t->fntY_c = y;
    t->fntX_c = x;

    char s[300];

    char tvbuf[fnt->Ydim * k + 1][fnt->Xdim * k + 1];

    va_list arg;
    va_start(arg, fmt);
        vsprintf(s, fmt, arg);
    va_end(arg);

    for (int i = 0; i < strlen(s); ++i)
    {
        if (s[i] == '\n')
        {
            t->fntY_c += fnt->Ydim * k + 1;
            t->fntX_c = 0;
        }
        if (s[i] == ' ')
            t->fntX_c += 3 * k;
        if (s[i] == '\t')
            t->fntX_c += 9;
        else
        {
            if (t->fntX_c >= t->Xdim)
            {
                t->fntY_c += fnt->Ydim * k + 1;
                t->fntX_c = 0;
            }
            if (t->fntY_c >= t->Ydim)
                return BADY;

            for (int a = 0; a < 256; ++a)
            {
                if (s[i] == fnt->letter[a].char_c)
                {
                    // dilata il carattere trovato


                        
                        for (int j = 0; j < fnt->Ydim * k + 1; ++j)
                            memset(tvbuf[j],'\0',fnt->Xdim * k + 1);

                        int yf = 0;
                        int xf = 0;

                        int stx = 0;
                        int sty = 0;

                        for (int yt = 0; yt < fnt->Ydim; ++yt)
                        { 
                            for (int xt = 0; xt < fnt->Xdim; ++xt)
                            {
                                yf = sty;
                                for (; yf < k + sty; ++yf)
                                { 
                                    xf = stx;
                                    for (; xf < k + stx; ++xf)
                                    {
                                        tvbuf[yf][xf] = fnt->letter[a].s[yt][xt];
                                    }
                                }
                                stx += k;
                            }
                            sty += k;
                            stx = 0;
                        }
                    
                    
                    // stampa il carattere
                    for (int y_r = 0; y_r < fnt->Ydim * k;++y_r)
                        gfxDrawStringTerm(t, y_r + t->fntY_c,t->fntX_c,tvbuf[y_r]);
                    
                    t->fntX_c += (fnt->Xdim + 2 ) * k;
                    break;
                }
            }
        }
    }
    return 0;
}


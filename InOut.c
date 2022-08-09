// header di funzioni per io sulla console

#include "basgfx6.h"

#define NOTINIT -1

HANDLE hConsole;
HANDLE hOriginal;
CONSOLE_CURSOR_INFO cci;
window lastStmp = NULL;
SMALL_RECT Rect;

void updlist(int c, window t);


int gfxInit(int Ydim, int Xdim, int Fy, int Fx, int MODE)
{   
    if ((hOriginal = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE || 
    (hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL)) == INVALID_HANDLE_VALUE)
        return CANTINITIALIZE; //err

    if (!SetConsoleActiveScreenBuffer(hConsole))
        return CANTINITIALIZE;


        //  Dimensiona al minimo la finestra perchè il buffer potrebbe diventare più piccolo della dimensione attuale 
        //  (ma non potrà essere minore di 0,0,1,1)
    SMALL_RECT h = {0,0,1,1};
    if (!SetConsoleWindowInfo(hConsole, TRUE, &h))
        return CANTINITIALIZE;

        // Cambia le caratteristiche del font e lo imposta 

    if (Fx <= 0 || Fy <= 0)
        return CANTINITIALIZE;

    CONSOLE_FONT_INFOEX cfx;
    cfx.cbSize = sizeof(cfx);
    cfx.dwFontSize.X = Fx;
    cfx.dwFontSize.Y = Fy;
    cfx.nFont = 0;
    cfx.FontFamily = FF_DONTCARE;
    cfx.FontWeight = FW_NORMAL;
    wcscpy(cfx.FaceName, L"Consolas");

    if (!SetCurrentConsoleFontEx(hConsole, FALSE, &cfx))
        return CANTINITIALIZE;
    
        // Crea il buffer della dimensione voluta
    COORD a = {Xdim, Ydim};
    
    if (MODE == MAXSIZE)
        a = GetLargestConsoleWindowSize(hConsole);
    else if (Xdim <= 0 || Ydim <= 0)
        return CANTINITIALIZE;

    if (!SetConsoleScreenBufferSize(hConsole, a))
        return CANTINITIALIZE;
    

        //  Dimensiona la finestra fisica in modo che abbia la stessa dimensione del buffer
    Rect.Bottom = a.Y -1;
    Rect.Left = 0;
    Rect.Right = a.X -1;
    Rect.Top = 0;

    if (!SetConsoleWindowInfo(hConsole, TRUE, &Rect))
        return CANTINITIALIZE;
    
    if (!GetConsoleCursorInfo(hConsole, &cci))
        return CANTINITIALIZE;

    gfxTurnCursor();

    return 0;
}


// Changes the Ydim and Xdim parameters
window gfxNewWindowDimension(int Ynew,int Xnew)
{
    if (Ynew < 0)
        return NULL;//_p_err(BADY,"gfxNewWindowDimension: BAD Y Dimension: %d for window: %p (err_c = BADY)",Ynew,t);

    else if (Xnew < 0)
        return NULL;//_p_err(BADX,"gfxNewWindowDimension: BAD X Dimension: %d for window: %p (err_c = BADX)",Xnew,t);
    
    CHAR_INFO *tmps;

    if ((tmps = (CHAR_INFO*) malloc(sizeof(CHAR_INFO) * Ynew * Xnew + 1)) == NULL)
        return NULL; //_p_err(NOSPACE,"gfxNewWindowDimension: Not enought space for window creation: Location: %p (err_c = NOSPACE)",t)

    int *tmpjmp;
    
    if ((tmpjmp = (int *) malloc(sizeof(int) * Ynew * Xnew)) == NULL)
    {
        free(tmps);
        return NULL;
    }

    memset(tmps,0,sizeof(CHAR_INFO) * Ynew * Xnew);
    memset(tmpjmp,NOTINIT,sizeof(int) * Ynew * Xnew);
    

    window t = malloc(sizeof(struct __win));    // analogo a char *p = malloc(sizeof(char)) e non char *p = malloc(sizeof(char *))

        // black on black
    t->crr_col = 0 | (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
    t->s = tmps;
    t->jmp = tmpjmp;
        
    for (register int i = 0; i < MAXWINDOWSATTACHED;++i)
        t->win[i] = NULL;
        
    t->title = NULL;
    t->focus = NULL;

    t->Ydim = Ynew;
    t->Xdim = Xnew;

    t->ORy = Ynew / 2;
    t->ORx = Xnew / 2;

    t->Yscale = 1;
    t->Xscale = 1;

    t->Ysign = POSY;
    t->Xsign = POSX;

    t->Yproj = 0;
    t->Xproj = 0;

    t->line_t = 0;
    t->frst = t->ppos = NOTINIT;

    t->Yws = 0;
    t->Xws = 0;

    t->fntY_c = 0;
    t->fntX_c = 0;
        
    return t;
}


int gfxSetColour(window t,int c)
{
    t->crr_col = c;
    return 0;
}


int gfxTerminate(void)
{   
    if (!SetConsoleActiveScreenBuffer(hOriginal))
        return _p_err(CANTCLOSE,"WINgfxTerminate: Can't restore the prevoius screen buffer (err_c = CANTCLOSE)");
            
    return 0;
}

inline void gfxCleanVideo(void)
{
    COORD P = {0,0};
    DWORD c;
    
        // pulisci NON CI DOVREBBE ESSERE BISOGNO DEI CARATTERI
    FillConsoleOutputAttribute(hConsole,0,gfxWindY() * gfxWindX(),P,&c);

    lastStmp = NULL;
}

void gfxShowFast(window t)
{      
    int next = 0;

    COORD P = {0,0};
    DWORD c;

    if (t != lastStmp)   // caso in cui ci sia un cambio di finestra
    {
        gfxShow(t);
        return;
    }

    lastStmp = t;

    if (t->frst == NOTINIT) // non ci sono aggiornamenti
        return;

    for (int i = t->frst;;i = next)
    {
        P.Y = i / t->Xdim;
        P.X = i - P.Y * t->Xdim;

        FillConsoleOutputAttribute(hConsole,t->s[i].Attributes,1,P,&c);
        FillConsoleOutputCharacter(hConsole,t->s[i].Char.AsciiChar,1,P,&c);

        if (t->jmp[i] == NOTINIT)
            break;
        
        next = t->jmp[i];
        t->jmp[i] = NOTINIT;
    }

    t->frst = NOTINIT;
}



void gfxShow(window t)
{
    COORD P = {0,0}, DIM = {t->Xdim,t->Ydim};
    SMALL_RECT writeArea = {0,0,t->Xdim - 1 ,t->Ydim - 1};

    if (lastStmp != NULL && t != lastStmp)
        gfxCleanVideo();
    
    WriteConsoleOutput(hConsole,t->s,DIM,P,&writeArea);

    // ocho sto eliminando la coda di un'altra finestra
    //t->frst = NOTINIT;  // cancella la coda (ormai ha stampato tutto) // colpa sua

    lastStmp = t;
}



//cleans the target array
void gfxCleanTable(window t)
{    
    // crea la lista dei soli caratteri non neri

    int i = 0;
    t->frst = NOTINIT;

    for (; i < t->Ydim * t->Xdim; ++i)
        if (t->s[i].Attributes != 0)
        {
            t->s[i].Attributes = 0;
            if (t->frst == NOTINIT)
                t->frst = t->ppos = i;
            else
            {
                t->jmp[t->ppos] = i;
                t->ppos = i;
            }
        }
}


// prima di distruggere, scollegare da eventuali collegamenti,in caso dovesse essere riusato
int gfxDestroyWindow(window t)
{
    // distrugge il buffer
    free(t->s);
    free(t->title);
    free(t->jmp);

    for (int i = 0; t->win[i] != NULL; ++i)
        t->win[i] = NULL;   

    free(t); 

    return 0;
}


// Draw a "point" in the target vector at the y,x locations
int __gfxDrawPoint(char point, window t, int y, int x)
{
    if (y < 0 || y >= t->Ydim)
        return _p_err(BADY,"gfxDrawPoint: BAD Y: %d (err_c = BADY)",y);
    else if (x < 0 || x >= t->Xdim)
        return _p_err(BADX,"gfxDrawPoint: BAD X: %d (err_c = BADX)",x);

    int c = (y * t->Xdim) + x;

    if (t->s[c].Char.AsciiChar == point && t->s[c].Attributes == t->crr_col)
        return 0;

    t->s[c].Char.AsciiChar = point;
    t->s[c].Attributes = t->crr_col;

    // registra il punto

    updlist(c, t);


    return 0;
}


int __cartDrawPoint(char point, window t, double y, double x, double Yscale, double Xscale)
{
    // dilatazione con centro non nell'origine
    
    x = (x + t->ORx) * Xscale + (t->Xdim /2) * (1 - Xscale);
    y = (t->ORy - y) * Yscale + (t->Ydim /2) * (1 - Yscale);

    // gestione del segno
    y *= t->Ysign;
    x *= t->Xsign;
        
    if (y < 0 || y >= t->Ydim )
        return _p_err(BADY,"cartDrawPoint: BAD Y: %f (err_c = BADY)",y);
    else if (x < 0 || x >= t->Xdim)
        return _p_err(BADX,"cartDrawPoint: BAD X: %f (err_c = BADX)",x);

    int c = (int)y * t->Xdim + (int)x;

    if (t->s[c].Char.AsciiChar == point && t->s[c].Attributes == t->crr_col)
        return 0;
    
    t->s[c].Char.AsciiChar = point;
    t->s[c].Attributes = t->crr_col;

    updlist(c, t);

    return 0;
}


void updlist(int c, window t)
{
    if (t->frst == NOTINIT) // se è il capo della lista
    {
        t->frst = t->ppos = c;
        return;
    }

    if (t->jmp[c] == NOTINIT)
    {
        t->jmp[t->ppos] = c;
        t->ppos = c;
    }
}



int gfxShowDesk(window bcg)
{
    int nwind;
    COORD P = {0, 0}, DIM = {bcg->Xdim,1};
    SMALL_RECT writeArea = {0,0,bcg->Xdim,0};
    CHAR_INFO tmps[bcg->Xdim];

    if (lastStmp != NULL && bcg != lastStmp)
        gfxCleanVideo();
    
    lastStmp = bcg;

    for (nwind = 0;bcg->win[nwind] != NULL;++nwind)  ;   // conta il numero di finestre collegate

    if (nwind == 0)
    {
        gfxShow(bcg);
        return _p_err(WINDOWSNOTATTACHED,"gfxShowDesk: No windows are attached to window: %p (err_c = WINDOWSNOTATTACHED)",&bcg);
    }

    for (int y = 0; y < bcg->Ydim; ++y)
    {
        int yb = y * bcg->Xdim;
        memcpy(tmps,bcg->s + yb, sizeof(CHAR_INFO) * bcg->Xdim);
        for (int i = 0; i < nwind; ++i)
            if (y >= bcg->win[i]->Yproj && y - bcg->win[i]->Yproj < bcg->win[i]->Ydim)
                memcpy(tmps + bcg->win[i]->Xproj, bcg->win[i]->s + (y - bcg->win[i]->Yproj) * bcg->win[i]->Xdim, sizeof(CHAR_INFO) * bcg->win[i]->Xdim);        

        
        WriteConsoleOutput(hConsole, tmps, DIM, P, &writeArea);

        writeArea.Bottom++;
        writeArea.Top++;
    }
        
    return 0;
}



bool gfxTurnCursor(void)
{
    cci.bVisible = cci.bVisible == true ? false : true;

    SetConsoleCursorInfo(hConsole,&cci);

    return cci.bVisible;
}

int gfxWindY(void)
{   return Rect.Bottom + 1;   }

int gfxWindX(void)
{   return Rect.Right + 1;    }



int gfxGetColour(window t)
{    return t->crr_col;     }

char gfxReadPoint(window t, int y, int x)
{
    if (y < 0 || y >= t->Ydim)
        return _p_err(BADY,"gfxReadPoint: BAD Y: %d (err_c = BADY)",y);
    else if (x < 0 || x >= t->Xdim)
        return _p_err(BADX,"gfxDrawPoint: BAD X: %d (err_c = BADX)",x);

    return t->s[(y * t->Xdim) + x].Char.AsciiChar;
}

char cartReadPoint(window t, double y, double x, double Yscale, double Xscale)
{
    // dilatazione con centro non nell'origine
    
    x = (x + t->ORx) * Xscale + (t->Xdim /2) * (1 - Xscale);
    y = (t->ORy - y) * Yscale + (t->Ydim /2) * (1 - Yscale);

    // gestione del segno
    y *= t->Ysign;
    x *= t->Xsign;
        
    if (y < 0 || y >= t->Ydim )
        return _p_err(BADY,"cartReadPoint: BAD Y: %f (err_c = BADY)",y);
    else if (x < 0 || x >= t->Xdim)
        return _p_err(BADX,"cartDrawPoint: BAD X: %f (err_c = BADX)",x);

    return t->s[((int)y * t->Xdim) + (int)x].Char.AsciiChar;
}



  
    /*COORD P = {0,0}, DIM = {t->Xdim,t->Ydim};
    SMALL_RECT writeArea = {0,0,t->Xdim - 1 ,t->Ydim - 1};
    
    
    WriteConsoleOutput(hConsole,t->s,DIM,P,&writeArea);*/


    // se il contenuto è costante :: questo
    // altrimenti generico

    // se molto frammentato e costante: SELEZIONE
    // se molto frammentato e incostante: SMISTAMENTO / TOTALE
    // se poco frammentato e costante: SELEZIONE
    // se poco frammentato e incostante: SMISTAMENTO / TOTALE

  
    /*COORD P = {0,0};

    for (int y = 0; y < t->Ydim; ++y)
    {
            for (int x = 0; x < t->Xdim; ++x)
            {
                int stx = x;
                int sty = y;

                for (int i = y * t->Xdim + x; i < t->Ydim * t->Xdim && (t->s[i].Attributes != t->back[i].Attributes 
                                                                    || t->s[i].Char.AsciiChar != t->back[i].Char.AsciiChar);++i)
                {
                    y = i / t->Xdim;
                    x = i - y * t->Xdim;
                    t->back[i].Attributes = t->s[i].Attributes;
                    t->back[i].Char.AsciiChar = t->s[i].Char.AsciiChar;
                }

                if (stx != x || sty != y)
                {
                COORD DIM = {x - stx + 1, y - sty + 1};
                SMALL_RECT writeArea = {stx, sty, x, y};

                WriteConsoleOutput(hConsole,t->s + sty * t->Xdim + stx,DIM,P,&writeArea);
                }
            }
    }*/






    /*
    long nupd = 0, ncaos = 0;
    for (int i = 0; i < t->Ydim * t->Xdim; ++i)
    {
        if (t->s[i].Attributes != t->back[i].Attributes ||
            t->s[i].Char.AsciiChar != t->back[i].Char.AsciiChar)
            ++nupd;
        if (i <= t->Ydim * t->Xdim)
            if (t->s[i].Attributes != t->s[i + 1].Attributes ||
            t->s[i].Char.AsciiChar != t->s[i + 1].Char.AsciiChar)
            ++ncaos;
    }
    int nc = 0;
    fprintf(stdout,"caos: %d\n",ncaos);
    if (nupd == 0)
    {
        fprintf(stdout,"NO Changes at all\n");
        fprintf(stdout,"-^-0 chiamate per frame-^-\n");
        return;
    }
    else if (nupd >= t->Ydim * t->Xdim / 2)
    {
        COORD P = {0,0}, DIM = {t->Xdim,t->Ydim};
        SMALL_RECT writeArea = {0,0,t->Xdim - 1 ,t->Ydim - 1};

        memcpy(t->back,t->s,sizeof(CHAR_INFO) * nupd);
        
        WriteConsoleOutput(hConsole,t->s,DIM,P,&writeArea);
        fprintf(stdout,"Printed all\n");
        fprintf(stdout,"-^-Una chiamata per frame-^-\n");
    }
    else
    {      
        bool complet = false;

        for (int i = 0, st = 0; nupd > 0 ; ++i)
        {
            if (t->s[i].Attributes != t->back[i].Attributes || t->s[i].Char.AsciiChar != t->back[i].Char.AsciiChar)
            {
                --nupd;
                if (!complet)
                    st = i;
                
                complet = true;

                t->back[i] = t->s[i];
            }
            else
                if (complet)
                {
                    int sty = st / t->Xdim;
                    int stx = st - sty * t->Xdim;

                    COORD P = {stx, sty};

                    if (i == st + 1) // solo un carattere
                    {
                        // P rappresenta la posizone a video
                        DWORD c;
                        FillConsoleOutputAttribute(hConsole,t->s[st].Attributes,1,P,&c);
                        FillConsoleOutputCharacter(hConsole,t->s[st].Char.AsciiChar,1,P,&c);
                        fprintf(stdout,"One char updated\n");
                    }
                    else    // Passo un sottovettore
                    {
                        // Qua P rappresenta la posizione all'interno del buffer
                        P.X = 0;
                        P.Y = 0;

                        int y = i / t->Xdim;
                        int x = i - y * t->Xdim;

                        COORD DIM = {x - stx + 1, y - sty + 1};
                        SMALL_RECT writeArea = {stx, sty, x, y};

                        WriteConsoleOutput(hConsole,t->s + sty * t->Xdim + stx,DIM,P,&writeArea);
                        fprintf(stdout,"Updated %d chars\n",i - st);
                    }

                    complet = false;
                    ++nc;
                }
        }
        fprintf(stdout,"-^-%d chiamate per frame-^-\n",nc);
    }

    */
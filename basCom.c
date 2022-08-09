// sorgente funzioni comuni

#include "basgfx6.h"

#define MAX(a,b) (a > b) ? (a) : (b)


// scambia la priorità di 2 finestre collegate al background desk, non fa nulla se w1 == w2
// se almeno una delle finestre non è collegata, esce
int gfxSwapPrior(window w1,window w2, window desk)
{
    // trova w1 e w2 e salva le posizioni
    bool fw1 = false;
    bool fw2 = false;

    if (w1 == w2)
        return _p_err(SAMEWINDOW,"gfxSwapPrior: Trying to swap the same window: %p (err_c = SAMEWINDOW)",w1);

    int off1,off2;
    
    for (register int i = 0;i < MAXWINDOWSATTACHED && !(fw1 && fw2);++i) // da ricontrollare
        if (desk->win[i] == w1)
        {
            off1 = i;
            fw1 = true;
        }
        else if (desk->win[i] == w2)
        {
            off2 = i;
            fw2 = true;
        }
    
    if (fw1 && fw2)
    {
        desk->win[off1] = w2;
        desk->win[off2] = w1;
        return 0;
    }

    return _p_err(WINDOWSNOTATTACHED,"gfxSwapPrior: At least one of the windows is not connected to background window: %p (err_c = WINDOWSNOTATTACHED)",desk);
}


// collega una finestra ad un determinato background in cima
int gfxLink(window top,window bcg)
{
    if (top == bcg)
        return _p_err(SAMEWINDOW,"gfxLink: Trying to link a window with itself: %p (err_c = SAMEWINDOW)",top);

    int i = 0;
    for (;bcg->win[i] != NULL;++i)
        if (bcg->win[i] == top) // c'è già
            return _p_err(WINDOWALRDATTACHED,"gfxLink: The window: %p is already attached to background window: %p (err_c = WINDOWALRDATTACHED)",top,bcg);
    
    if (i >= MAXWINDOWSATTACHED)
        return _p_err(NOSPACE,"gfxLink: Too many windows attached (max %d) to window: %p (err_c = NOSPACE)",MAXWINDOWSATTACHED,bcg);
    
      // aggiunge top in cima
    bcg->win[i] = top;
    bcg->focus = top;

    return 0;
}


// scollega una finestra dal background e quindi imposta il focus alla finestra subito prima di questa
// se la finestra non è collegata, esce
int gfxDeLink(window top,window bcg)
{
    if (top == bcg)
        return _p_err(SAMEWINDOW,"gfxDeLink: Trying to delink a window from itself: %p (err_c = SAMEWINDOW)",top); 

    for (int i = 0;bcg->win[i] != NULL;++i)
        if (bcg->win[i] == top)     // se trovata
        {
            for (;bcg->win[i - 1] != NULL;++i)  // finchè la finestra non è "isolata dopo la fine"
                gfxSwapPrior(bcg->win[i],bcg->win[i + 1],bcg);
            
            bcg->win[i] = NULL;
            top->Yproj = 0;
            top->Xproj = 0;
            bcg->focus = bcg->win[i - 2];   // -2 perché fine lista e finestra appena scollegata
            return 0;
        }

    return _p_err(WINDOWSNOTATTACHED,"gfxDeLink: The window: %p is not connected to background window: %p (err_c = WINDOWSNOTATTACHED)",top,bcg);
}


// imposta una finestra come focus del background in modo da mantenere l'ordine di precedenza
// se la finestra non è stata collegata al background, esce
int gfxChangeFocus(window newtop, window bcg)
{
    if (newtop == bcg)
        return _p_err(SAMEWINDOW,"gfxChangeFocus: Trying to change focus with itself: %p (err_c = SAMEWINDOW)",newtop); 

    bool found = false;

    int i = 0;
    for (;bcg->win[i] != NULL;++i)  
        if (bcg->win[i] == newtop) 
        {
            found = true;
            break;
        }
    
    if (!found)
        return _p_err(WINDOWSNOTATTACHED,"gfxChangeFocus: The window: %p is not attached to background window: %p (err_c = WINDOWSNOTATTACHED)",newtop,bcg);

    for (;bcg->win[i] != NULL;++i)     // mantiene comunque l'ordine di precendenza
        gfxSwapPrior(newtop,bcg->win[i],bcg); 

    bcg->focus = newtop;
    return 0;
}


// aggiunta dinamica
int gfxMoveOn(window top,int dy,int dx,window bcg)
{    
    int err_c = gfxLink(top,bcg);
    __gfxPos(top,dy,dx,bcg,true); 
    return err_c;
}


// posizione statica
int gfxSetOn(window top,int dy,int dx,window bcg)
{
    int err_c = gfxLink(top,bcg);
    __gfxPos(top,dy,dx,bcg,false);
    return err_c;
}


void __gfxPos(window top,int dy,int dx,window bcg, bool add)
{
    if (dy < 0)
        if (dy + top->Yproj < 0)
            top->Yproj  = 0;
        else
            top->Yproj = (add ? top->Yproj + dy : dy); 
    else
        if (dy + top->Yproj  + top->Ydim  >= bcg->Ydim)
            top->Yproj += bcg->Ydim - (top->Ydim + top->Yproj);
        else
            top->Yproj = (add ? top->Yproj + dy : dy); 
    
    if (dx < 0)
        if (dx + top->Xproj < 0)
            top->Xproj  = 0;
        else
            top->Xproj = (add ? top->Xproj + dx : dx); 
    else
        if (dx + top->Xproj  + top->Xdim >= bcg->Xdim)
            top->Xproj += bcg->Xdim - (top->Xdim + top->Xproj);
        else
            top->Xproj = (add ? top->Xproj + dx : dx);

    // collegare alla finestra top
}


// il titolo è una cosa visiva esterna, e quindi rimane solo quì e non all'inizializzazione
// il titolo va mantenuto per comodità
// rimuovere var titolo perché è inutile?
// var titolo si può usare per comparare le finestre oltre che con l'indirizzo
int gfxWindBorder(window t,char *title)
{
    int err_c = 0;

    for (int i = 0; i < t->Ydim;++i)
        __gfxDrawPoint(0xba,t,i,0);
        
    for (int i = 0; i < t->Ydim;++i)
        __gfxDrawPoint(0xba,t,i,t->Xdim - 1);

    for (int i = 0; i < t->Xdim;++i)
        __gfxDrawPoint(0xcd,t,0,i);
    for (int i = 0; i < t->Xdim;++i)
        __gfxDrawPoint(0xcd,t,t->Ydim - 1,i);
    
    
    t->title = title;
    ++err_c;

    gfxDrawStringTerm(t,0,1,title);

    /*
    if (strncpy(t->s[0] + 1,title,strlen(title)) < 0)
        err_c = _p_err(NOSPACE,"gfxWindBorder: NO SPACE for the  requested title on window: %p (err_c = NOSPACE)",t);
    */

    __gfxDrawPoint(0xc9,t,0,0);
    __gfxDrawPoint(0xbb,t,0,t->Xdim - 1);
    __gfxDrawPoint(0xc8,t,t->Ydim - 1, 0);
    __gfxDrawPoint(0xbc,t,t->Ydim - 1, t->Xdim -1);

    return err_c;
}


int gfxCleanLine(window t, int y, int x1, int x2)
{   return gfxDrawLine(' ',t,y,x1,y,x2);    }


int gfxCleanCol(window t, int x, int y1, int y2)
{   return gfxDrawLine(' ',t,y1,x,y2,x);    }

int gfxDrawStringTerm(window t, int y, int x, char *fmt,...)
{
    char line[t->Xdim];
    va_list arg;
    
    va_start(arg,fmt);
        vsprintf(line,fmt,arg);
    va_end(arg);

    int dim = strlen(line);
    int err_c;

    int i = 0;
    for (; i < dim && i < t->Xdim; ++i)
        _g_ferr(gfxDrawPoint(line[i],t,y,x + i),&err_c);
    
    t->Yws = y;
    t->Xws = x + i - 1;
    
    if (dim > t->Xdim - x)
        return _p_err(TRUNCREQUIRED,"gfxDrawStringTerm: truncation required (err_c = TRUNCREQUIRED)");
    
    return err_c;
}

int gfxDrawStringCont(window t, int y, int x, int sbx, int ebx, bool trunc, char *fmt,...)
{
    char line[t->Xdim * t->Ydim];

    va_list arg;
    va_start(arg,fmt);
        vsprintf(line,fmt,arg);
    va_end(arg);

    int dim = strlen(line);
    int err_c;

    for (int i = 0; i < dim; ++i)
    {
        if (x > ebx + 1)
        {
            ++y;
            x = sbx;
        }
        
        _g_ferr(gfxDrawPoint(line[i],t,y,x), &err_c);
        ++x;
    }

    t->Yws = y;
    t->Xws = x;

    return err_c;
}



int gfxSetLineThickness(window t, int nring)
{
    if (nring < 0)
        return _p_err(BADRADIUS,"gfxSetLineThickness: Invalid line thickness value (%d) on window: %p (err_c = BADRADIUS)",nring,t);
    
    t->line_t = nring;
    return 0;
}

int gfxDrawPoint(char point, window t, int y, int x)
{
    int err_c;
    for (register int yi = y - t->line_t; yi <= y + t->line_t; ++yi)
        for (register int xi = x - t->line_t; xi <= x + t->line_t; ++xi)
            _g_ferr(__gfxDrawPoint(point,t,yi,xi), &err_c);
    return err_c;
}

int gfxDrawLine(char point,window t,int corner1Y,int corner1X,int corner2Y,int corner2X)
{
    double dx =  fabs (corner2X - corner1X), sx = corner1X < corner2X ? 1 : -1;
    double dy = -fabs (corner2Y - corner1Y), sy = corner1Y < corner2Y ? 1 : -1; 
    double err = dx + dy, e2;
    int err_c;
 
    while(1)
    {
        _g_ferr(gfxDrawPoint(point,t,corner1Y,corner1X),&err_c);
        if (corner1X == corner2X && corner1Y == corner2Y) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; corner1X += sx; }
        if (e2 <= dx) { err += dx; corner1Y += sy; }
    }
    
    return err_c;
}

int cartDrawPoint(char point, window t, double y, double x, double Yscale, double Xscale)
{
    int err_c;
    for (double yi = y - t->line_t; yi <= y + t->line_t; yi += 1)
        for (double xi = x - t->line_t; xi <= x + t->line_t; xi += 1)
            _g_ferr(__cartDrawPoint(point,t,yi,xi,Yscale,Xscale), &err_c);

    return err_c;
}


int cartDrawLine(char point,window t,int corner1Y,int corner1X,int corner2Y,int corner2X)
{
    double dx =  fabs (corner2X - corner1X), sx = corner1X < corner2X ? 1 : -1;
    double dy = -fabs (corner2Y - corner1Y), sy = corner1Y < corner2Y ? 1 : -1; 
    double err = dx + dy, e2;
    int err_c;
 
    while(1)
    {
        _g_ferr(cartDrawPoint(point,t,corner1Y,corner1X,t->Yscale,t->Xscale),&err_c);
        if (corner1X == corner2X && corner1Y == corner2Y) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; corner1X += sx; }
        if (e2 <= dx) { err += dx; corner1Y += sy; }
    }
    
    return err_c;
}


int gfxDrawPointP(char point, window t, ddpoint p)
{   return gfxDrawPoint(point,t,p.y,p.x);   }



void cartSetOrigin(window t,int y, int x)
{
    // qualsiasi coordinata va bene
    t->ORy = y;
    t->ORx = x;
}


int gfxYdim(window t)
{   return t->Ydim; }

int gfxXdim(window t)
{   return t->Xdim; }


int gfxFillRect(char point,window t,int corner1Y,int corner1X,int corner2Y,int corner2X)
{

    if ((corner1Y < 0 && corner2Y < 0) || (corner1Y >= t->Ydim && corner2Y >= t->Ydim))
        return _p_err(BADY,"gfxFillRect: BAD Y of corner1 or corner2: %d %d (err_c = BADY)",corner1Y, corner2Y);

    if ((corner1X < 0 && corner2X < 0 )|| (corner1X >= t->Xdim && corner2X >= t->Xdim))
        return _p_err(BADX,"gfxFillRect: BAD X of corner1 or corner2: %d %d (err_c = BADX)",corner1X, corner2X);
    


    // trova upleft e upright

    int downrightY = MAX(corner1Y, corner2Y);
    int upleftY = downrightY == corner1Y ? corner2Y : corner1Y;
    
    int downrightX = MAX(corner1X, corner2X);
    int upleftX = downrightX == corner1X ? corner2X : corner1X;  
    
    for (int y = downrightY; y >= upleftY;--y)
        gfxDrawLine(point,t,y,upleftX,y,downrightX);     // non c'è bisogno della gestione degli errori dato che sono controllati all'inizio   

    return 0;
}


int cartDrawCirc(char point,window t, double Yc, double Xc, double r)
{
    int err_c = 0;
    double sa, ca;

    for (double a = 0.0f; a <= 3.1415926f / 2.0f; a += 0.01f)
    {
        sa = sin(a);
        ca = cos(a);

        _g_ferr(cartDrawPoint(point, t, Yc + r * sa, Xc + r * ca, t->Yscale, t->Xscale), &err_c);
        _g_ferr(cartDrawPoint(point, t, Yc - r * sa, Xc + r * ca, t->Yscale, t->Xscale), &err_c);
        _g_ferr(cartDrawPoint(point, t, Yc + r * sa, Xc - r * ca, t->Yscale, t->Xscale), &err_c);
        _g_ferr(cartDrawPoint(point, t, Yc - r * sa, Xc - r * ca, t->Yscale, t->Xscale), &err_c);
    }

    
    // in realtà calcola solo 1/4 di circonferenza

    return err_c;
}


int gfxDrawRect(const char point,window t,int corner1Y,int corner1X,int corner2Y,int corner2X)
{
    if ((corner1Y < 0 && corner2Y < 0) || (corner1Y >= t->Ydim && corner2Y >= t->Ydim))
        return _p_err(BADY,"gfxDrawRect: BAD Y of corner1 or corner2: %d %d (err_c = BADY)",corner1Y, corner2Y);

    if ((corner1X < 0 && corner2X < 0) || (corner1Y >= t->Ydim && corner2Y >= t->Ydim))
        return _p_err(BADX,"gfxDrawRect: BAD X of corner1 or corner2: %d %d (err_c = BADX)",corner1X, corner2X);

    gfxDrawLine(point,t,corner1Y,corner1X,corner1Y,corner2X);
    gfxDrawLine(point,t,corner1Y,corner2X,corner2Y,corner2X);
    gfxDrawLine(point,t,corner2Y,corner2X,corner2Y,corner1X);
    gfxDrawLine(point,t,corner2Y,corner1X,corner1Y,corner1X);    

    return 0;
}


// Header distribuito al pubblico
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <wchar.h>
#include <windows.h>

#define MAXWINDOWSATTACHED 10

typedef struct __win {
    int Ydim, Xdim;
    double ORy, ORx;
    int Ysign, Xsign;
    double Yscale, Xscale;
    int Yproj, Xproj;
    int line_t;
    int crr_col;
    CHAR_INFO *s;
    char *title;
    int *jmp;
    int frst;
    int ppos;
    int Yws, Xws;
    int fntY_c, fntX_c;
    struct __win *win[MAXWINDOWSATTACHED], *focus;
} *window; 


typedef struct ddpoint {
    double y;
    double x;
} ddpoint;


typedef struct f_letter {
    char char_c;    // carattere che rappresenta
    char **s;   // griglia contenente il carattere
} f_letter;

typedef struct font {
    char *name;  // nome del font
    f_letter letter[256];    // caratteri che lo compongono
    int Ydim;   // dimensione Y del carattere
    int Xdim;   // dimensione X del carattere
} font;


extern font __basfnt;
#define gfxstdfont &__basfnt


// Errorlevels
enum fcodes {
    POSY = 1,
    POSX = 1,
    NEGY = -1,
    NEGX = -1,
    // 0 rappresenta tutto coretto
    BADY = 1,
    BADX,
    CANTINITIALIZE,
    WINDOWSNOTATTACHED,
    WINDOWALRDATTACHED,
    SAMEWINDOW,
    BADRADIUS,
    BADYSIGN,
    BADXSIGN,
    NOSPACE,
    TRUNCREQUIRED,
    USINGNATIVE,
    CANTCLOSE,
    GRTR,
    DNTCR,
    MAXSIZE
};

enum COLOUR // from olc ma da rivedere in realtà
{
	FG_BLACK		= 0x0000,
	FG_DARK_BLUE    = 0x0001,	
	FG_DARK_GREEN   = 0x0002,
	FG_DARK_CYAN    = 0x0003,
	FG_DARK_RED     = 0x0004,
	FG_DARK_MAGENTA = 0x0005,
	FG_DARK_YELLOW  = 0x0006,
	FG_GREY			= 0x0007, 
   FG_DARK_GREY    = 0x0008,
	FG_BLUE			= 0x0009,
	FG_GREEN		= 0x000A,
	FG_CYAN			= 0x000B,
	FG_RED			= 0x000C,
	FG_MAGENTA		= 0x000D,
	FG_YELLOW		= 0x000E,
	FG_WHITE		= 0x000F,
	BG_BLACK		= 0x0000,
	BG_DARK_BLUE	= 0x0010,
	BG_DARK_GREEN	= 0x0020,
	BG_DARK_CYAN	= 0x0030,
	BG_DARK_RED		= 0x0040,
	BG_DARK_MAGENTA = 0x0050,
	BG_DARK_YELLOW	= 0x0060,
	BG_GREY			= 0x0070,
	BG_DARK_GREY	= 0x0080,
	BG_BLUE			= 0x0090,
	BG_GREEN		= 0x00A0,
	BG_CYAN			= 0x00B0,
	BG_RED			= 0x00C0,
	BG_MAGENTA		= 0x00D0,
	BG_YELLOW		= 0x00E0,
	BG_WHITE		= 0x00F0,
};

int gfxWindY(void);
int gfxWindX(void);


int gfxYdim(window t);
// DA AGGIUNGERE RETTE COL COEFF ANGOLARE E RETTE PARALLELE AGLI ASSI

int gfxXdim(window t);
void gfxCleanVideo(void);

int gfxGetColour(window t);

/* 
   @brief Dimensiona ed inizializza una nuova finestra t
   @param[in] t Finestra 
   @param[in] Ynew Dimensione Y della finestra
   @param[in] Xnew Dimensione X della finestra 
   @return Un puntatore ad una struttura window se la creazione ha avuto buon fine, oppure in caso di dimensioni non adeguate, 
   o se non c'è spazio da allocare per una nuova finestra, NULL
*/
window gfxNewWindowDimension(int Ynew,int Xnew);

float __fsqrt(float n);


/* 
   @brief Pulisce il buffer della finestra t
   @param[in] t Finestra 
*/
void gfxCleanTable(window t);

/* 
   @brief Aggiunge un piano cartesiano alla finestra t
   @param[in] t Finestra 
   @param[in] showYaxis Asse Y visibile
   @param[in] showXaxis Asse X visibile
   @param[in] ShowYscale Scala su Y visibile
   @param[in] ShowYscale Scala su X visibile
*/
void cartAddCartesian(window t,bool showYaxis,bool showXaxis,bool showYscale,bool showXscale);

/* 
   @brief Stampa su schermo il buffer della finestra t il più velocemente possibile (causa flickering)
   @param[in] t Finestra 
*/
void gfxShowFast(window t);


/* 
   @brief Stampa su schermo il buffer della finestra t
   @param[in] t Finestra 
*/
void gfxShow(window t);

/* 
   @brief Stampa su schermo tutte le finestre collegate a bcg, tenendo conto della sovrapposizione e dell'ordine
   @param[in] bcg Finestra di base  
   @return WINDOWSNOTATTACHED se non ci sono finestre collegate a bcg, altrimenti 0
*/
int gfxShowDesk(window bcg);

/* 
   @brief Scambia l'ordine di sovrapposizione di due finestre w1 e w2 sulla finestra di base desk
   @param[in] w1 Prima finestra
   @param[in] w2 Seconda finestra
   @param[in] desk Finestra di base
   @return SAMEWINDOW se w1 e w2 sono la stessa finestra, WINDOWSNOTATTACHED se w1 o w2 non sono collegate al desk, altrimenti 0
*/
int gfxSwapPrior(window w1,window w2, window desk);

/* 
   @brief Collega una finestra top ad un'altra bcg, e rende top il focus di bcg
   @param[in] top Finestra da collegare  
   @param[in] bcg Finestra di base  
   @return SAMEWINDOW se top e bcg sono la stessa finestra, 
   WINDOWSALRDATTACHED se top è già collegata a bcg, NOSPACE se il numero di finestre collegate a bcg >= MAXWINDOWSATTACHED, 
   altrimenti 0
*/
int gfxLink(window top,window bcg);

/* 
   @brief Scollega una finestra top ad un'altra bcg e aggiorna il focus all'finestra subito prima di top
   @param[in] top Finestra da scollegare  
   @param[in] bcg Finestra di base  
   @return SAMEWINDOW se top e bcg sono la stessa finestra, 
   WINDOWNOTATTACHED se top non è collegata a bcg, altrimenti 0
*/
int gfxDeLink(window top,window bcg);

/* 
   @brief Aggiorna il focus della finestra bcg, con newtop mantendendo l'ordine di precedenza
   @param[in] newtop Nuovo focus  
   @param[in] bcg Finestra di base  
   @return SAMEWINDOW se newtop e bcg sono la stessa finestra, 
   WINDOWNOTATTACHED se newtop non è collegata a bcg, altrimenti 0
*/
int gfxChangeFocus(window newtop, window bcg);


bool gfxTurnCursor(void);
int gfxMoveOn(window top,int dy,int dx,window bcg);
int gfxSetOn(window top,int dy,int dx,window bcg);
void __gfxPos(window top,int dy,int dx,window bcg, bool add);
int gfxWindBorder(window t,char *title);
int gfxDestroyWindow(window t);
int gfxCleanLine(window t, int y, int x1, int x2); // dipendente
int gfxCleanCol(window t, int x, int y1, int y2);  // dipendente

int gfxSetLineThickness(window t,int nring);
int gfxSetColour(window t,int c);

// Gestione primitive
int gfxDrawStringCont(window t, int y, int x, int sbx, int ebx, bool trunc, char *fmt,...);
int gfxDrawStringTerm(window t, int y, int x, char *fmt,...);
void cartSetOrigin(window t,int y, int x);


int gfxDrawPoint(char point, window t, int y, int x);
int __gfxDrawPoint(char point, window t, int y, int x);    // dipendente
int cartDrawPoint(char point, window t, double y, double x, double Yscale, double Xscale);
int __cartDrawPoint(char point, window t, double y, double x, double Yscale, double Xscale);   // dipendente

int gfxDrawPointP(char point, window t, ddpoint p);
int cartDrawPointP(char point, window t,ddpoint p, double Yscale, double Xscale);


int gfxFillRect(char point,window t,int corner1Y,int corner1X,int corner2Y,int corner2X);
int cartFillRect(char point,window t,double corner1Y,double corner1X,double corner2Y,double corner2X);

int gfxFillRectP(char point,window t,ddpoint p1, ddpoint p2);
int cartFillRectP(char point,window t,ddpoint p1, ddpoint p2);


int gfxDrawLine(char point,window t,int corner1Y,int corner1X,int corner2Y,int corner2X);
int cartDrawLine(char point,window t,int corner1Y,int corner1X,int corner2Y,int corner2X);

int gfxDrawLineP(char point,window t,ddpoint p1, ddpoint p2);
int cartDrawLineP(char point,window t,ddpoint p1, ddpoint p2);


int gfxDrawRect(char point,window t,int corner1Y,int corner1X,int corner2Y,int corner2X);
int cartDrawRect(char point,window t,double corner1Y,double corner1X,double corner2Y,double corner2X);

int gfxDrawRectP(char point,window t,ddpoint p1, ddpoint p2);
int cartDrawRectP(char point,window t,ddpoint p1, ddpoint p2);


int gfxDrawCirc(char point,window t, int Yc, int Xc, int r);
int cartDrawCirc(char point,window t, double Yc, double Xc, double r);

int gfxDrawCircP(char point,window t, ddpoint c, int r);
int cartDrawCircP(char point,window t, ddpoint c, double r);

int gfxFillCirc(char point,window t, int Yc, int Xc, int r);
int cartFillCirc(char point,window t, double Yc, double Xc, double r);

int gfxFillCircP(char point,window t, ddpoint c, int r);
int cartFillCircP(char point,window t, ddpoint c, double r);


int gfxInit(int Ydim, int Xdim, int Fy, int Fx, int MODE);
int gfxTerminate(void);


/* 
   @brief Carica il font specificato in path e ne restituisce il puntatore
   @param[in] path Percorso al file del font
   @return Un puntatore al font, oppure NULL se risulta impossibile aprirlo 
*/
font *gfxLoadFont(char *path);

/* 
   @brief Scrive a schermo sulla finestra t con il font fnt alle coordinate y x e dilatazione k
   @param[in] t Finestra
   @param[in] fnt Puntatore ad un font aperto dal programma
   @param[in] y Coordinata y del punto di inizio della stringa
   @param[in] X Coordinata x del punto di inizio della stringa
   @param[in] k Dilatazione (omotetia) di ogni carattere
   @param[in] fmt Stringa
   @return 
*/
int gfxFontWrite(window t,font *fnt,int y, int x, int k, char *fmt, ...);

char cartReadPoint(window t, double y, double x, double Yscale, double Xscale);
char gfxReadPoint(window t, int y, int x);


int _p_err(int err_c, char *fmt,...);
void _g_ferr(int new_c, int *err_c);




#include "basgfx6.h"
#include <conio.h>
#include <sys/time.h>

#define G 6.67430e-11
#define t_unit 1e-7
#define rdo2 sqrt(2.0f) / 2.0f 


typedef struct dddpoint {
    double y;
    double x;
} dddpoint;

typedef struct vector {
    double y,x;
} vector;

typedef struct obj {
    dddpoint pos;
    vector v;
    double mass;
    int colour;
} obj;


// KNOWN BUGS OR ISSUES::

// -When the simulation is paused the real time does not flow because the program remains in the while loop
// -On my virtual machine the switch "/?" is recognized as "/C" 
// -Collisions are not handled and when they happen bodys splash away
// -I positioned each planet of the default simulation in its aphelion so the effect is not real
// -Trying to observe objects with a small tail using a big scale may result in the tail being concentrated in a point and becoming invisible
// -Objects on the axes x and y are not painted
// -With hig values of t_unit the simulation goes crazy (maybe because the system is chaotic)



void chkArgs(int argc, char *argv[], int i);
obj *parseFile(FILE *f, int *nc);


int main(int argc, char *argv[])
{
    int GFXdims[4] = {600, 600, 1, 1};
    int GFXswitch = GRTR;
    int trace = 100; 
    float t_un = t_unit;
    FILE *fsource = NULL;
    float scale = 1;


    if (argc != 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (strcmp(argv[i],"/?") == 0 || strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"--help") == 0)
            {
                printf("ncorps: - Jmdp10\n\n"
                       "Usage: ncorps [/?] [-h] [--help]\n"
                       "  /? , -h , --help                                          Prints this help and exits\n"
                       "  /D [YDIM] [XDIM] [PY] [PX] , -D [YDIM] [XDIM] [PY] [PX]   Specifies the window size and the resolutions\n"
                       "  /M [PY] [PX] , -M [PY] [PX]                               Sets the maximum window size according to the resolution\n"
                       "  /S [X] , -S [X]                                           Specifies the number of saved positions for each corp\n"
                       "                                                            (default is 100)\n"
                       "  /T [X] , -T [X]                                           Specifies delta t (default is 1e-7)\n"
                       "  /R [X] , -R [X]                                           Sets the rappresentation scale (default is 1)\n"
                       "  /F [X] , -F [X]                                           Imports corps settings from file [X]\n"
                       "                                                            The search for the file starts from the specified source\n"
                       "                                                            and continues in the same directory of the executable.\n\n"
                       " FILE SYNTAX - file must be a .txt and it must contain this scheme:\n\n"
                       " \"Yposition_of_corp Xposition_of_corp Yvelocity_of_corp Xvelocity_of_corp Mass_of_corp Colour(optional) \"\n"
                       " NOTE: datas must be in scientific notation and in the IS units of measurement.\n"
                       " Each line must end with a space.\n"
                       " Color range from 0 (black) to 16 (white).\n"
                       " For example: \"1e10f 2e3f 0.0f 10.0f 1e22 5 \"             sets an object at position 1e10f 2e3f, with speed 0.0f 10.0f,\n"
                       "                                                          mass 1e22 and color 5.\n"
                       " Comments are made by placing a '#' at the beginning of a line.\n\n"
                       " If no file is specified, the program will simulate the solar system.\n"
                       "\nErrorlevels:\n"
                       "   0                    Everything is fine\n"
                       "   1                    Runtime error (file not found or problems with memory allocations)\n"
                       "   2                    Syntax error on input\n");

                exit(0);
            }
            else if (strcmp(argv[i],"/D") == 0 || strcmp(argv[i],"-D") == 0)
            {
                for (int j = 0; j < 4; j++)
                {
                    chkArgs(argc, argv, i);

                    ++i;
                    GFXdims[j] = atoi(argv[i]);

                    if (GFXdims[j] <= 0)
                    {
                        printf("ncorps: Invalid argument \" %d \"", GFXdims[j]);
                        exit(2);
                    }
                } 
            }
            else if (strcmp(argv[i],"/M") == 0 || strcmp(argv[i],"-M") == 0)
            {
                GFXswitch = MAXSIZE;
                for (int j = 0; j < 2; j++)
                {
                    chkArgs(argc, argv, i);

                    ++i;
                    GFXdims[j + 2] = atoi(argv[i]);

                    if (GFXdims[j + 2] <= 0)
                    {
                        printf("ncorps: Invalid argument \" %d \"", GFXdims[j + 2]);
                        exit(2);
                    }
                } 
            }
            else if (strcmp(argv[i],"/T") == 0 || strcmp(argv[i],"-T") == 0)
            {
                chkArgs(argc, argv, i);

                ++i;

                sscanf(argv[i],"%e",&t_un);

                if (t_un <= 0.0f)
                {
                    printf("ncorps: Invalid argument \" %e \"", t_un);
                    exit(2);
                }   

            }
            else if (strcmp(argv[i],"/S") == 0 || strcmp(argv[i],"-S") == 0)
            {
                chkArgs(argc, argv, i);

                ++i;
                trace = atoi(argv[i]);
                
                if(trace < 0)
                {
                    printf("ncorps: Invalid argument \" %d \"", trace);
                    exit(2);
                }
            }
            else if (strcmp(argv[i],"/R") == 0 || strcmp(argv[i],"-R") == 0)
            {
                chkArgs(argc, argv, i);

                ++i;

                sscanf(argv[i],"%e", &(scale));
                
                if(scale < 0)
                {
                    printf("ncorps: Invalid argument \" %f \"", scale);
                    exit(2);
                }
            }
            else if (strcmp(argv[i],"/F") == 0 || strcmp(argv[i],"-F") == 0)
            {
                chkArgs(argc, argv, i);

                ++i;
                
                // Opens the file

                
                if ((fsource = fopen(argv[i],"r")) == NULL)
                {
                    char *path = (char *) malloc(sizeof(char) * MAX_PATH);
                    GetModuleFileName(NULL,path,MAX_PATH);

                    int a = 0;
                    for (a = strlen(path); path[a] != '\\';--a)    ;
    
                    path[a + 1] = '\0';

                    char *fpath = strcat(path, argv[i]);

                    if ((fsource = fopen(fpath, "r")) == NULL)
                    {
                        printf("ncorps: Cant find file \" %s \" neither in the specified path, nor in the current directory\n",argv[i]);
                        exit(1);
                    }
                }
                
                if(trace < 0)
                {
                    printf("ncorps: Invalid argument \" %d \"", trace);
                    exit(2);
                }
            }
            else
            {
                printf("ncorps: Unknown or unexpected argument  \" %s \", see the HELP (/?)\n",argv[i]);
                exit(2);
            }
        }
    }



    if (gfxInit(GFXdims[0],GFXdims[1],GFXdims[2],GFXdims[3],GFXswitch))
    {
        printf("ncorps: Failed graphical initialization\n");
        exit(1);
    }

    window back = gfxNewWindowDimension(gfxWindY(), gfxWindX());
    window full = gfxNewWindowDimension(gfxWindY(), gfxWindX());
    window swap = back;


    obj *corpses;

    int cnum;
    

    if (fsource != NULL)
    {
        corpses = parseFile(fsource, &cnum);
    }
    else
    {
        cnum = 10;
        obj demo[10] = {
            {
                {0.0f, 0.0f},
                {0.0f, 0.0f},
                1.981e30,
                FOREGROUND_RED  // Sun
            },

            {
                {0.0f, 152097701e3},
                {29291, 0.0f},
                5.972e24,
                FOREGROUND_GREEN    // Earth
            },

            {
                {0.0f, 249228730e3},
                {21972, 0.0f},
                6.418e23,
                FOREGROUND_BLUE     // Mars
            },

            {
                {0.0f, 1.0894e11},
                {34790, 0.0f},
                4.867e24,
                FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY   // Venus
            },

            {
                {0.0f, 6.982e10},
                {38860, 0.0f},
                3.301e23,
                FOREGROUND_BLUE | FOREGROUND_RED    // Mercury
            }, 
            {
                {0.0f, 816081455e3},
                {12446, 0.0f},
                1.89819e27,
                FOREGROUND_GREEN | FOREGROUND_RED   // June
            },
            
            {
                {0.0f, 1515500000e3},
                {9.09e3, 0.0f},
                5.6834e26,
                FOREGROUND_BLUE | FOREGROUND_GREEN  // Saturn
            },
            
            {
                {0.0f, 4536874325e3},
                {5385, 0.0f},
                1.024e26,
                FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED // Neptune
            },
            
            {
                {0.0f, 3003.62e9},
                {6.49e3, 0.0f},
                86.813e24,
                FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY // Uranus
            },
            
            {
                {0.0f, 7375930000e3},
                {3676, 0.0f},
                1.303e22,
                FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY   // Pluto
            }
        };

        corpses = demo;
    }
    
    back->Yscale = back->Xscale = full->Yscale = full->Xscale = scale;

    dddpoint dist[cnum][cnum];
    double diq[cnum][cnum];
    double dst[cnum][cnum];
    vector as[cnum][cnum];
    vector ar[cnum];
    double t = 0.0f; 

    dddpoint *hst[cnum];
    int it = 0;

    bool showLines = true;


    for (int i = 0; i < cnum; ++i)
    {
        if ((hst[i] = (dddpoint *) malloc(sizeof(dddpoint) * trace)) == NULL)
        {
            printf("ncorps: Parameter trace too big (not enought space for allocation: %d) \n", trace);
            exit(1);
        }
    }

    struct timeval stop, start;

    gettimeofday(&start, NULL);


    while (1)
    {
        // Cleans resulting accelerations
        for (int i = 0; i < cnum; ++i)
        {
            for (int a = 0; a < cnum; ++a)
                if (a != i)
                {
                    ar[i].y = 0;
                    ar[i].x = 0;
                }
        }

        // Calculates distances and accelerations individually
        for (int i = 0; i < cnum; ++i)
        {
            for (int a = 0; a < cnum; ++a)
                if (a != i)
                {
                    dist[i][a].y = corpses[i].pos.y - corpses[a].pos.y;
                    dist[i][a].x = corpses[i].pos.x - corpses[a].pos.x; 

                    diq[i][a] = dist[i][a].y * dist[i][a].y + dist[i][a].x * dist[i][a].x;

                    dst[i][a] = sqrt(diq[i][a]);

                    as[i][a].y = -dist[i][a].y * dst[i][a] / diq[i][a] * G * corpses[a].mass / diq[i][a];
                    as[i][a].x = -dist[i][a].x * dst[i][a] / diq[i][a] * G * corpses[a].mass / diq[i][a];

                }
        }

        // Calculates the resultants of the accelerations

        for (int i = 0; i < cnum; ++i)
        {
            for (int a = 0; a < cnum; ++a)
                if (a != i)
                {
                    ar[i].y += as[i][a].y;
                    ar[i].x += as[i][a].x;
                }
        }
        

        // Calculate hourly and print laws
        for (int i = 0; i < cnum; ++i)
        {
            corpses[i].v.y += ar[i].y * t_un;
            corpses[i].v.x += ar[i].x * t_un;

            corpses[i].pos.y += corpses[i].v.y * t_un + 0.5f * ar[i].y * t_un * t_un;
            corpses[i].pos.x += corpses[i].v.x * t_un + 0.5f * ar[i].x * t_un * t_un;


            if ((int)(hst[i][trace-1].y * 1.0f / back->Yscale) != (int)(corpses[i].pos.y * 1.0f / back->Yscale) || 
                (int)(hst[i][trace-1].x * 1.0f / back->Xscale) != (int)(corpses[i].pos.x * 1.0f / back->Xscale))
            {

                if (kbhit())
                {
                    int c = getch();

                    switch (c)
                    {
                        case '+':
                            full->Yscale = full->Xscale = back->Yscale = back->Xscale /= 1.5f;
                            gfxCleanTable(back);
                            gfxCleanTable(full);
                            gfxCleanVideo();
                            break;

                        case '-':
                            full->Yscale = full->Xscale = back->Yscale = back->Xscale *= 1.5f;
                            gfxCleanTable(back);
                            gfxCleanTable(full);
                            gfxCleanVideo();
                            break;
                        
                        case 'd':
                            full->ORx = back->ORx -= 50 * back->Yscale;
                            gfxCleanTable(back);
                            gfxCleanTable(full);
                            gfxCleanVideo();
                            break;
                        
                        case 'a':
                            full->ORx = back->ORx += 50 * back->Xscale;
                            gfxCleanTable(back);
                            gfxCleanTable(full);
                            gfxCleanVideo();
                            break;
                        
                        case 'w':
                            full->ORy = back->ORy += 50 * back->Xscale;
                            gfxCleanTable(back);
                            gfxCleanTable(full);
                            gfxCleanVideo();
                            break;
                        
                        case 's':
                            full->ORy = back->ORy -= 50 * back->Xscale;
                            gfxCleanTable(back);
                            gfxCleanTable(full);
                            gfxCleanVideo();
                            break;
                        
                        case 'l':   // I've implemented a very slow font writing system, so it can be deactivated
                            if (showLines)
                            {
                                showLines = false;
                                gfxCleanTable(back);
                                gfxCleanTable(full);
                                gfxCleanVideo();
                            }
                            else
                                showLines = true;
                            break;
                        
                        case 'q':
                            exit(0);

                        case '1':
                            swap = back;
                            break;
                        
                        case '2':
                            swap = full;
                            break;
                        
                        case ' ':
                            while (1)
                            {   
                                if (kbhit())
                                    c = getch();
                                
                                if (c == ' ')
                                    break;
                                else if (c == 'q')
                                    exit(0);
                            }
                            break;
                    }
                }

                gfxSetColour(back,corpses[i].colour);
                    cartDrawPoint(219,back,corpses[i].pos.y, corpses[i].pos.x, 1.0f / back->Yscale, 1.0f / back->Xscale);

                gfxSetColour(full,corpses[i].colour);
                    cartDrawPoint(219,full,corpses[i].pos.y, corpses[i].pos.x, 1.0f / back->Yscale, 1.0f / back->Xscale);
                    
                // crates the tail
                if (it < trace)
                {
                    hst[i][it] = corpses[i].pos;
                    if (i == cnum - 1)
                        ++it;
                }
                else
                {
                    gfxSetColour(back,0);
                        cartDrawPoint(219, back, hst[i][0].y, hst[i][0].x, 1.0f / back->Yscale, 1.0f / back->Xscale);

                    for (int a = 1; a < trace; a++)
                        hst[i][a - 1] = hst[i][a];

                    hst[i][trace - 1] = corpses[i].pos;
                }                
            }
        }

        gfxShowFast(swap);
        t += t_un;
        
        
        if (showLines)
        {
            gfxSetColour(swap, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
            gfxFontWrite(swap, gfxstdfont, 5, 5, 2, "scale: 1 px = %.1e m\n",swap->Xscale);
            gfxFontWrite(swap, gfxstdfont, swap->fntY_c, 5, 2, "delta t: %.2e s\n",t_un);
            gfxFontWrite(swap, gfxstdfont, swap->fntY_c, 5, 2, "current t: %.2e s (%.1e years)\n",t, t / (60.0f * 60.0f * 24.0f * 30.0f * 12.0f));
            gfxFontWrite(swap, gfxstdfont, swap->fntY_c, 5, 2, "                  ",t);
        }
        
        gettimeofday(&stop, NULL);

        if (showLines)
        {
            gfxFontWrite(swap, gfxstdfont, swap->fntY_c, 5, 2, "irl t: %.2e s\n",(stop.tv_sec - start.tv_sec) + (float)(stop.tv_usec - start.tv_usec) / 1000000);
            gfxFontWrite(swap, gfxstdfont, swap->fntY_c, 5, 2, "wasd to move, +- to zoom\n");
            gfxFontWrite(swap, gfxstdfont, swap->fntY_c, 5, 2, "q to quit, space to pause\n");
            gfxFontWrite(swap, gfxstdfont, swap->fntY_c, 5, 2, "1 for tail, 2 for trace\n");
        }
    }

    exit(0);

}



void chkArgs(int argc, char *argv[], int i)
{
    if (i + 1 == argc)
    {
        printf("ncorps: Expected argument after token \" %s \", see the HELP \n",argv[i]);
        exit(2);
    }  
}


obj *parseFile(FILE *f, int *nc)
{
    char lbuf[100];
    char wbuf[100];
    int nobj = 0;
    float py, px, vy, vx, m;

    int st, end;

    obj *ob = (obj*) malloc(sizeof(obj));

    while (fgets(lbuf, 100, f) != NULL)
    {
        if (lbuf[0] != '#')
        {
            st = end = 0;
            if (realloc(ob, sizeof(obj) * (nobj + 1)) == NULL)
            {
                printf("ncorps: Can't allocate enought memory for the objects\n");
                exit(1);
            }
               // It surely isn't elegant and its dangerous but works (i struggled parsing from the file)

            end = (int) (strchr(lbuf + st, ' ') - lbuf);
            strncpy(wbuf,lbuf + st, end - st);
            sscanf(wbuf, "%e",&py);

            memset(wbuf, 0, sizeof(char) * 100);

            st = end + 1;

            end = (int) (strchr(lbuf + st, ' ') - lbuf);
            strncpy(wbuf,lbuf + st, end - st);
            sscanf(wbuf, "%e",&px);

            memset(wbuf, 0, sizeof(char) * 100);

            st = end + 1;

            end = (int) (strchr(lbuf + st, ' ') - lbuf);
            strncpy(wbuf,lbuf + st, end - st);
            sscanf(wbuf, "%e",&vy);

            memset(wbuf, 0, sizeof(char) * 100);

            st = end + 1;

            end = (int) (strchr(lbuf + st, ' ') - lbuf);
            strncpy(wbuf,lbuf + st, end - st);
            sscanf(wbuf, "%e",&vx);

            memset(wbuf, 0, sizeof(char) * 100);

            st = end + 1;

            end = (int) (strchr(lbuf + st, ' ') - lbuf);
            strncpy(wbuf,lbuf + st, end - st);
            sscanf(wbuf, "%e",&m);

            memset(wbuf, 0, sizeof(char) * 100);

            st = end + 1;

            char *clp;
            if ((clp = strchr(lbuf + st, ' ')) != NULL)
            {
                end = (int) (strchr(lbuf + st, ' ') - lbuf);
                strncpy(wbuf,lbuf + st, end - st);
                sscanf(wbuf, "%d",&ob[nobj].colour);

                memset(wbuf, 0, sizeof(char) * 100);

            }
            else
                ob[nobj].colour = FOREGROUND_RED;

            ob[nobj].pos.y = py;
            ob[nobj].pos.x = px;
            ob[nobj].v.y = vy;
            ob[nobj].v.x = vx;
            ob[nobj].mass = m;  

            ++nobj;
        }

    }

    *nc = nobj;

    fclose(f);
    return ob;

}




/*
    
    obj corpses[8] = {

        {
            {0.0f, 100.0f},
            {200.0f, 0.0f},
            2.0e17,
            rand() % 32
        },

        {
            {100.0f * rdo2, 100.0f * rdo2},
            {200.0f * rdo2, -200.0f * rdo2},
            2.0e17,
            rand() % 32
        },

        {
            {100.0f, 0.0f},
            {0.0f, -200.0f},
            2.0e17,
            rand() % 32
        },

        {
            {100.0f * rdo2, -100.0f * rdo2},
            {-200.0f * rdo2, -200.0f * rdo2},
            2.0e17,
            rand() % 32
        },

        {
            {0.0f, -100.0f},
            {-200.0f, 0.0f},
            2.0e17,
            rand() % 32
        },
        
        {
            {-100.0f * rdo2, -100.0f * rdo2},
            {-200.0f * rdo2, 200.0f * rdo2},
            2.0e17,
            rand() % 32
        },

        {
            {-100.0f, 0.0f },
            {0.0f, 200.0f},
            2.0e17,
            rand() % 32
        },

        {
            {-100.0f * rdo2, 100.0f * rdo2},
            {200.0f * rdo2, 200.0f * rdo2},
            2.0e17,
            rand() % 32
        }
    };*/




    
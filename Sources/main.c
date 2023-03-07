#include <windows.h>
#include <gl/gl.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <glut.h>
#include <stdbool.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"
#include "../Headers/camera_engine.h"
#include "../Headers/box_editor.h"

int WindowWidth, WindowHeight;
int timer = 0;
HWND hwnd;
BOOL SHOW_FONT = TRUE;
BOOL INDICATORLOAD = FALSE;
#define pW 10
#define pH 10
#define StrCnt 1
#define IS_COLOR_SHOW FALSE //shows the current color which cursor is located
BOOL IS_HOLD = FALSE;
struct
{
    float x, y, z;
    BOOL ACTIVE;
} enemy[StrCnt];

struct
{
    int x,y;
} MousePos;

POINT scrSize;
float scrKoef;

/*
struct
{
    float width, depth, height, lx, ly, px,py;
} basicSize = {1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0};
*/
//float route[4][5][3] = {};
float routeIND[4][7] = {}; //contain a route number according a color code and route length
int kubevertnum[8][1];
float colorInd[] = {1,1};
float kubeMap[] = {0,0,0, 0,1,0, 1,1,0, 1,0,0, 0,0,1, 0,1,1, 1,1,1, 1,0,1};
float kube[] = {-1,-1,-1, -1,1,-1, 1,1,-1, 1,-1,-1, -1,-1,-1, 1,-1,-1, 1,-1,1, 1,1,1, 1,1,-1, 1,1,1, -1,1,1, -1,1,-1, -1,1,1, -1,-1,1, 1,-1,1, -1,-1,1};
GLuint kubeInd[] = {0,1,2, 2,3,0, 4,5,6, 6,7,4, 3,2,5, 6,7,3, 0,1,5, 5,4,0, 1,2,6, 6,5,1, 0,3,7, 7,4,0};
//float kubevert[] = {-1,-1,-1, -1,1,-1, 1,1,-1, 1,-1,-1, 1,-1,1, 1,1,1, -1,1,1, -1,-1,1};
BOOL ShowMask = FALSE;
static float rectCoord[] = {0,0, 1,0, 1,1, 0,1};
static float rectTex[] = {0,1, 1,1, 1,0, 0,0};

int tex_textID;
float *charWidthArray;

void SetCharSize(unsigned char *data, int width, int cnt,
                 float **cWidthArray, int checkByte)
{
    int pixPerChar = width / 16;
    for(int k = 0; k<256; k++)
    {
        int x = (k%16)*pixPerChar;
        int y = (k/16)*pixPerChar;
        int i;
        //int ind;
        unsigned char alpha;
        for(i = x + pixPerChar - 1; i>x; i--)
        {
            for(int j = y + pixPerChar - 1; j>y; j--)
            {
                alpha = data[(j*width+i)*cnt+checkByte];
                if(alpha>0) break;
            }
            if(alpha>0) break;
        }

        i+=pixPerChar/10.0;
        if(i>x+pixPerChar-1) i =x + pixPerChar - 1;
        if(k==32) i = (x+pixPerChar/2.0);
        (*cWidthArray)[k] = (i-x)/(float)pixPerChar;
    }
}

void LoadTexture(char *file_name, int *target, float **cWidthArray, int checkByte)
{
    int width, height, cnt;
    unsigned char *data = stbi_load(file_name, &width, &height, &cnt, 0);
    if(cWidthArray != NULL)
        SetCharSize(data, width, cnt, cWidthArray, checkByte);
    glGenTextures(1, target);
    glBindTexture(GL_TEXTURE_2D, *target);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                 0, cnt == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}


void TexShow(float ox, float oy, char *text, float *cWidthArray)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_textID);
    glPushMatrix();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, rectCoord);
    glTexCoordPointer(2, GL_FLOAT, 0, rectTex);

    GLint viewport[4];
    GLdouble mvm[16];
    GLdouble projm[16];

    GLdouble wx, wy, wz;
    GLfloat zval;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, mvm);
    glGetDoublev(GL_PROJECTION_MATRIX, projm);

    glReadPixels(ox, oy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zval);
    gluUnProject(ox, oy, zval, mvm, projm, viewport, &wx, &wy, &wz);

    glTranslatef(wx+0.01,-wy+0.035,1);
    glScalef(0.08, 0.08*scrKoef, 1);

    while(*text)
    {
        static float charSize = 1/16.0;
        unsigned char c = *text;
        float cWidth = cWidthArray[c];
        int y = c>>4;
        int x = c & 0b1111;
        struct
        {
            float left, right, top, bottom;
        } rct;
        rct.left = x*charSize;
        rct.right = rct.left + charSize * cWidth;
        rct.top = y * charSize;
        rct.bottom = rct.top + charSize;
        rectTex[0] = rectTex[6] = rct.left;
        rectTex[2] = rectTex[4] = rct.right;
        rectTex[1] = rectTex[3] = rct.bottom;
        rectTex[5] = rectTex[7] = rct.top;
        rectCoord[2] = rectCoord[4] = cWidth;
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        text++;
        glTranslatef(cWidth,0,0);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
}
/*
void init_route();
void init_route()
{
    for(int i = 0; i<4; i++)
        route[i][0][0]=basicSize.lx;
    for(int i = 0; i<4; i++)
        route[i][0][1]=basicSize.ly;
    for(int i = 0; i<4; i++)
        route[i][0][2]=kubevert[14];

    for(int i = 0; i<2; i++)
        route[i][1][0] = kubevert[12+i*6];
    for(int i = 2; i<4; i++)
        route[i][1][0] = basicSize.lx;
    for(int i = 0; i<2; i++)
        route[i][1][1]= basicSize.ly;
    route[2][1][1]= kubevert[16];
    route[3][1][1]= kubevert[13];
    for(int i = 0; i<4; i++)
        route[i][1][2]=kubevert[14];

    for(int i = 0; i<2; i++)
        route[i][2][0] = kubevert[12+i*6];
    for(int i = 2; i<4; i++)
        route[i][2][0] = basicSize.lx;
    for(int i = 0; i<2; i++)
        route[i][2][1]= basicSize.ly;
    route[2][2][1]= kubevert[16];
    route[3][2][1]= kubevert[13];
    for(int i = 0; i<4; i++)
        route[i][2][2]=kubevert[11];

    for(int i = 0; i<2; i++)
        route[i][3][0]=basicSize.px;
    for(int i = 2; i<4; i++)
        route[i][3][0] = basicSize.lx;
    for(int i = 0; i<2; i++)
        route[i][3][1]=basicSize.ly;
    for(int i = 2; i<4; i++)
        route[i][3][1] = basicSize.py;
    for(int i = 0; i<4; i++)
        route[i][3][2]=kubevert[11];

    for(int i = 0; i<4; i++)
        route[i][4][0] = basicSize.px;
    for(int i = 0; i<4; i++)
        route[i][4][1] = basicSize.py;
    for(int i = 0; i<4; i++)
        route[i][4][2]=kubevert[11];
}
*/
typedef struct
{
    float r, g, b;
} TColor;

typedef struct
{
    TColor clr;
} TCell;

TCell map[pW][pH];

void Map_Init()
{
    for(int i = 0; i<pW; i++)
        for(int j = 0; j<pH; j++)
        {
            if((i%2)==0)
            {
                if((j%2)==0)
                {
                    map[i][j].clr.r = 0;
                    map[i][j].clr.g = 0.5;
                    map[i][j].clr.b = 0;
                }
                else
                {
                    map[i][j].clr.r = 1;
                    map[i][j].clr.g = 1;
                    map[i][j].clr.b = 1;
                }
            }
            else
            {
                if((j%2)==0)
                {
                    map[i][j].clr.r = 1;
                    map[i][j].clr.g = 1;
                    map[i][j].clr.b = 1;
                }
                else
                {
                    map[i][j].clr.r = 0;
                    map[i][j].clr.g = 0.5;
                    map[i][j].clr.b = 0;
                }
            }
        }
}

void WndResize(int x, int y);
void Move_engine()
{
    if(!SHOW_FONT)
    player_Move();
}
void PROG_Init()
{
    charWidthArray = malloc(sizeof(*charWidthArray)*256);
    for(int i = 0; i<256; i++)
        charWidthArray[i] = 1;
    LoadTexture("Font_B_alpha.png", &tex_textID, &charWidthArray, 0);
    glBindTexture(GL_TEXTURE_2D, tex_textID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.99);
    glEnable(GL_DEPTH_TEST);
    Map_Init();
    Enemy_Init();

    RECT rct;
    GetClientRect(hwnd, &rct);
    WndResize(rct.right, rct.bottom);
}

void Enemy_Init()
{
    for(int i = 0; i<StrCnt; i++)
    {
        enemy[i].ACTIVE = TRUE;
        enemy[i].x = 0;
        enemy[i].y = 0;
        enemy[i].z = 2;
    }
}
float theta = 0.0f;

extern bool PointlnVERTEX;
extern bool PointlnPoint;

int routeST[4][2][1];

typedef struct
{
    float x;
    float y;
} Vector2f;

//
//  Draws rounded rectangle.
//
//  Slightly tuned version of http://stackoverflow.com/questions/5369507/opengles-1-0-2d-rounded-rectangle
//
#define ROUNDING_POINT_COUNT 8      // Larger values makes circle smoother.
void DrawRoundRect(float x, float y, float width, float height, float* color, float radius)
{
    Vector2f top_left[ROUNDING_POINT_COUNT];
    Vector2f bottom_left[ROUNDING_POINT_COUNT];
    Vector2f top_right[ROUNDING_POINT_COUNT];
    Vector2f bottom_right[ROUNDING_POINT_COUNT];

    if( radius == 0.0 )
    {
        radius = min(width, height);
        radius *= 0.10; // 10%
    }

    int i = 0;
    float x_offset, y_offset;
    float step = ( 2.0f * M_PI ) / (ROUNDING_POINT_COUNT * 4),
          angle = 0.0f;

    unsigned int index = 0, segment_count = ROUNDING_POINT_COUNT;
    Vector2f bottom_left_corner = { x + radius, y - height + radius };


    while( i != segment_count )
    {
        x_offset = cosf( angle );
        y_offset = sinf( angle );


        top_left[ index ].x = bottom_left_corner.x -
                              ( x_offset * radius );
        top_left[ index ].y = ( height - ( radius * 2.0f ) ) +
                              bottom_left_corner.y -
                              ( y_offset * radius );


        top_right[ index ].x = ( width - ( radius * 2.0f ) ) +
                               bottom_left_corner.x +
                               ( x_offset * radius );
        top_right[ index ].y = ( height - ( radius * 2.0f ) ) +
                               bottom_left_corner.y -
                               ( y_offset * radius );


        bottom_right[ index ].x = ( width - ( radius * 2.0f ) ) +
                                  bottom_left_corner.x +
                                  ( x_offset * radius );
        bottom_right[ index ].y = bottom_left_corner.y +
                                  ( y_offset * radius );


        bottom_left[ index ].x = bottom_left_corner.x -
                                 ( x_offset * radius );
        bottom_left[ index ].y = bottom_left_corner.y +
                                 ( y_offset * radius );


        top_left[ index ].x = top_left[ index ].x;
        top_left[ index ].y = top_left[ index ].y;


        top_right[ index ].x = top_right[ index ].x;
        top_right[ index ].y = top_right[ index ].y;


        bottom_right[ index ].x = bottom_right[ index ].x ;
        bottom_right[ index ].y = bottom_right[ index ].y;


        bottom_left[ index ].x =  bottom_left[ index ].x ;
        bottom_left[ index ].y =  bottom_left[ index ].y ;

        angle -= step;

        ++index;

        ++i;
    }

    //static GLubyte clr[] = { 156, 207, 255, 128 };   // Light blue, 50% transparent.

    if( color == 1 )
        glColor3ub(190,190,190);
    if(color == 2)
        glColor3ub(255,255,255);
    if(color == 3)
        glColor3ub(255,255,102);
    if(color == 4)
        glColor3ub(102,255,102);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, scrSize.x, scrSize.y, 0.0f, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glBegin( GL_TRIANGLE_STRIP );
    {

        // Top
        for( i = segment_count - 1 ; i >= 0 ; i--)
        {
            glVertex2f(top_left[ i ].x, top_left[ i ].y);
            glVertex2f(top_right[ i ].x, top_right[ i ].y);
        }

        // In order to stop and restart the strip.
        glVertex2f(top_right[ 0 ].x, top_right[ 0 ].y);
        glVertex2f(top_right[ 0 ].x, top_right[ 0 ].y);

        // Center
        glVertex2f( top_right[ 0 ].x, top_right[ 0 ].y);
        glVertex2f( top_left[ 0 ].x, top_left[ 0 ].y);
        glVertex2f( bottom_right[ 0 ].x, bottom_right[ 0 ].y);
        glVertex2f( bottom_left[ 0 ].x, bottom_left[ 0 ].y);

        // Bottom
        for( i = 0; i != segment_count ; i++ )
        {
            glVertex2f( bottom_right[ i ].x, bottom_right[ i ].y);
            glVertex2f( bottom_left[ i ].x, bottom_left[ i ].y);
        }
    }
    glEnd();
} //DrawRoundRect
float min_length = 0;
void SizeHint();
int timerSH = 0;
int iterFR = 0;
int moverSH = -40;
void SizeHint()
{
    RECT rct;
    GetClientRect(hwnd, &rct);
    int Width = 75;
    int Height = 35;
    GLubyte clr[3];
    int height = rct.bottom - rct.top;
    //int width = rct.right - rct.left;
    glReadPixels(MousePos.x,MousePos.y, 1,1, GL_RGB, GL_UNSIGNED_BYTE, &clr);
    POINT o;
    GLdouble model_view[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    GLdouble pos3D_x, pos3D_y, pos3D_z;
    GLdouble winx, winy, winz;

        if(INDICATORLOAD)
        {
            if(moverSH<12)
            moverSH+=4;
        if(moverSH<15)
            moverSH+=0.2;
        int colormatch;
        if(timer>300)
        {
            colormatch = 2;
            moverSH = -40;
        }
        else
            colormatch = 4;
        if(PointlnVERTEX||PointlnPoint)
            colormatch = 3;
        DrawRoundRect(rct.right/2.0-100,rct.bottom-moverSH,200,Height,1,10);
        DrawRoundRect(rct.right/2.0+2-100,rct.bottom-2-moverSH,200-4,Height-4,colormatch,8);
        }

    for(int i = 0; i<4; i++)
    {
        routeIND[i][6] = (min_length - routeIND[i][5]);
        if(routeIND[i][0]==clr[0]&&routeIND[i][1]==clr[1]&&routeIND[i][2]==clr[2])
        {
            iterFR = routeIND[i][3];
        }
    }
    double a = (route[iterFR][0][0]+route[iterFR][1][0])/2;
    double b = (route[iterFR][0][1]+route[iterFR][1][1])/2;
    double c = (route[iterFR][0][2]+route[iterFR][1][2])/2;
    int res = gluProject(a,b,c, model_view, projection, viewport, &winx, &winy, &winz);
    winy=height-winy;
    o.x = winx;
    o.y = winy;
    int ret;
    char buffer[6];
    if(routeIND[iterFR][4]==1)
    {
        ret = snprintf(buffer, sizeof buffer, "%1.1f",
                       (routeIND[iterFR][5] + basicSize.height));
        Width = 95;
    }
    else
    {
        ret = snprintf(buffer, sizeof buffer, "%1.2f",
                       routeIND[iterFR][6]);
        Width = 175;
        if(routeIND[iterFR][6]==0.00)
            Width = 135;
    }
    if(ret<0)
    {
        buffer[0] = 0;
    }

    if((clr[0]==102&&clr[1]==255&&clr[2]==102)||(clr[0]==192&&clr[1]==192&&clr[2]==192)||(clr[0]==192&&clr[1]==192&&clr[2]==191)||(clr[0]==192&&clr[1]==192&&clr[2]==190)||(clr[0]==192&&clr[1]==192&&clr[2]==189)||(clr[0]==192&&clr[1]==192&&clr[2]==188))
    {
        timerSH = 250;
    }
    if(timerSH>0)
    {
        DrawRoundRect(o.x,o.y-10,Width,Height,1,10);
        DrawRoundRect(o.x+2,o.y-2-10,Width-4,Height-4,2,8);
        if(routeIND[iterFR][4]==1)
            glColor3ub(51, 255, 51);
        else
            glColor3ub(255,51,51);
        float ox = o.x;
        float oy = o.y;
        TexShow(ox,oy,buffer, charWidthArray);
        timerSH--;
    }
}

void Enemy_Show()
{
    timer++;
    glPushAttrib(GL_ENABLE_BIT);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, kube);
    for(int i = 0; i<StrCnt; i++)
    {
        if(!enemy[i].ACTIVE) continue;
        glPushMatrix();
        glRotatef(theta, 0.0f, 0.0f, 0.1f);
        glTranslatef(enemy[i].x, enemy[i].y, enemy[i].z);
        glScalef(basicSize.width,basicSize.depth,basicSize.height);
        glColor3ub(244, 60, 43);

        if(ShowMask)
        {
            for(int j = 0; j<22; j+=3)
            {
                glPointSize(30);
                glBegin(GL_POINTS);
                glColor3ub(0,0,255-(j/3));
                glVertex3f(kubevert[j],kubevert[j+1],kubevert[j+2]);
                kubevertnum[j/3][0]=(255-(j/3));
                glEnd();
            }
        }
        glVertexPointer(3, GL_FLOAT, 0, kube);
        glLineWidth(5);
        glColor3ub(244, 60, 43);
        if(PointlnVERTEX)
            glColor3ub(214, 60, 43);
        glDrawArrays(GL_LINE_LOOP, 0, 16);
        glPointSize(15);
        glBegin(GL_POINTS);
        glColor3ub(255,153,0); //for mask color 250 153 0
        if(ShowMask) glColor3ub(250,153,0);
        glVertex3f(basicSize.lx, basicSize.ly, kubevert[14]);
        glEnd();

        glPointSize(15);
        glBegin(GL_POINTS);
        glColor3ub(255,153,0);
        if(ShowMask) glColor3ub(255,150,0); //for mask color 255 150 0
        glVertex3f(basicSize.px, basicSize.py, kubevert[2]);
        glEnd();

        init_route();

        int min_length_num = 0;
        float buf;
        BOOL IS_FIRST = TRUE;
        float a, b, c;
        for(int i = 0; i<4; i++)
        {
            buf = 0;
            for(int j = 0; j<4; j++)
            {
                a = (route[i][j+1][0] - route[i][j][0]);
                b = (route[i][j+1][1] - route[i][j][1]);
                c = (route[i][j+1][2] - route[i][j][2]);
                buf += sqrt((a*a)+(b*b)+(c*c));
            }
            if((i==2)||(i==3))
                buf+=basicSize.depth;
            else
                buf+=basicSize.width;
            routeIND[i][5] = buf;
            if(buf<=min_length||IS_FIRST)
            {
                min_length=buf;
                min_length_num=i;
                IS_FIRST = FALSE;
                routeIND[i][4] = 1;
                for(int j = 0; j<i; j++)
                {
                    routeIND[j][4] = 0;
                }
            }
            else
                routeIND[i][4] = 0;

        }

        for(int i = 0; i<4; i++)
        {
            if(i==min_length_num)
            {
                glLineStipple(1, 0xFFFF);
                glEnable(GL_LINE_STIPPLE);
                glLineWidth(10);
                glBegin(GL_LINES);
                glColor3ub(102, 255, 102);
                routeIND[i][0] = 102;
                routeIND[i][1] = 255;
                routeIND[i][2] = 102;
                routeIND[i][3] = i;
            }
            else if(timer<300)
            {
                glLineStipple(2, 0x00FF);
                glEnable(GL_LINE_STIPPLE);
                glLineWidth(8);
                glBegin(GL_LINES);
                glColor3ub(192,192,192-i);
                routeIND[i][0] = 192;
                routeIND[i][1] = 192;
                routeIND[i][2] = 192-i;
                routeIND[i][3] = i;
            }
            for(int j = 0; j<4; j++)
            {
                glVertex3f(route[i][j][0], route[i][j][1], route[i][j][2]);
                glVertex3f(route[i][j+1][0], route[i][j+1][1], route[i][j+1][2]);
            }
            glEnd();
        }

        glDisable(GL_LINE_STIPPLE);
        if(!PointlnVERTEX&&!PointlnPoint)
            theta += 0.5f;
        else if(PointlnVERTEX)
        {
            theta += 0.01;
            resize_proc();
        }
        else if(PointlnPoint)
        {
            theta = 0;
            resize_proc();
        }
        SizeHint();
        glPopMatrix();
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopAttrib();
}

typedef struct
{
    char name[20];
    float vert[8];
    BOOL hover;
} TButton;

TButton btn[] =
{
    {"start", {0,0, 100,0, 100,30, 0,30}, FALSE},
    {"stop", {0,40, 100,40, 100,70, 0,70}, FALSE},
    {"quit", {0,80, 100,80, 100,110, 0,110}, FALSE}
};

int btnCnt = sizeof(btn)/sizeof(btn[0]);

void WndResize(int x, int y)
{
    glViewport(0,0, x,y);
    scrSize.x = x;
    scrSize.y = y;
    scrKoef = x/(float)y;
    float k = x/(float)y;

}

void TButton_Show(TButton btn)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    if(btn.hover) glColor3f(0,0.7,0);
    else glColor3f(0,1,0);
    glVertexPointer(2, GL_FLOAT, 0, btn.vert);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

BOOL PointlnButton(int x, int y, TButton btn)
{
    return (x>btn.vert[0]) && (x<btn.vert[4]) && (y>btn.vert[1]) && (y<btn.vert[5]);
}

float vert[] = {1,1,0, 1,-1,0, -1,-1,0, -1,1,0};
float xAlfa = 20;
float zAlfa = 0;
POINTFLOAT pos = {0,0};

void showMenu()
{
    glPushMatrix();
    glScalef(0.3, 0.3, 1);
    glOrtho(0,WindowWidth, WindowHeight, 0, -1,1);
    for(int i =0; i<btnCnt; i++)
        TButton_Show(btn[i]);
    glPopMatrix();
}

void showWorld()
{

    float sz = 0.1;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-scrKoef*sz,scrKoef*sz,-sz,sz,sz*2,100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.6, 0.8, 1, 0);
    if(SHOW_FONT)
        glClearColor(1,1,1,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    Camera_Apply();
    glEnableClientState(GL_VERTEX_ARRAY);


    glVertexPointer(3, GL_FLOAT, 0, kubeMap);
    for(int i = -pW+1; i<pW; i++)
        for(int j = -pH+1; j<pH; j++)
        {
            glPushMatrix();
            glTranslatef(i, j, 0);
            glColor3f(map[abs(i)][abs(j)].clr.r, map[abs(i)][abs(j)].clr.g, map[abs(i)][abs(j)].clr.b);
            if(SHOW_FONT)
                glColor3f(255,255,255);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, kubeInd);
            glPopMatrix();
        }
    glDisableClientState(GL_VERTEX_ARRAY);
    Enemy_Show();
    glPopMatrix();
}

typedef struct
{
    int clr[3];
} Color_pip;
int r_m, g_m, b_m;
int VertNum;
int PointNum;
//int offset_x, offset_y;
void Moving(int x, int y)
{
    ShowMask = TRUE;
    showWorld();
    ShowMask = FALSE;
    RECT rct;
    GLubyte clr[3];
    GetClientRect(hwnd, &rct);
    int height = rct.bottom - rct.top;
    glReadPixels(x,height-y, 1,1, GL_RGB, GL_UNSIGNED_BYTE, &clr);
    r_m = clr[0];
    g_m = clr[1];
    b_m = clr[2];
    //printf("%d %d\n", x, y);
    if((clr[0]==0)&&(clr[1]==0)&&(clr[2]>=248))
    {
        PointlnVERTEX = true;
        VertNum = abs(clr[2]-255);
    }
    if((clr[0]==250)&&(clr[1]==153)&&(clr[2]==0))
    {
        PointlnPoint = true;
        PointNum = 14;
    }
    if((clr[0]==255)&&(clr[1]==150)&&(clr[2]==0))
    {
        PointlnPoint = true;
        PointNum = 2;
    }
}
//int itercnt = 0;
/*
void resize_proc()
{
    timer = 0;
    int a,b,c;
    RECT rct;
    GetClientRect(hwnd, &rct);
    POINT cur;
    GetCursorPos(&cur);
    POINT o;
    int height = rct.bottom - rct.top;
    GLdouble model_view[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    GLdouble pos3D_x, pos3D_y, pos3D_z;
    GLdouble winx, winy, winz;
    GLdouble win2x, win2y, win2z;
    {
        if(PointlnVERTEX)
        {
            int i = VertNum;
            a = kubevert[(i*3)];
            b = kubevert[(i*3)+1];
            c = kubevert[(i*3)+2];
        }
        if(PointlnPoint)
        {
            ShowCursor(FALSE);
            int i = PointNum;
            if(i==14)
            {
                a = basicSize.lx;
                b = basicSize.ly;
                c = kubevert[14];
            }
            if(i==2)
            {
                a = basicSize.px;
                b = basicSize.py;
                c = kubevert[2];
            }
        }
    }
    int res = gluProject(a,b,c, model_view, projection, viewport, &winx, &winy, &winz);
    winy=height-winy;
    o.x = winx;
    o.y = winy;
    ClientToScreen(hwnd, &o);
    SetCursorPos(o.x, o.y);
    float z;
    float zdif;
    glReadPixels(winx,winy, 1,1, GL_DEPTH_COMPONENT,GL_FLOAT, &z);
    int res_dif = gluProject(-a,-b,c, model_view, projection, viewport, &win2x, &win2y, &win2z);
    win2y=height-win2y;
    glReadPixels(win2x,win2y, 1,1, GL_DEPTH_COMPONENT,GL_FLOAT, &zdif);
    if(itercnt==2)
        ScreenToClient(hwnd, &cur);
    if(itercnt==2)
    {
        offset_x = (cur.x-winx);
        offset_y = (winy-cur.y);
        itercnt = 0;
        if(cur.x<rct.right&&cur.y<rct.right)

            if(PointlnVERTEX)
            {
                {
                    if(abs(offset_y)>1) basicSize.height+=((float)offset_y/1000);
                    if(cur.x>rct.right/2.0)
                    {
                        if(z<zdif)
                        {
                            if(abs(offset_x)>1) basicSize.width+=((float)offset_x/1000);
                        }
                        if(z>zdif)
                        {
                            if(abs(offset_x)>1) basicSize.depth+=((float)offset_x/1000);
                        }
                    }
                    if(cur.x<rct.right/2.0)
                    {
                        if(z>zdif)
                        {
                            if(abs(offset_x)>1) basicSize.width-=((float)offset_x/1000);
                        }
                        if(z<zdif)
                        {
                            if(abs(offset_x)>1) basicSize.depth-=((float)offset_x/1000);
                        }
                    }
                }
            }

        if(PointlnPoint&&PointNum==14)
        {
            if(basicSize.ly<-1.0) basicSize.ly = -1.0;
            if(basicSize.ly>1.0) basicSize.ly = 1.0;
            if(abs(offset_x)>1) basicSize.ly+=((float)offset_x/1000);
            if(basicSize.lx<-1.0) basicSize.lx = -1.0;
            if(basicSize.lx>1.0) basicSize.lx = 1.0;
            if(abs(offset_y)>1) basicSize.lx-=((float)offset_y/1000);
        }
        if(PointlnPoint&&PointNum==2)
        {
            if(basicSize.py<-1.0) basicSize.py = -1.0;
            if(basicSize.py>1.0) basicSize.py = 1.0;
            if(abs(offset_x)>1) basicSize.py+=((float)offset_x/1000);
            if(basicSize.px<-1.0) basicSize.px = -1.0;
            if(basicSize.px>1.0) basicSize.px = 1.0;
            if(abs(offset_y)>1) basicSize.px-=((float)offset_y/1000);
        }

    }
    else
    {
        offset_x = cur.x;
        offset_y = cur.y;
    }
    itercnt+=1;
}
*/

void Color_ind()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glPushMatrix();
    glOrtho(0,WindowWidth, WindowHeight, 0, -1,1);
    glColor3ub(r_m,g_m,b_m);
    glPointSize(100);
    glVertexPointer(2, GL_FLOAT, 0, colorInd);
    glDrawArrays(GL_POINTS, 0, 1);
    glPopMatrix();
    glFlush();
    glDisableClientState(GL_VERTEX_ARRAY);
}
BOOL SHOW_FPS = FALSE;
void CalculateFrameRate()
{
    static float framesPerSecond    = 0.0f;       // This will store our fps
    static float lastTime   = 0.0f;       // This will hold the time from the last frame
    float currentTime = GetTickCount() * 0.001f;
    ++framesPerSecond;
    if( currentTime - lastTime > 1.0f )
    {
        lastTime = currentTime;
        if(SHOW_FPS == 1) printf("\nCurrent Frames Per Second: %f\n", (float)framesPerSecond);
        char buff[20];
        strcpy(buff, "OpenGL Sample ");
        buff[15] = (char)framesPerSecond;
        LPCSTR lp = buff;
        SetWindowTextA(hwnd, lp);
        framesPerSecond = 0;
    }
}

void player_Move()
{
    if(GetKeyState(VK_UP)<0) camera.Xrot = ++camera.Xrot > 180 ? 180 : camera.Xrot;
    if(GetKeyState(VK_DOWN)<0) camera.Xrot = --camera.Xrot < 0 ? 0 : camera.Xrot;
    if(GetKeyState(VK_LEFT)<0) camera.Zrot++;
    if(GetKeyState(VK_RIGHT)<0) camera.Zrot--;
    if(GetKeyState(VK_SHIFT)<0) camera.z-=0.1;
    if(GetKeyState(VK_SPACE)<0) camera.z+=0.1;

    float ugol = -camera.Zrot/180 * M_PI;
    float speed = 0;
    if(GetKeyState('W')<0) speed =0.1;
    if(GetKeyState('S')<0) speed = -0.1;
    if(GetKeyState('A')<0)
    {
        speed = 0.1;
        ugol -= M_PI*0.5;
    }
    if(GetKeyState('D')<0)
    {
        speed = 0.1;
        ugol += M_PI*0.5;
    }
    if(speed!=0)
    {
        camera.x += sin(ugol)* speed;
        camera.y += cos(ugol) * speed;
    }
    /*
        POINT cur;
    static POINT base = {400,300};
    GetCursorPos(&cur);
    Camera_Rotation((base.y - cur.y)/5.0, (base.x - cur.x)/5.0);
    SetCursorPos(base.x, base.y);
    */
}

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;

    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          850,
                          700,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);
    SetWindowPos(hwnd, HWND_TOP, 100, 20, 850, 700, SWP_NOSIZE); //modifier!!!

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    PROG_Init();


    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {

            /* OpenGL animation code goes here */
            Move_engine();
            showWorld();

            if(!IS_HOLD)
            ShowCursor(TRUE);

            //CalculateFrameRate();
            //showMenu();
            if(IS_COLOR_SHOW)
                Color_ind();
            SwapBuffers(hDC);
            Sleep (1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        WndResize(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_LBUTTONUP:
        PointlnVERTEX = false;
        PointlnPoint = false;
        IS_HOLD = FALSE;
        if(PointlnPoint)
            printf("%s\n", "up");
        break;

    case WM_LBUTTONDOWN:
        IS_HOLD = TRUE;
        Moving(LOWORD(lParam), HIWORD(lParam));
        for(int i = 0; i<btnCnt; i++)
            if(PointlnButton(LOWORD(lParam), HIWORD(lParam), btn[i]))
            {
                printf("%s\n", btn[i].name);

                if(strcmp(btn[i].name, "start") == 0)
                {
                    if(SHOW_FONT)
                        SHOW_FONT = FALSE;
                    else
                        SHOW_FONT = TRUE;
                    //printf("%1.1f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n", basicSize.width, basicSize.depth, basicSize.height, basicSize.lx, basicSize.ly, basicSize.px, basicSize.py);
                }

                if(strcmp(btn[i].name, "stop") == 0)
                {
                    if(INDICATORLOAD)
                        INDICATORLOAD = FALSE;
                    else
                     INDICATORLOAD = TRUE;
                    //printf("x = %f y = %f xrot = %f zrot = %f\n", camera.x, camera.y, camera.Xrot, camera.Zrot);
                }

                //if(strcmp(btn[i].name, "quit") == 0)
                    //PostQuitMessage(0);
            }
        break;


    case WM_MOUSEMOVE:
        for(int i = 0; i<btnCnt; i++)
            btn[i].hover = PointlnButton(LOWORD(lParam), HIWORD(lParam), btn[i]);
        MousePos.x = LOWORD(lParam);
        MousePos.y = HIWORD(lParam);
        break;

    case WM_DESTROY:
        return 0;

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
    }
    break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}


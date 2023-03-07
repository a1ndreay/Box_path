#include "../Headers/box_editor.h"
#include <glut.h>
#include <stdbool.h>

HWND hwnd;
float kubevert[] = {-1,-1,-1, -1,1,-1, 1,1,-1, 1,-1,-1, 1,-1,1, 1,1,1, -1,1,1, -1,-1,1};
struct Sbox basicSize = {1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0};
itercnt = 0;
timer = 0;
extern bool PointlnVERTEX = false;
extern bool PointlnPoint = false;

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




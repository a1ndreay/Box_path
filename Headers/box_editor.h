#ifndef BOX_EDITOR_H_INCLUDED
#define BOX_EDITOR_H_INCLUDED


struct Sbox
{
    float width, depth, height, lx, ly, px,py;
} basicSize;

float kubevert[];
int timer;
float route[4][5][3];
int itercnt;
void init_route();
void resize_proc();
int VertNum;
int PointNum;
int offset_x, offset_y;



#endif // BOX_EDITOR_H_INCLUDED

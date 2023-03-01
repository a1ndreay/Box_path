#include <GL/gl.h>
#include "../Headers/camera_engine.h"

struct SCamera camera = {4.56,-0.185,3.7, 70, 90};

void Camera_Apply()
{
    glRotatef(-camera.Xrot, 1,0,0);
    glRotatef(-camera.Zrot, 0,0,1);
    glTranslatef(-camera.x, -camera.y, -camera.z);
}

void Camera_Rotation(float xAngle, float zAngle)
{
    camera.Zrot += zAngle;
    if(camera.Zrot<0) camera.Zrot+=360;
    if(camera.Zrot>360) camera.Zrot-=360;
    camera.Xrot += xAngle;
    if(camera.Xrot<0) camera.Xrot=0;
    if(camera.Xrot>180) camera.Xrot=180;
}

#ifndef CAMERA_ENGINE_H_INCLUDED
#define CAMERA_ENGINE_H_INCLUDED

struct SCamera{
    float x,y,z;
    float Xrot,Zrot;
} camera;

void Camera_Apply();
void Camera_Rotation(float xAngle, float zAngle);


#endif // CAMERA_ENGINE_H_INCLUDED

//matrix operations
#pragma once
#include "vectors.h"
#define PI 3.141592653589

Mat4 Translate(Vec3 trans);
Mat4 Scale(Vec3 scale);
Mat4 Rotate(float theta, Vec3 rot);

Mat4 LookAt(Vec3 eye, Vec3 center, Vec3 up);
Mat4 Perspective(float fovy, float aspect, float zNear, float zFar);
Mat4 Orthographic(float left, float right, float bottom, float top, float zNear, float zFar);

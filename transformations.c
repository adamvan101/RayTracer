#include "transformations.h"
int testing = 1;

Mat4 Translate(Vec3 trans){
  Mat4 t;
  t.matrix[0] = 1.0;
  t.matrix[5] = 1.0;
  t.matrix[10] = 1.0;
  t.matrix[15] = 1.0;
  t.matrix[12] = trans.v[0];
  t.matrix[13] = trans.v[1];
  t.matrix[14] = trans.v[2];
  return t;
}

Mat4 Scale(Vec3 scale){
  Mat4 s;
  s.matrix[0] = scale.v[0];
  s.matrix[5] = scale.v[1];
  s.matrix[10] = scale.v[2];
  s.matrix[15] = 1.0;
  return s;
}

Mat4 Rotate(float theta, Vec3 rot){
  float x,y,z;
  Vec3 rotNorm = rot.normal();
  x = rotNorm.v[0];
  y = rotNorm.v[1];
  z = rotNorm.v[2];
  Mat4 r;

  float c = cos(theta*PI/180.0); //convert degree to radians
  float s = sin(theta*PI/180.0); 
  r.matrix[0] = c+x*x*(1-c);
  r.matrix[1] = y*x*(1-c)+z*s;
  r.matrix[2] = z*x*(1-c)-y*s;

  r.matrix[4] = x*y*(1-c)-z*s;
  r.matrix[5] = y*y*(1-c)+c;
  r.matrix[6] = y*z*(1-c)+x*s;

  r.matrix[8] = x*z*(1-c)+y*s;
  r.matrix[9] = y*z*(1-c)-x*s;
  r.matrix[10] = z*z*(1-c)+c;

  r.matrix[15] = 1.0;
  return r;
}


Mat4 LookAt(Vec3 eye, Vec3 center, Vec3 up){
  float m[16] = {0};
  Vec3 f, Up, s, u;
  f = (center-eye).normal();
  Up = up.normal();
  s = f.cross(Up);
  u = s.normal().cross(f);
  m[0] = s.v[0];
  m[4] = s.v[1];
  m[8] = s.v[2];
  m[1] = u.v[0];
  m[5] = u.v[1];
  m[9] = u.v[2];
  m[2] = -1*f.v[0];
  m[6] = -1*f.v[1];
  m[10] =-1*f.v[2];
  m[15] = 1;

  Mat4 M =  Mat4(m);
  Vec3 zero; //hacky, should just implement scalar multiplicaton
  Mat4 T = Translate(zero-eye);
  return M*T;
}


Mat4 Perspective(float fovy, float aspect, float zNear, float zFar){
  float m[16] = {0};
  float f = 1/tan(fovy * PI / 360);
  m[0] = f/aspect;
  m[5] = f;
  m[10] = (zFar+zNear)/(zNear-zFar);
  m[11] = -1;
  m[14] = 2*zFar*zNear/(zNear-zFar);
  return Mat4(m);
}

Mat4 Orthographic(float left, float right, float bottom, float top, float zNear, float zFar){
  float m[16] = {0};
  m[0] = 2/(right-left);
  m[5] = 2/(top-bottom);
  m[10] = -2/(zFar-zNear);
  m[12] = -(right+left)/(right-left);
  m[13] = -(top+bottom);
  m[14] = -(zFar+zNear)/(zFar-zNear);
  m[15] = 1;
  return Mat4(m);
}

void TransTest(){
  Mat4 A = Rotate(30, Vec3(1, 0, 0));
  Mat4 B = Rotate(60, Vec3(1, 0, 0));
  Mat4 C = Rotate(90, Vec3(1, 0, 0));
  A *= B;
  printf("AB = ");
  A.print();
  printf("should equal...\n");
  printf("C = ");
  C.print();
}

// //get rid of this when integrating into full program
// int main(){
//   if(testing){
//     VecTest();
//     TransTest();
//   }
// }

#include "vectors.h"

Vec3::Vec3(){
  v[0] = 0;
  v[1] = 0;
  v[2] = 0;
}

Vec3::Vec3(float x, float y, float z){
  v[0] = x;
  v[1] = y;
  v[2] = z;
}

Vec3 Vec3::scale(float f){
  float s1, s2, s3;
  s1 = v[0]*f;
  s2 = v[1]*f;
  s3 = v[2]*f;
  return Vec3(s1, s2, s3);
}

float Vec3::dist(Vec3 other)
{
  return sqrt(pow((v[0]-other[0]), 2) + pow((v[1]-other[1]), 2) + pow((v[2]-other[2]), 2));
}

float &Vec3::operator[](int i)
{
    if (i < 3)
      return v[i];
    return v[0];
}

void Vec3::print(){
  printf("[");
  int i;
  for (i=0;i<3;i++){ printf("%.2f, ", v[i]);
  }
  printf("]\n");
}

Vec3 Vec3::cross(Vec3 rhs){
  float s1, s2, s3;
  s1 = v[1]*rhs.v[2]-v[2]*rhs.v[1];
  s2 = v[2]*rhs.v[0]-v[0]*rhs.v[2];
  s3 = v[0]*rhs.v[1]-v[1]*rhs.v[0];
  Vec3 result(s1, s2, s3);
  return result;
}

float Vec3::dot(Vec3 rhs){
  return v[0]*rhs.v[0]+v[1]*rhs.v[1]+v[2]*rhs.v[2];
}

float Vec3::mag(){
  return sqrt(this->dot(*this));
}

Vec3 Vec3::normal(){
  Vec3 result;
  float mag = this->mag();
  result.v[0] = v[0]/mag;
  result.v[1] = v[1]/mag;
  result.v[2] = v[2]/mag;
  return result;
}

Vec3 Vec3::operator*(const float f) const{
  return Vec3(v[0]*f, v[1]*f, v[2]*f);
}

Vec3 Vec3::operator+(const Vec3& rhs) const{
  Vec3 result(v[0]+rhs.v[0],v[1]+rhs.v[1],v[2]+rhs.v[2]);
  return result;
}

Vec3 Vec3::operator-(const Vec3& rhs) const{
  Vec3 result(v[0]-rhs.v[0],v[1]-rhs.v[1],v[2]-rhs.v[2]);
  return result;
}

Vec4::Vec4(){
  v[0] = 0;
  v[1] = 0;
  v[2] = 0;
  v[3] = 0;
}

Vec4::Vec4(float x, float y, float z, float w){
  v[0] = x;
  v[1] = y;
  v[2] = z;
  v[3] = w;
}

void Vec4::print(){
  printf("[");
  int i;
  for (i=0;i<4;i++){ printf("%.2f, ", v[i]);
  }
  printf("]\n");
}

Vec4& Vec4::operator*=(const Mat4& rhs){
  //NOTE: v *= A returns Av, so A is left multiplied
  int i,k;
  float temp[4] = {0};
  for (i=0;i<4;i++){
    for (k=0;k<4;k++){
      temp[i] += v[k]*rhs.matrix[i+4*k];
    }
  }
  for(i=0;i<4;i++){
    v[i] = temp[i];
  }
  return *this;
}

float &Vec4::operator[](int i)
{
    if (i < 4)
      return v[i];
    return v[0];
}

Mat4::Mat4(){
  int i;
  for (i=0;i<16;i++){
    matrix[i] = 0;
  }
}

Mat4::Mat4(float m[16]){
  int i;
  for (i=0;i<16;i++){
    matrix[i] = m[i];
  }
}
void Mat4::setVal(int i, float x){
  matrix[i] = x;
}

void Mat4::print(){
  int i,j;
  printf("[");
  for (i=0;i<4;i++){
    printf("[");
    for (j=0;j<4;j++){
      printf("%.2f, ", matrix[i+4*j]);
    }
    if (i!=4){
      printf("]\n");
    }
    else if(i == 4){
      printf("]]\n");
    }
  }
}

Mat4 Mat4::operator*(const Mat4& rhs) const{
  Mat4 prod;  
  int i,j,k;
  for (i=0;i<4;i++){
    for (j=0;j<4;j++){
      for (k=0;k<4;k++){
        prod.matrix[i+4*j] += matrix[i+4*k]*rhs.matrix[4*j+k];
      }
    }
  }
  return prod;
}

Mat4& Mat4::operator*=(const Mat4& rhs) {
  float prod[16] = {0};  
  int i,j,k;
  for (i=0;i<4;i++){
    for (j=0;j<4;j++){
      for (k=0;k<4;k++){
        prod[i+4*j] += matrix[i+4*k]*rhs.matrix[4*j+k];
      }
    }
  }
  for (i=0;i<15;i++){
    matrix[i] = prod[i];
  }
  return *this;
}

Vec4 Mat4::operator*(const Vec4& rhs) const{
  int i,k;
  float temp[4] = {0};
  for (i=0;i<4;i++){
    for (k=0;k<4;k++){
      temp[i] += rhs.v[k]*matrix[i+4*k];
    }
  }
  return Vec4(temp[0], temp[1], temp[2], temp[3]);
}

Mat4 identity(){
  Mat4 id;
  id.matrix[0] = 1;
  id.matrix[5] = 1;
  id.matrix[10] = 1;
  id.matrix[15] = 1;
  return id;
}

//Unit Tests
void VecTest(){
    //vector 3 tests
    Vec3 a(1,1,0);
    Vec3 b(0,1,1);
    Vec3 c;
    c = a.cross(b);
    printf("a x b = ");
    c.print();
    printf("a has magnitude: %f\n", c.mag());
    c = a+b;
    printf("a + b = ");
    c.print();
    printf("c normalized is ");
    c.normal().print();
    
    //Matrix vector tests
    Vec4 x(1,2,3,4);
    Vec4 y;
    printf("x = ");
    x.print();
    Mat4 A,B,C;
    A.setVal(0,1);
    A.setVal(1,3);
    A.setVal(4,2);
    A.setVal(5,4);
    y = A*x;
    printf("(*) Ax = ");
    y.print();
    x *= A;
    printf("(*=) Ax = ");
    x.print();

    //Matrix Matrix tests
    B.setVal(0,1);
    B.setVal(1,3);
    B.setVal(4,2);
    B.setVal(5,4);
    printf("A = ");
    A.print();
    printf("B = ");
    B.print();
    printf("(*) AB = ");
    C = A*B;
    C.print();
    printf("(*=) AB = ");
    A *= B;
    A.print();
}

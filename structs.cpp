#include "structs.h"

// Mesh::Mesh(){
// }

//           position     at        up
Camera::Camera(Vec3 e, Vec3 c, Vec3 up)
{
    origin = e;
    z = (c-e).normal().scale(-1);
    x = up.cross(z).normal();
    y = z.cross(x);
}

Ray::Ray(Vec3 ori, Vec3 dir){
    origin = ori;
    direction = dir;
}

Light::Light(Vec3 pos, Vec3 inten, Vec3 atten){
    position = pos;
    intensity = inten;
    attenuation = atten;
}

Light::Light(){}

Pigment::Pigment(int t, Vec3 col1, Vec3 col2, float s){
    type = t;
    color = col1;
    color2 = col2;
    scale = s;
}

Vec3 Pigment::GetColor(Vec3 point)
{
    if (type == SOLID)
        return color;
    else
    {
        if ((int)(floor(point[0] / scale) + floor(point[1] / scale) + floor(point[2] / scale))%2 == 0)
        {
            return color;
        }
        else
        {
            return color2;
        }
    }
}

Pigment::Pigment(){}

Finish::Finish(float am, float dif, float spec, float shin, float refl, float trans, float ior){
    ambiance = am;
    diffuse = dif;
    specular = spec;
    shininess = shin;
    reflect = refl;
    transmission = trans;
    refraction = ior;
}

Finish::Finish(){}

Sphere::Sphere(){}

Sphere::Sphere(Vec3 c, float r)
{
    center = c;
    radius = r;
}

bool Object::Collide(Ray r, float &t)
{
    if (type == SPHERE)
    {
        float a = r.direction.dot(r.direction);
        float b = 2 * (r.direction.dot((r.origin-sphere.center)));
        float c = (r.origin-sphere.center).dot(r.origin-sphere.center) - (sphere.radius * sphere.radius);

        // printf("Center: ");
        // sphere.center.print();
        // printf ("Radius: %f, OR: ", sphere.radius);
        // (r.origin-sphere.center).print();
        // printf("DIR: ");
        // r.direction.print();
        // printf("ORI: ");
        // r.origin.print();

        float disc = b*b - (4*a*c);
        // printf("A: %f, B: %f, C: %f\n", a, b, c);
        // printf("Disc: %f\n", disc);
        // r.direction.print();
        // r.origin.print();
        if (disc < 0)
        {
            return false;
        }

        float disc_sqrt = sqrt(disc);
        float t1 = (-b+disc_sqrt)/(2*a); 
        float t2 = (-b-disc_sqrt)/(2*a);   
        if(t2>0.05f)
        {
            t = t2;
            return true;
        }
        else if (t1>=0.05f)
        {
            t = t1;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

Vec3 Object::normal(Vec3 point)
{
    if (type == SPHERE)
    {
        return (point - sphere.center).normal();
    }
    else
    {
        return Vec3(1, 1, 1);
    }
}

Object::Object(Sphere s, int col, int f, int n){
    sphere = s;
    color = col;
    finish = f;
    numTrans = n;
    type = SPHERE;
}

Object::Object(){}
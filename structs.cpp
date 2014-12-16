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
        // point.print();
        if ((int)(floor(point[0] / scale) + floor(point[1] / scale) + floor(point[2] / scale))%2 == 0)
        {
            // printf("COlor1\n");
            // color.print();
            return color;
        }
        else
        {
            // printf("COlor2\n");
            // color2.print();
            return color2;
        }
    }
}

Pigment::Pigment(){}

Finish::Finish(float am, float dif, float spec, float shin, float refl, float trans, float i){
    ambiance = am;
    diffuse = dif;
    specular = spec;
    shininess = shin;
    reflect = refl;
    transmission = trans;
    ior = i;
}

Finish::Finish(){}

Sphere::Sphere(){}

Sphere::Sphere(Vec3 c, float r)
{
    center = c;
    radius = r;
}

Polygon::Polygon(){}

Polygon::Polygon(int n)
{
    numFaces = n;
    numFacesSet = 0;
    faces = new Vec4[n];
}

void Polygon::AddFace(Vec4 v)
{
    faces[numFacesSet] = v;
    numFacesSet++;
}

Mesh::Mesh(){}

bool satisfies(Vec4 plane, Vec3 point, bool ineq){
    //plane: Ax+By+Cz+D>=0
    // float t;
    if (ineq)
    {
        return plane[0]*point[0]+plane[1]*point[1]+plane[2]*point[2]+plane[3] >= -0.05f;
        // printf("T: %f\n",t);
        // return t >= -0.05f;
    }

    return fabs(plane[0]*point[0]+plane[1]*point[1]+plane[2]*point[2]+plane[3]) < 0.05f;
    // printf("T: %f\n",t);
    // return t < 0.05f;
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
    else if (type == POLY)
    {
        t = INT_MAX;
        int i;
        Vec3 n,p0,p;
        float rdn, t2;
        for(i=0;i<poly.numFaces;i++){
            n = Vec3(poly.faces[i][0],poly.faces[i][1],poly.faces[i][2]);
            rdn = r.direction.dot(n);
            if (rdn != 0){
                // poly.faces[i].print();
                if (poly.faces[i][2] != 0)
                    p0 = Vec3(0,0,-poly.faces[i][3]/poly.faces[i][2]);
                else if (poly.faces[i][1] != 0)
                    p0 = Vec3(0,-poly.faces[i][3]/poly.faces[i][1],0);
                else
                    p0 = Vec3(-poly.faces[i][3]/poly.faces[i][0],0,0);
                t2 = (p0-r.origin).dot(n)/(rdn);
                // n.print();
                // p0.print();
                // r.origin.print();
                // printf("T2: %f, rdn: %f\n", t2, rdn);
            }
            if (t2 < t && t2>0){
                bool inside = true;
                for(int j=0;j<poly.numFaces;j++){
                    p = r.direction*t2+r.origin;
                    // p.print();
                    if (!satisfies(poly.faces[j],p,true)){
                        inside = false;
                    }
                }
                if (inside){
                    t = t2;
                }
            }
        }
        if (t < INT_MAX){
            // printf("COLLIDE\n");
            return true;
        }
        return false;
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
    else if (type == POLY)
    {
        for(int i=0;i<poly.numFaces;i++){
            if(satisfies(poly.faces[i],point,false)){
                return Vec3(poly.faces[i][0],poly.faces[i][1],poly.faces[i][2]);
            }
        }

        return Vec3(1, 1, 1).normal();
    }
    else
    {
        return Vec3(1, 1, 1).normal();
    }
}

Object::Object(Sphere s, int col, int f, int n){
    sphere = s;
    color = col;
    finish = f;
    numTrans = n;
    type = SPHERE;
}

Object::Object(Polygon p, int col, int f, int n){
    poly = p;
    color = col;
    finish = f;
    numTrans = n;
    type = POLY;
}

Object::Object(){}
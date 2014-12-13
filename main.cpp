#include "time.h"
#include "structs.h"
#include <string.h>
#include <stdio.h>
#include <ios>
#include <iostream>
#include <fstream>
#include <vector>

// #define PPM6

// #define DEBUGMODE
#define DEBUG_X 700
#define DEBUG_Y 100

#define DEPTH 2

// Some variables we'll read from the input file
int width;
int height;

Vec3 background_color = Vec3(0.5, 0.5, 0.5);
Vec3 cameraPos;
Vec3 cameraAt;
Vec3 cameraUp;
float fovy;

Light *lights;
int lightCount;
Pigment *pigments;
Finish *finishes;
Object *objects;
int objectCount;

Vec3* colorPoints;

char _filename[70];
std::ofstream ofs;

float myClamp(float n, float lower, float upper) {
  return std::max(lower, std::min(n, upper));
}

void WritePpm(Vec3 vec)
{
    int a = myClamp(vec[0], 0, 1)*255;
    int b = myClamp(vec[1], 0, 1)*255;
    int c = myClamp(vec[2], 0, 1)*255;

#ifdef PPM6
    ofs << (unsigned char)a << (unsigned char)b << (unsigned char)c;
#else
    ofs << a << " " << b << " " << c << " " << "\n";
#endif

    // printf("%c %c %c", (unsigned char)(vec[0]*255), (unsigned char)(vec[1]*255), (unsigned char)(vec[2]*255));
    // printf("%c %c %c\n", (int)(vec[0]*255), (int)(vec[1]*255), (int)(vec[2]*255));
    // printf("%d %d %d\n", a, b, c);
}

Vec3 Sample(Ray ray, int depth)
{
    if (depth >= 2)
    {
        return Vec3(0, 0, 0);
    }

#ifdef DEBUGMODE
    printf("Ray Origin (world): ");
    ray.origin.print();
    printf("Ray Dir (world): ");
    ray.direction.print();
#endif

    for (int i = 0; i < objectCount; i++)
    {
        float t;
        if (objects[i].Collide(ray, t))
        {
            Vec3 collide_point = ray.origin + ray.direction.scale(t);
            Vec3 normal = objects[i].normal(collide_point);

#ifdef DEBUGMODE
            printf("Collision with (%d) object number [%d]\n", objects[i].type, i);
            printf("Collision point: ");
            collide_point.print();
            printf("Collision normal: ");
            normal.print();
#endif
            int colorIdx = objects[i].color;
            int finishIdx = objects[i].finish;

            Vec3 phong = Vec3(0, 0, 0);//pigments[colorIdx].GetColor(collide_point);
            
            // Start at 1 to skip ambient light
            for (int j = 1; j < lightCount; j++)
            {
                Ray shadowRay = Ray(collide_point, (lights[j].position - collide_point).normal());

                int shadedByObject = -1;
                for (int k = 0; k < objectCount; k++)
                {
                    if (i == k)
                    {
                        // Don't test with this object
                        // AND
                        // Don't test if normal points perpendiular to/away from light
                        continue;
                    }
                    // TODO: Check that object is opaque
                    if (objects[k].Collide(shadowRay, t))
                    {
#ifdef DEBUGMODE
            printf("In shadow from light %d by (%d) object number [%d]\n", j, objects[i].type, k);
            printf("T: %f, Collided at: ", t);
            (shadowRay.origin + shadowRay.direction.scale(t)).print();
#endif
                        shadedByObject = k;
                        break;
                    }
                    else
                    {
                        float dist = ray.origin.dist(lights[j].position);
                        float distTerm = 1.0f / (float)(lights[j].attenuation[0] + (lights[j].attenuation[1]*dist) + (lights[j].attenuation[2]*pow(dist, 2)));
                        float lDotN = shadowRay.direction.dot(normal);
                        Vec3 toView = (cameraPos - collide_point).normal();
                        Vec3 r_vec = (normal * 2 * lDotN) - shadowRay.direction;
                        Vec3 difComponent = pigments[colorIdx].GetColor(collide_point) * distTerm * finishes[finishIdx].diffuse * lDotN;
                        Vec3 specComponent = lights[j].intensity * distTerm * finishes[finishIdx].specular * pow(toView.dot(r_vec), finishes[finishIdx].shininess);
                        if (normal.dot(shadowRay.direction) <= 0)
                        {
                            specComponent = Vec3(0, 0, 0);
                        }
                        Vec3 ambiComponent = lights[0].intensity * finishes[finishIdx].ambiance;
                        phong = phong + difComponent + specComponent + ambiComponent;
#ifdef DEBUGMODE
            printf("Visible by light number [%d], distance: %f, distTerm: %f\n", j, dist, distTerm);

            // More technical - optional output
            printf("lDotN %f\n", lDotN);
            toView.print();
            r_vec.print();
            printf("The dot: %f\n", toView.dot(r_vec));

            printf("Diffuse Component: ");
            difComponent.print();
            printf("Specular Component: ");
            specComponent.print();
            printf("Ambient Component: ");
            ambiComponent.print();
            printf("Phong So Far: ");
            phong.print();
#endif
                        break;
                    }
                }
            }
            
            return phong;
            // Vec3 pigments[colorIdx].GetColor(collide_point);
        }
    }

    return background_color;
}

void WritePpm(const char* str)
{
    ofs.open(_filename, std::ofstream::out | std::ofstream::app);
    ofs << str;
    ofs.close();
}



int main(int argc, char **argv) {

    // We need an input filename
    if (argc < 2) { return 0; }

    const char * filename = argv[1];
    // Load file
    FILE* fp;
    char head[70]; // we can assume this is a maximum line length per spec
    fp = fopen( filename, "rb" );
    if ( !fp ) {
      return 0;
    }

    float x,y,z,x2,y2,z2,x3,y3,z3;
 
    // Get filename line
    fgets(_filename, 70, fp);
    size_t ln = strlen(_filename) - 1;
    if (_filename[ln] == '\n')
    {
        _filename[ln] = '\0';
    }
    // sscanf(head, "%[^ ] ", _filename);
    // height, width
    fgets(head, 70, fp);
    sscanf( head, "%d %d", &width, &height);
    // camera pos
    fgets(head, 70, fp);
    sscanf( head, "%f %f %f", &x, &y, &z);
    cameraPos = Vec3(x, y, z);
    // camera at
    fgets(head, 70, fp);
    sscanf( head, "%f %f %f", &x, &y, &z);
    cameraAt = Vec3(x, y, z);
    // camera up
    fgets(head, 70, fp);
    sscanf( head, "%f %f %f", &x, &y, &z);
    cameraUp = Vec3(x, y, z);
    // fovy
    fgets(head, 70, fp);
    sscanf( head, "%f", &fovy);

    int count;
    char type[20];
    // Lights
    fgets(head, 70, fp);
    sscanf( head, "%d", &lightCount);
    lights = new Light[lightCount];
    for (int i = 0; i < lightCount; i++)
    {
        fgets(head, 70, fp);
        sscanf( head, "%f %f %f %f %f %f %f %f %f", &x, &y, &z, &x2, &y2, &z2, &x3, &y3, &z3);
        lights[i] = Light(Vec3(x,y,z), Vec3(x2,y2,z2), Vec3(x3,y3,z3));
    }
    // Pigments
    fgets(head, 70, fp);
    sscanf( head, "%d", &count);
    pigments = new Pigment[count];
    for (int i = 0; i < count; i++)
    {
        fgets(head, 70, fp);
        sscanf( head, "%[^ ] %f %f %f %f %f %f %f", type, &x, &y, &z, &x2, &y2, &z2, &x3);
        if (strcmp(type, "solid") == 0)
        {
            pigments[i] = Pigment(SOLID, Vec3(x,y,z), Vec3(x,y,z), 1);
        }
        else
        {
            pigments[i] = Pigment(CHECKERED, Vec3(x,y,z), Vec3(x2,y2,z2), x3);   
        }
    }
    // Finishes
    fgets(head, 70, fp);
    sscanf( head, "%d", &count);
    finishes = new Finish[count];
    for (int i = 0; i < count; i++)
    {
        fgets(head, 70, fp);
        sscanf( head, "%f %f %f %f %f %f %f", &x, &y, &z, &x2, &y2, &z2, &x3);
        finishes[i] = Finish(x, y, z, x2, y2, z2, x3);
    }
    // Transformations
    fgets(head, 70, fp);
    sscanf( head, "%d", &count);
    for (int i = 0; i < count; i++)
    {
        fgets(head, 70, fp);
    }
    // Objects
    fgets(head, 70, fp);
    sscanf( head, "%d", &objectCount);
    objects = new Object[objectCount];
    for (int i = 0; i < objectCount; i++)
    {
        fgets(head, 70, fp);
        if (strstr(head, "sphere") != NULL)
        {
            sscanf( head, "%f %f %f %[^ ] %f %f %f %f", &x, &y, &z, type, &x2, &y2, &z2, &x3);
            objects[i] = Object(Sphere(Vec3(x2,y2,z2), x3), x, y, z);
        }
    }

    fclose( fp );
    
    ofs.open(_filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    char buffer[50];
#ifdef PPM6
    sprintf(buffer, "P6 %d %d 255 \n", width, height);
#else
    sprintf(buffer, "P3 %d %d 255 \n", width, height);
#endif // PPM6
    WritePpm(buffer);

    Camera cam = Camera(cameraPos, cameraAt, cameraUp);
    // The Ray Tracer
    colorPoints = new Vec3[height*width];
    float nCols = width;
    float nRows = height;
    float h = 2 * tan(fovy*PI/360);
    float w = h * nCols/nRows;

#ifdef DEBUGMODE

    int i = DEBUG_Y;
    int j = DEBUG_X;
    printf("i: %d, j: %d\n", i, j);
    printf("w: %f, h: %f\n", w, h);

    float p_x = w*((float)j/nCols) - (w/2);
    float p_y = -h*((float)i/nRows) + (h/2);
    printf("P_x: %f, P_y: %f\n", p_x, p_y);

    Vec3 at = cam.origin + cam.x.scale(p_x) + cam.y.scale(p_y) + cam.z.scale(-1);
    
    printf("At (world): ");
    at.print();

    colorPoints[i*height + j] = Sample(Ray(cam.origin, (at - cam.origin).normal()), 1);

#else

    // printf("%f, %f, %f, %f\n", nCols, nRows, h, w);
    //1000.000000, 1000.000000, 3.239550, 3.239550
    ofs.open(_filename, std::ofstream::out | std::ofstream::app);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)   
        {
            float p_x = w*((float)j/nCols) - (w/2);
            float p_y = -h*((float)i/nRows) + (h/2);
            // printf("%f, %f\n", p_x, p_y);
            //P(i, j) = E + ( px)C.x + ( py)C.y + (−1)C.z

            Vec3 at = cam.origin + cam.x.scale(p_x) + cam.y.scale(p_y) + cam.z.scale(-1);

            colorPoints[i*height + j] = Sample(Ray(cam.origin, (at - cam.origin).normal()), 1);
            WritePpm(colorPoints[i*height + j]);
        }
    }

    ofs.close();

#endif // DEBUGMODE

    return 0;
}
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
#define DEBUG_X 659
#define DEBUG_Y 815

// #region Some parameters we might want to change
#define DEPTH 5
float AA = 3.0f;
float AA_DEL = 0.5f;
// #endregion

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

int randSymRange(int m)
{
    return (rand() % (m*2) - m);
}

float myClamp(float n, float lower, float upper) {
  return std::max(lower, std::min(n, upper));
}

Vec3 myClamp(Vec3 v, float lower, float upper) {
    return Vec3(myClamp(v[0], lower, upper), myClamp(v[1], lower, upper), myClamp(v[2], lower, upper));
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

#ifdef DEBUGMODE
    printf("Depth: %d\n", depth);
    printf("Ray Origin (world): ");
    ray.origin.print();
    printf("Ray Dir (world): ");
    ray.direction.print();
#endif

    if (depth >= DEPTH)
    {
        return Vec3(0, 0, 0);
    }

    float t = INT_MAX;
    float lowT;
    int firstObjectIdx = -1;
    for (int i = 0; i < objectCount; i++)
    {
        if (objects[i].Collide(ray, lowT))
        {
            if (lowT < t)
            {
                t = lowT;
                firstObjectIdx = i;
            }
        }
    }

    if (firstObjectIdx == -1)
    {
        return background_color;
    }
    else
    {
        // Calculate color

        Vec3 collide_point = ray.origin + ray.direction.scale(t);
        Vec3 normal = objects[firstObjectIdx].normal(collide_point);

#ifdef DEBUGMODE
        printf("Collision with (%d) object number [%d]\n", objects[firstObjectIdx].type, firstObjectIdx);
        printf("Collision point: ");
        collide_point.print();
        printf("Collision normal: ");
        normal.print();
#endif
        int colorIdx = objects[firstObjectIdx].color;
        int finishIdx = objects[firstObjectIdx].finish;

        Vec3 phong = Vec3(0, 0, 0);//pigments[colorIdx].GetColor(collide_point);
        
        // Start at 1 to skip ambient light
        for (int j = 1; j < lightCount; j++)
        {
            Ray shadowRay = Ray(collide_point, (lights[j].position - collide_point).normal());
            float disToLight = collide_point.dist(lights[j].position);

            int shadedByObject = -1;
            int lowestT = INT_MAX;
            for (int k = 0; k < objectCount; k++)
            {
                if (firstObjectIdx == k)
                {
                    // Don't test with this object
                    // AND
                    // Don't test if normal points perpendiular to/away from light
                    continue;
                }
                // TODO: Check that object is opaque
                
                if (objects[k].Collide(shadowRay, t))
                {
                    Vec3 shadow_collide_point = shadowRay.origin + shadowRay.direction * t;
                    if (disToLight > collide_point.dist(shadow_collide_point))
                    {
                        
#ifdef DEBUGMODE
        printf("In shadow from light %d by (%d) object number [%d]\n", j, objects[firstObjectIdx].type, k);
        printf("T: %f, Collided at: ", t);
        (shadowRay.origin + shadowRay.direction.scale(t)).print();
#endif
                        if(t<lowestT){
                          lowestT = t;
                          shadedByObject = k;
                        }
                    }
                }
            }
            if (shadedByObject == -1) 
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
                else
                {
                    difComponent[0] *= lights[j].intensity[0];
                    difComponent[1] *= lights[j].intensity[1];
                    difComponent[2] *= lights[j].intensity[2];
                }

                Vec3 ambiComponent = lights[0].intensity * finishes[finishIdx].ambiance;
                phong = phong + difComponent + specComponent + ambiComponent;
#ifdef DEBUGMODE
      printf("Spec: %f, tdr: %f, shine: %f\n", finishes[finishIdx].specular, toView.dot(r_vec), finishes[finishIdx].shininess);
      printf("Visible by light number [%d], distance: %f, distTerm: %f\n", j, dist, distTerm);
      printf("Light intensity: ");
      lights[j].intensity.print();
      printf("Light attenuation: ");
      lights[j].attenuation.print();

      // More technical - optional output
      // printf("lDotN %f\n", lDotN);
      // toView.print();
      // r_vec.print();
      // printf("The dot: %f\n", toView.dot(r_vec));

      printf("Diffuse Component: ");
      difComponent.print();
      printf("Specular Component: ");
      specComponent.print();
      printf("Ambient Component: ");
      ambiComponent.print();
      printf("Phong So Far: ");
      phong.print();
#endif
            }
            else
            {
                phong = phong + Sample(shadowRay, depth+1)*finishes[objects[shadedByObject].finish].reflect;     
            }
        }    

        Vec3 reflect = Vec3(0, 0, 0);
        if (finishes[finishIdx].reflect != 0)
        {
            float NdotV = -normal.dot(ray.direction);
            Vec3 ref_Direction = ray.direction + (normal * 2 * NdotV);
            reflect = Sample(Ray(collide_point, ref_Direction.normal()), depth+1);

#ifdef DEBUGMODE
        printf("Reflection Direction: ");
        ref_Direction.normal().print();
        printf("Reflect Component: ");
        reflect.print();
#endif
        }

        // Entering object
        float ior = finishes[finishIdx].ior;  
        float cos_theta_1 = -normal.dot(ray.direction);
        float sin_theta_1 = sqrt(1 - pow(cos_theta_1, 2));
        // bool enter = true;

        if (cos_theta_1 <= 0)
        {
            // Leaving object
            ior = 1/ior;
            normal = normal * -1;
            cos_theta_1 *= -1;
            // enter = false;
            // printf("LEAVING!\n");
        }

        // float c2 = sqrt(1-pow(ior, 2) * (1 - pow(NdotV, 2)));
        // Vec3 refract_Dir = (ray.direction * ior) + normal * (ior * NdotV - c2);

        float sin_theta_2_sq = pow(sin_theta_1/ior, 2);
        Vec3 refract_Dir = ray.direction*(1/ior) + normal * ((1/ior)*cos_theta_1 - sqrt(1 - sin_theta_2_sq));

#ifdef DEBUGMODE
        if (finishes[finishIdx].transmission > 0)
        {
            printf("Depth: %d\n", depth);
            printf("refract_Dir Direction: ");
            refract_Dir.normal().print();
            printf("IOR: %f\n", ior);
        }
#endif

        Vec3 refraction = Vec3(0, 0, 0);
        if (sin_theta_2_sq < 1 && finishes[finishIdx].transmission != 0)
        {
            refraction = Sample(Ray(collide_point, refract_Dir.normal()), depth+1);   
        }

#ifdef DEBUGMODE
        if (finishes[finishIdx].transmission > 0)
        {
            printf("Refraction Component: ");
            refraction.print();
        }
#endif

        return myClamp(phong, 0, 1) * (1 - finishes[finishIdx].transmission) + reflect * finishes[finishIdx].reflect + refraction * finishes[finishIdx].transmission;
    }
}

void WritePpm(const char* str)
{
    ofs.open(_filename, std::ofstream::out | std::ofstream::app);
    ofs << str;
    ofs.close();
}

void getNextLine(FILE* fp, char* hold, int len) {
    fgets(hold, len, fp);
    while (hold[0] == '#') {
        printf("Commented line: %s\n", hold);
        fgets(hold, len, fp);
    }
}

int main(int argc, char **argv) {

    srand (time(NULL));

    time_t startTime = time(0);
    if (AA == 1)
    {
        AA_DEL = 0;
    }

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
    getNextLine(fp, _filename, 70);

    strcpy(_filename, filename);
    strcat(_filename, ".ppm");

    printf("Output filename: %s\n", _filename);

    size_t ln = strlen(_filename) - 1;
    if (_filename[ln] == '\n')
    {
        _filename[ln] = '\0';
    }
    // height, width
    getNextLine(fp, head, 70);
    sscanf( head, "%d %d", &width, &height);
    // camera pos
    getNextLine(fp, head, 70);
    sscanf( head, "%f %f %f", &x, &y, &z);
    cameraPos = Vec3(x, y, z);
    // camera at
    getNextLine(fp, head, 70);
    sscanf( head, "%f %f %f", &x, &y, &z);
    cameraAt = Vec3(x, y, z);
    // camera up
    getNextLine(fp, head, 70);
    sscanf( head, "%f %f %f", &x, &y, &z);
    cameraUp = Vec3(x, y, z);
    // fovy
    getNextLine(fp, head, 70);
    sscanf( head, "%f", &fovy);

    int count;
    char type[20];
    // Lights
    getNextLine(fp, head, 70);
    sscanf( head, "%d", &lightCount);
    lights = new Light[lightCount];
    for (int i = 0; i < lightCount; i++)
    {
        getNextLine(fp, head, 70);
        sscanf( head, "%f %f %f %f %f %f %f %f %f", &x, &y, &z, &x2, &y2, &z2, &x3, &y3, &z3);
        lights[i] = Light(Vec3(x,y,z), Vec3(x2,y2,z2), Vec3(x3,y3,z3));
    }
    // Pigments
    getNextLine(fp, head, 70);
    sscanf( head, "%d", &count);
    pigments = new Pigment[count];
    for (int i = 0; i < count; i++)
    {
        getNextLine(fp, head, 70);
        sscanf( head, "%[^ ] %f %f %f %f %f %f %f", type, &x, &y, &z, &x2, &y2, &z2, &x3);

        // Normalize 255 color space
        if (x > 1) { x /= 255; } if (y > 1) { y /= 255; } if (z > 1) { z /= 255; }
        if (x2 > 1) { x2 /= 255; } if (y2 > 1) { y2 /= 255; } if (z2 > 1) { z2 /= 255; }

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
    getNextLine(fp, head, 70);
    sscanf( head, "%d", &count);
    finishes = new Finish[count];
    for (int i = 0; i < count; i++)
    {
        getNextLine(fp, head, 70);
        sscanf( head, "%f %f %f %f %f %f %f", &x, &y, &z, &x2, &y2, &z2, &x3);
        finishes[i] = Finish(x, y, z, x2, y2, z2, x3);
    }
    // Transformations
    getNextLine(fp, head, 70);
    sscanf( head, "%d", &count);
    for (int i = 0; i < count; i++)
    {
        getNextLine(fp, head, 70);
    }
    // Objects
    getNextLine(fp, head, 70);
    sscanf( head, "%d", &objectCount);
    objects = new Object[objectCount];
    for (int i = 0; i < objectCount; i++)
    {
        getNextLine(fp, head, 70);
        if (strstr(head, "sphere") != NULL)
        {
            sscanf( head, "%f %f %f %[^ ] %f %f %f %f", &x, &y, &z, type, &x2, &y2, &z2, &x3);

            // For now, use 0.01 as our flag for "random"
            if (x2 == 0.01f ) {
                x2 = randSymRange(100);
                printf("Random x: %f\n", x2);
            }
            if (y2 == 0.01f ) {
                y2 = randSymRange(40);
                printf("Random y: %f\n", y2);
            }
            if (z2 == 0.01f ) {
                z2 = randSymRange(100);
                printf("Random z: %f\n", z2);
            }
            
            objects[i] = Object(Sphere(Vec3(x2,y2,z2), x3), x, y, z);
        }
        else if (strstr(head, "polyhedron") != NULL) 
        {
            sscanf( head, "%f %f %f %[^ ] %f", &x, &y, &z, type, &x2);
            objects[i] = Object(Polygon(x2), x, y, z);

            for (int j = 0; j < x2; j++)
            {
                getNextLine(fp, head, 70);
                sscanf( head, "%f %f %f %f", &x, &y, &z, &x3);
                objects[i].poly.AddFace(Vec4(x, y, z, x3));
            }
        }
        else
        {
            getNextLine(fp, head, 70);
        }
    }

    fclose( fp );
    
    ofs.open(_filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    char buffer[50];
#ifdef PPM6
    sprintf(buffer, "P6 %d %d 255 ", width, height);
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

    Vec3 color = Sample(Ray(cam.origin, (at - cam.origin).normal()), 1);
    printf("Color: ");
    color.print();

#else

    // printf("%f, %f, %f, %f\n", nCols, nRows, h, w);
    //1000.000000, 1000.000000, 3.239550, 3.239550

    float del_x = AA_DEL * w/nCols;
    float del_y = AA_DEL * h/nRows;

    ofs.open(_filename, std::ofstream::out | std::ofstream::app);
    int pixelCnt = 0;
    float reportStep = 5;
    float progressReport = reportStep;
    float totalPixels = (float)height * width / 100.0f;

    float p_x, p_y;
    float progress;
    Vec3 at;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)   
        {
            Vec3 color_AA = Vec3(0, 0, 0);

            for (int ii = 0; ii < AA; ii++)
            {
                for (int jj = 0; jj < AA; jj++)   
                {
                    p_x = w*((float)j/nCols) - (w/2);
                    p_y = -h*((float)i/nRows) + (h/2);
                    // printf("px %f, py %f, ii %d, jj %d\n",p_x, p_y, ii, jj);
                    p_x += (jj*2*del_x)/(float)AA-del_x;
                    p_y += (ii*2*del_y)/(float)AA-del_y;
                    // printf("px %f, py %f, %f, %f, %f\n",p_x, p_y, AA, del_x, del_y);

                    at = cam.origin + cam.x.scale(p_x) + cam.y.scale(p_y) + cam.z.scale(-1);

                    color_AA = color_AA + Sample(Ray(cam.origin, (at - cam.origin).normal()), 1) * (1/(float)(AA*AA));
                }
            }

            colorPoints[i*height + j] = color_AA;
            WritePpm(colorPoints[i*height + j]);

            progress = pixelCnt++ / totalPixels;

            // printf("%f, %f, %f, %d\n", progress, progressReport, totalPixels, pixelCnt);

            if (progress > progressReport) {
                printf("Progress: %d%%\n", (int)progress);
                progressReport += reportStep;
            }
        }
    }

    ofs.close();

#endif // DEBUGMODE

    printf("Total time: %lds\n", time(0) - startTime);
    return 0;
}

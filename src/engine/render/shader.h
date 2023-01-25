#ifndef SHADER_H_
#define SHADER_H_

enum
{
    Shader_Default    = 0,
    Shader_World      = 1 << 0,
    Shader_Refract    = 1 << 2,
    Shader_Option     = 1 << 3,
    Shader_Dynamic    = 1 << 4,
    Shader_Triplanar  = 1 << 5,

    Shader_Invalid    = 1 << 8,
    Shader_Deferred   = 1 << 9
};

extern float blursigma;
extern Shader *nullshader, *hudshader, *hudnotextureshader, *nocolorshader, *foggednotextureshader, *ldrnotextureshader;
const int maxblurradius = 7;

extern int getlocalparam(const char *name);
extern void setupblurkernel(int radius, float *weights, float *offsets);
extern void setblurshader(int pass, int size, int radius, const float *weights, const float *offsets, GLenum target = GL_TEXTURE_2D);

#endif

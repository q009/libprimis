#ifndef LIGHT_H_
#define LIGHT_H_

struct PackNode
{
    PackNode *child1, *child2;
    ushort x, y, w, h;
    int available;

    PackNode(ushort x, ushort y, ushort w, ushort h) : child1(0), child2(0), x(x), y(y), w(w), h(h), available(min(w, h)) {}

    void discardchildren()
    {
        DELETEP(child1);
        DELETEP(child2);
    }

    void forceempty()
    {
        discardchildren();
        available = 0;
    }

    void reset()
    {
        discardchildren();
        available = min(w, h);
    }

    bool resize(int nw, int nh)
    {
        if(w == nw && h == nw)
        {
            return false;
        }
        discardchildren();
        w = nw;
        h = nh;
        available = min(w, h);
        return true;
    }

    ~PackNode()
    {
        discardchildren();
    }

    bool insert(ushort &tx, ushort &ty, ushort tw, ushort th);
    void reserve(ushort tx, ushort ty, ushort tw, ushort th);
};

extern bvec ambient, skylight, sunlight;
extern float ambientscale, skylightscale, sunlightscale;
extern float sunlightyaw, sunlightpitch;
extern vec sunlightdir;
extern int fullbright, fullbrightlevel;

extern void clearlights();
extern void initlights();
extern void clearlightcache(int id = -1);
extern void brightencube(cube &c);
extern void setsurfaces(cube &c, const surfaceinfo *surfs, const vertinfo *verts, int numverts);
extern void setsurface(cube &c, int orient, const surfaceinfo &surf, const vertinfo *verts, int numverts);

extern void calcnormals(bool lerptjoints = false);
extern void clearnormals();
extern void resetsmoothgroups();
extern int smoothangle(int id, int angle);
extern void findnormal(const vec &key, int smooth, const vec &surface, vec &v);

extern bool calclight_canceled;

extern void check_calclight_canceled();

#endif

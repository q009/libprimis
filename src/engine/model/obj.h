#ifndef OBJ_H_
#define OBJ_H_

struct obj;

struct obj : vertloader<obj>
{
    obj(const char *name) : vertloader(name) {}

    static const char *formatname();
    static bool cananimate();
    bool flipy() const;
    int type() const;

    struct objmeshgroup : vertmeshgroup
    {
        public:
            bool load(const char *filename, float smooth);

        private:
            void parsevert(char *s, std::vector<vec> &out);
            void flushmesh(const string meshname,
                           vertmesh *curmesh,
                           const std::vector<vert> &verts,
                           const std::vector<tcvert> &tcverts,
                           const std::vector<tri> &tris,
                           const std::vector<vec> &attrib,
                           float smooth);
    };

    vertmeshgroup *newmeshes()
    {
        return new objmeshgroup;
    }

    bool loaddefaultparts();
};

#endif


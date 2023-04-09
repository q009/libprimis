#ifndef SKELBIH_H_
#define SKELBIH_H_

class skelhitzone;

struct skelhittri : skelmodel::tri
{
    uchar Mesh, id;
};

class skelzonekey
{
    public:
        skelzonekey();
        skelzonekey(int bone);
        skelzonekey(const skelmodel::skelmesh *m, const skelmodel::tri &t);

        bool includes(const skelzonekey &o) const;
        void subtract(const skelzonekey &o);

        int blend;
        std::array<uchar, 12> bones;

    private:
        bool hasbone(int n) const;

        int numbones() const;
        void addbone(int n);
        void addbones(const skelmodel::skelmesh *m, const skelmodel::tri &t);
};

class skelhitdata
{
    public:
        skelhitdata();
        ~skelhitdata();
        void build(const skelmodel::skelmeshgroup *g, const uchar *ids);
        void propagate(const skelmodel::skelmeshgroup *m, const dualquat *bdata1, dualquat *bdata2);
        void cleanup();
        void intersect(const skelmodel::skelmeshgroup *m, skelmodel::skin *s, const dualquat *bdata1, dualquat *bdata2, const vec &o, const vec &ray);

        int getblendcount();
        skelmodel::blendcacheentry &getcache();

    private:
        int numblends;
        skelmodel::blendcacheentry blendcache;
        int numzones, rootzones, visited;
        skelhitzone *zones;
        skelhitzone **links;
        skelhittri *tris;

        uchar chooseid(const skelmodel::skelmeshgroup *g, const skelmodel::skelmesh *m, const skelmodel::tri &t, const uchar *ids);

        class skelzoneinfo
        {
            public:
                int index, parents, conflicts;
                skelzonekey key;
                std::vector<skelzoneinfo *> children;
                std::vector<skelhittri> tris;

                skelzoneinfo() : index(-1), parents(0), conflicts(0) {}
                skelzoneinfo(const skelzonekey &key) : index(-1), parents(0), conflicts(0), key(key) {}
        };
    //need to set htcmp to friend because it must be in global scope for hashtable macro to find it
    friend bool htcmp(const skelzonekey &x, const skelhitdata::skelzoneinfo &y);
};

#endif

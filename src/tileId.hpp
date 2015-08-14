#ifndef TILEID_HPP_INCLUDED
#define TILEID_HPP_INCLUDED

#include <iostream>

struct tileId {
        unsigned int _z, _x, _y;
public:
        tileId ();
        tileId (unsigned int zz, unsigned int xx, unsigned int yy);
        unsigned int z() const;
        unsigned int x() const;
        unsigned int y() const;
        unsigned int operator[] (unsigned char i) const;
};
bool operator== (const tileId& a, const tileId& b);
bool operator< (const tileId& a, const tileId& b);
std::ostream& operator<<(std::ostream& os, const tileId& id);

inline tileId::tileId ()
    : _z(0), _x(0), _y(0)
{
        //No-op
}

inline tileId::tileId (unsigned int zz, unsigned int xx, unsigned int yy)
{
        _z = zz; _x = xx; _y = yy;
}

inline unsigned int tileId::z() const {return _z;}
inline unsigned int tileId::x() const {return _x;}
inline unsigned int tileId::y() const {return _y;}

inline unsigned int tileId::operator[] (unsigned char i) const
{
        if (i > 2) return 0;
        if (i == 0) return _z;
        if (i == 1) return _x;
        return _y;
}

inline bool operator== (const tileId& a, const tileId& b)
{
        return a.z() == b.z() && a.x() == b.x() && a.y() == b.y();
}

inline bool operator< (const tileId& a, const tileId& b)
{
        if (a.z() == b.z()) {
                if (a.x() == b.x()) {
                        return a.y() < b.y();
                } else {
                        return a.x() < b.x();
                }
        }
        return a.z() < b.z();
}

inline std::ostream& operator<<(std::ostream& os, const tileId& id)
{
    os << "(" << id.z() << ", " << id.x() << ", " << id.y() << ")";
    return os;
}


#endif //TILEID_HPP_INCLUDED
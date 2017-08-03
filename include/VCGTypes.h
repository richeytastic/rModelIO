#ifndef RMODELIO_VCG_TYPES_H
#define RMODELIO_VCG_TYPES_H

#include "rModelIO_Export.h"
#include <boost/shared_ptr.hpp>
#include <vcg/complex/complex.h>    // VCG

namespace RModelIO
{

using namespace vcg;

class VCGObjModelFace;
class VCGObjModelVertex;
struct VCGObjModelUsedTypes : public UsedTypes< Use<VCGObjModelVertex>::AsVertexType, Use<VCGObjModelFace>::AsFaceType >{};

class VCGObjModelVertex : public Vertex< VCGObjModelUsedTypes, vertex::Coord3f, vertex::BitFlags >{};   // x,y,z
class VCGObjModelFace   : public Face< VCGObjModelUsedTypes, face::VertexRef, face::FFAdj, face::WedgeTexCoord2f, face::BitFlags >{};

class VCGObjModel : public tri::TriMesh< std::vector<VCGObjModelVertex>, std::vector<VCGObjModelFace> >
{
public:
    typedef boost::shared_ptr<VCGObjModel> Ptr;
};  // end class

}   // end namespace

#endif





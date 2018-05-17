/************************************************************************
 * Copyright (C) 2017 Richard Palmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

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
struct VCGObjModelUsedTypes : public UsedTypes< Use<VCGObjModelVertex>::AsVertexType, Use<VCGObjModelFace>::AsFaceType>{};

class VCGObjModelVertex : public Vertex< VCGObjModelUsedTypes, vertex::Coord3f, vertex::BitFlags >{};   // x,y,z
class VCGObjModelFace   : public Face< VCGObjModelUsedTypes, face::VertexRef, face::FFAdj, face::WedgeTexCoord2f, face::BitFlags >{};

class VCGObjModel : public tri::TriMesh< std::vector<VCGObjModelVertex>, std::vector<VCGObjModelFace> >
{
public:
    typedef boost::shared_ptr<VCGObjModel> Ptr;
};  // end class

}   // end namespace

#endif





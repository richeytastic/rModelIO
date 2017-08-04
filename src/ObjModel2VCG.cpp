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

#include <ObjModel2VCG.h>
using RModelIO::ObjModel2VCG;
using RModelIO::VCGObjModel;
using RFeatures::ObjModel;
#include <cassert>
#include <iostream>
#include <boost/foreach.hpp>


// public
ObjModel2VCG::ObjModel2VCG()
{
}   // end ctor


VCGObjModel::Ptr ObjModel2VCG::create( const ObjModel::Ptr model)
{
    typedef boost::unordered_map<int,int> IIMap;
    VCGObjModel* vmod = new VCGObjModel;

    // Since ObjModel vertices are iterated over in an unpredictable order, store the mappings to the VCG vertex pointers.
    int i = 0;
    const IntSet& vidxs = model->getVertexIds();
    IIMap objToVCGVerts;
    std::vector<VCGObjModel::VertexPointer> vps( model->getNumVertices());

    BOOST_FOREACH ( int vidx, vidxs)
    {
        objToVCGVerts[vidx] = i; // Map Obj to VCG vertex ID
        const cv::Vec3f& v = model->vtx(vidx);
        vps[i++] = &*vcg::tri::Allocator<VCGObjModel>::AddVertex( *vmod, VCGObjModel::CoordType( v[0], v[1], v[2]));
    }   // end foreach

    // Add faces, setting the texture coords as we go.
    IIMap m2t;  // ObjModel Material ID 2 vmod->textures index.
    int tn;
    int vorder[3];      // Vertex IDs in texture offset order on a face from ObjModel
    cv::Vec2f uvs[3];   // Corresponding texture offsets from the model
    const IntSet& fids = model->getFaceIds();
    BOOST_FOREACH ( int fid, fids)
    {
        // Only gets texture offsets if a material exists on face fid.
        const int mid = model->getOrderedFaceTextureOffsets( fid, vorder, uvs);
        if ( mid < 0)
        {
            // No material, so set according to the ascending order of vertex IDs given in the ObjPoly.
            const int *vindices = model->getFace(fid).vindices;
            vcg::tri::Allocator<VCGObjModel>::AddFace( *vmod, vps[objToVCGVerts[vindices[0]]],
                                                              vps[objToVCGVerts[vindices[1]]],
                                                              vps[objToVCGVerts[vindices[2]]]);
        }   // end if
        else 
        {
            VCGObjModel::FaceIterator fi = vcg::tri::Allocator<VCGObjModel>::AddFace( *vmod, vps[objToVCGVerts[vorder[0]]],
                                                                                             vps[objToVCGVerts[vorder[1]]],
                                                                                             vps[objToVCGVerts[vorder[2]]]);
            (*fi).WT(0).U() = uvs[0][0];
            (*fi).WT(0).V() = uvs[0][1];
            (*fi).WT(1).U() = uvs[1][0];
            (*fi).WT(1).V() = uvs[1][1];
            (*fi).WT(2).U() = uvs[2][0];
            (*fi).WT(2).V() = uvs[2][1];

            // VCG only supports one texture per material (use diffuse)
            if ( m2t.count(mid) == 0)
                m2t[mid] = (int)vmod->textures.size();
            tn = m2t[mid];
            vmod->textures.resize( tn+1);   // Resize the texture filename array, but leave the elements empty.

            (*fi).WT(0).N() = (*fi).WT(1).N() = (*fi).WT(2).N() = tn; // Index into vmod->textures

            // Finally, store reference to the diffuse texture in the material (though will take ambient or specular too)
            _tmaps.resize( tn+1);
            const ObjModel::Material& mat = model->getMaterial(mid);
            if ( !mat.diffuse.empty())  // Preference diffuse...
                _tmaps[tn] = mat.diffuse[0];
            else if ( !mat.ambient.empty()) // then ambient if diffuse unavailable...
                _tmaps[tn] = mat.ambient[0];
            else if ( !mat.specular.empty())    // then specular if ambient unavailable.
                _tmaps[tn] = mat.specular[0];
            else
            {
                assert(false);
                std::cerr << "[ERROR] ObjModel2VCG::create: no texture maps in referenced ObjModel::Material!" << std::endl;
            }   // end else
        }   // end if
    }   // end foreach

    return VCGObjModel::Ptr( vmod);
}   // end create

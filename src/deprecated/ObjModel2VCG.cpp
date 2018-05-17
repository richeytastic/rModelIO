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


// If vertex vidx from model has not yet been added to vmod, add it.
void checkAddVertex( int vidx, boost::unordered_map<int, VCGObjModel::VertexPointer>* vvmap, const ObjModel::Ptr model, VCGObjModel* vmod)
{
    if ( vvmap->count(vidx) == 0)
    {
        const cv::Vec3f& v = model->vtx(vidx);
        (*vvmap)[vidx] = &*vcg::tri::Allocator<VCGObjModel>::AddVertex( *vmod, VCGObjModel::CoordType( v[0], v[1], v[2]));
    }   // end if
}   // end checkAddVertex


// public
VCGObjModel::Ptr ObjModel2VCG::create( const ObjModel::Ptr model)
{
    VCGObjModel* vmod = new VCGObjModel;

    // Since ObjModel vertices are iterated over in an unpredictable order, store the mappings to the VCG vertex pointers.
    const IntSet& fids = model->getFaceIds();
    VCGObjModel::FaceIterator fi = vcg::tri::Allocator<VCGObjModel>::AddFaces( *vmod, fids.size());

    // Map ObjModel vertex indices to the VCGObjModel's corresponding vertex pointers added in checkAddVertex.
    boost::unordered_map<int, VCGObjModel::VertexPointer>* vvmap = new boost::unordered_map<int, VCGObjModel::VertexPointer>;

    boost::unordered_map<int,int> m2t;  // ObjModel material ID to VCGObjModel texture id (index into vmod->textures).
    BOOST_FOREACH ( int fid, fids)
    {
        const int *vids = model->getFaceVertices(fid);
        checkAddVertex( vids[0], vvmap, model, vmod);
        checkAddVertex( vids[1], vvmap, model, vmod);
        checkAddVertex( vids[2], vvmap, model, vmod);
        fi->V(0) = vvmap->at(vids[0]);
        fi->V(1) = vvmap->at(vids[1]);
        fi->V(2) = vvmap->at(vids[2]);

        const int mid = model->getFaceMaterialId( fid);
        if ( mid >= 0)
        {
            const int* uvids = model->getFaceUVs(fid);
            const cv::Vec2f& uv0 = model->uv( mid, uvids[0]);
            const cv::Vec2f& uv1 = model->uv( mid, uvids[1]);
            const cv::Vec2f& uv2 = model->uv( mid, uvids[2]);
            VCGObjModel::FacePointer fp = &*fi;
            (*fp).WT(0).U() = uv0[0];
            (*fp).WT(0).V() = uv0[1];
            (*fp).WT(1).U() = uv1[0];
            (*fp).WT(1).V() = uv1[1];
            (*fp).WT(2).U() = uv2[0];
            (*fp).WT(2).V() = uv2[1];

            // VCG only supports one texture per material (use diffuse)
            if ( m2t.count(mid) == 0)
                m2t[mid] = (int)vmod->textures.size();
            const int tn = m2t[mid];
            vmod->textures.resize( tn+1);   // Resize the texture filename array, but leave the elements empty.

            (*fp).WT(0).N() = (*fp).WT(1).N() = (*fp).WT(2).N() = tn; // Index into vmod->textures

            // Store reference to the diffuse texture in the material (though will take ambient or specular too)
            _tmaps.resize( tn+1);
            const std::vector<cv::Mat>& ambient = model->getMaterialAmbient(mid);
            const std::vector<cv::Mat>& diffuse = model->getMaterialDiffuse(mid);
            const std::vector<cv::Mat>& specular = model->getMaterialSpecular(mid);

            if ( !diffuse.empty())  // Preference diffuse...
                _tmaps[tn] = diffuse[0];
            else if ( !ambient.empty()) // then ambient if diffuse unavailable...
                _tmaps[tn] = ambient[0];
            else if ( !specular.empty())    // then specular if ambient unavailable.
                _tmaps[tn] = specular[0];
            else
            {
                assert(false);
                std::cerr << "[ERROR] ObjModel2VCG::create: no textures in ObjModel::Material!" << std::endl;
            }   // end else
        }   // end if

        fi++;   // Ready for next face
    }   // end foreach

    delete vvmap;
    return VCGObjModel::Ptr( vmod);
}   // end create


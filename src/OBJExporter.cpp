/************************************************************************
 * Copyright (C) 2019 Richard Palmer
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

#include <OBJExporter.h>
using RModelIO::OBJExporter;
using RFeatures::ObjModel;
#include <boost/filesystem/operations.hpp>
#include <fstream>


OBJExporter::OBJExporter() : RModelIO::ObjModelExporter()
{
    addSupported( "obj", "Wavefront OBJ");
}   // end ctor

namespace {

std::string getMaterialName( const std::string& fname, int midx)
{
    const std::string fstem = boost::filesystem::path(fname).filename().stem().string();
    std::ostringstream oss;
    oss << fstem << "_" << midx;
    return oss.str();
}   // end getMaterialName


// Write out the .mtl file - returning any error string.
std::string writeMaterialFile( const ObjModel* model, const std::string& fname)
{
    const boost::filesystem::path ppath = boost::filesystem::path(fname).parent_path();
    std::string err;
    std::ofstream ofs;
    try
    {
        ofs.open( fname.c_str(), std::ios::out);
        ofs << "# Wavefront OBJ material file produced by RModelIO (https://github.com/richeytastic/rModelIO)" << std::endl;
        ofs << std::endl;

        int pmid = 0;   // Will be set to the 'pseudo' material ID in the event nfaces < total model faces.
        int nfaces = 0;
        const IntSet& mids = model->materialIds();
        for ( int mid : mids)
        {
            nfaces += int(model->materialFaceIds(mid).size());
            const std::string matname = getMaterialName( fname, mid);
            ofs << "newmtl " << matname << std::endl;
            ofs << "illum 1" << std::endl;
            const cv::Mat tx = model->texture(mid);
            if ( !tx.empty())
            {
                std::ostringstream oss;
                oss << matname << ".png";
                ofs << "map_Kd " << oss.str() << std::endl;
                const std::string imgfile = (ppath / oss.str()).string();
                cv::imwrite( imgfile, tx);
            }   // end if

            ofs << std::endl;
            pmid = mid+1;
        }   // end foreach

        // Do we need an extra 'pseudo' material?
        assert( nfaces <= model->numPolys());
        if ( nfaces < model->numPolys())
        {
            ofs << "newmtl " << getMaterialName( fname, pmid) << std::endl;
            ofs << "illum 1" << std::endl;
        }   // end if

        ofs.close();
    }   // end try
    catch ( const std::exception &e)
    {
        err = e.what();
    }   // end catch

    return err;
}   // end writeMaterialFile


using IIMap = std::unordered_map<int,int>;

void writeVertices( std::ostream& os, const ObjModel* model, IIMap& vvmap)
{
    int i = 0;
    const IntSet& vids = model->vtxIds();
    for ( int vid : vids)
    {
        vvmap[vid] = ++i;   // Preincrement because vertex indices start at one for OBJ
        const cv::Vec3f& v = model->vtx(vid);
        os << "v\t" << v[0] << " " << v[1] << " " << v[2] << std::endl;
    }   // end for
}   // end writeVertices


void writeMaterialUVs( std::ostream& os, const ObjModel* model, int midx, IIMap& uvmap)
{
    int i = 0;
    const IntSet& uvids = model->uvs( midx);
    for ( int uvid : uvids)
    {
        uvmap[uvid] = ++i;  // Pre-increment (.obj lists start at 1)
        const cv::Vec2f& uv = model->uv(midx, uvid);
        os << "vt\t" << uv[0] << " " << uv[1] << " " << 0.0 << std::endl;
    }   // end for
    os << std::endl;
}   // end writeMaterialUVs


void writeMaterialFaces( std::ostream& os, const ObjModel* model, int midx, const IIMap& vvmap, const IIMap& uvmap, IntSet& rfids)
{
    // Vertex indices are +1 because .obj vertex list starts at 1.
    const IntSet& mfids = model->materialFaceIds( midx);
    for ( int fid : mfids)
    {
        rfids.erase(fid);
        const int* vidxs = model->fvidxs(fid);
        const int* fuvs = model->faceUVs(fid);
        os << "f\t" << vvmap.at(vidxs[0]) << "/" << uvmap.at(fuvs[0]) << " "
                    << vvmap.at(vidxs[1]) << "/" << uvmap.at(fuvs[1]) << " "
                    << vvmap.at(vidxs[2]) << "/" << uvmap.at(fuvs[2]) << std::endl;
    }   // end for
}   // end writeMaterialFaces

}   // end namespace


// protected
bool OBJExporter::doSave( const ObjModel* model, const std::string& fname)
{
    const std::string matfile = boost::filesystem::path(fname).replace_extension("mtl").string();
    std::string err = writeMaterialFile( model, matfile);
    if ( !err.empty())
    {
        setErr( "Unable to write OBJ .mtl file! " + err);
        return false;
    }   // end if

    std::ofstream ofs;
    try
    {
        ofs.open( fname.c_str(), std::ios::out);
        ofs << "# Wavefront OBJ file produced by RModelIO (https://github.com/richeytastic/rModelIO)" << std::endl;
        ofs << std::endl;
        ofs << "mtllib " << boost::filesystem::path(matfile).filename().string() << std::endl;
        ofs << std::endl;

        ofs << "# Model has " << model->numVtxs() << " vertices" << std::endl;

        IIMap vvmap;
        writeVertices( ofs, model, vvmap);

        ofs << std::endl;

        IntSet remfids; // Will hold face IDs not associated with a material
        const IntSet& fids = model->faces();
        for ( int fid : fids)
            remfids.insert(fid);

        int pmid = 0;   // Pseudo material ID if required.
        const IntSet& mids = model->materialIds();
        for ( int mid : mids)
        {
            const std::string mname = getMaterialName( fname, mid);
            ofs << "# " << model->uvs(mid).size() << " UV coordinates on material '" << mname << "'" << std::endl;
            IIMap uvmap;
            writeMaterialUVs( ofs, model, mid, uvmap);
            ofs << std::endl;
            ofs << "# Mesh '" << mname << "' with " << model->materialFaceIds(mid).size() << " faces" << std::endl;
            ofs << "usemtl " << mname << std::endl;
            writeMaterialFaces( ofs, model, mid, vvmap, uvmap, remfids);
            pmid = mid+1;
        }   // end for

        ofs << std::endl;
        // Not all faces accounted for in materials, so write out the remainder without texture coordinates.
        if ( !remfids.empty())
        {
            const std::string mname = getMaterialName( fname, pmid);
            ofs << "# Mesh '" << mname << "' with " << remfids.size() << " faces" << std::endl;
            ofs << "usemtl " << mname << std::endl;
            for ( int fid : remfids)
            {
                const int* vidxs = model->fvidxs(fid);
                ofs << "f\t" << vvmap.at(vidxs[0]) << " " << vvmap.at(vidxs[1]) << " " << vvmap.at(vidxs[2]) << std::endl;
            }   // end for
        }   // end if

        ofs << std::endl;
        ofs.close();
    }   // end try
    catch ( const std::exception &e)
    {
        err = e.what();
    }   // end catch

    bool success = true;
    if ( !err.empty())
    {
        setErr( "Unable to write OBJ file! : " + err);
        success = false;
    }   // end if
    return success;
}   // end doSave


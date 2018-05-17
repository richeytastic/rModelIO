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


bool equal( const cv::Mat a, const cv::Mat b)
{
    if ( a.size() != b.size() || a.type() != b.type())
        return false;
    cv::Mat diff = a - b;
    return cv::countNonZero(diff.reshape(1)) == 0;
}   // end equal


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
        size_t nfaces = 0;
        const IntSet& mids = model->getMaterialIds();
        for ( int mid : mids)
        {
            nfaces += model->getMaterialFaceIds(mid).size();
            const std::string matname = getMaterialName( fname, mid);
            ofs << "newmtl " << matname << std::endl;
            ofs << "illum 1" << std::endl;

            const std::vector<cv::Mat>& ambient = model->getMaterialAmbient(mid);
            const std::vector<cv::Mat>& diffuse = model->getMaterialDiffuse(mid);
            const std::vector<cv::Mat>& specular = model->getMaterialSpecular(mid);

            int i = 0;
            cv::Mat tx;
            if ( !diffuse.empty())
            {
                std::ostringstream oss;
                oss << matname << i << ".png";
                ofs << "map_Kd " << oss.str() << std::endl;
                const std::string imgfile = (ppath / oss.str()).string();
                cv::imwrite( imgfile, diffuse[0]);
                tx = diffuse[0];
            }   // end if

            if ( !ambient.empty())
            {
                ofs << "map_Ka ";
                if ( !equal( ambient[0], tx))
                {
                    i++;
                    std::ostringstream oss;
                    oss << matname << i << ".png";
                    const std::string imgfile = (ppath / oss.str()).string();
                    cv::imwrite( imgfile, ambient[0]);
                    tx = ambient[0];
                }   // end if
                ofs << matname << i << ".png" << std::endl;
            }   // end if

            if ( !specular.empty())
            {
                ofs << "map_Ks ";
                if ( !equal( specular[0], tx))
                {
                    i++;
                    std::ostringstream oss;
                    oss << matname << i << ".png";
                    const std::string imgfile = (ppath / oss.str()).string();
                    cv::imwrite( imgfile, specular[0]);
                }   // end if
                ofs << matname << i << ".png" << std::endl;
            }   // end if

            ofs << std::endl;
            pmid = mid+1;
        }   // end foreach

        // Do we need an extra 'pseudo' material?
        assert( nfaces <= model->getNumFaces());
        if ( nfaces < model->getNumFaces())
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


typedef std::unordered_map<int,int> IntIntMap;

int writeVertices( std::ostream& os, const ObjModel* model, IntIntMap& obj2FileVids)
{
    int i = 0;
    const IntSet& vidxs = model->getVertexIds();
    for ( int vidx : vidxs)
    {
        obj2FileVids[vidx] = ++i;   // Pre-increment (.obj vertex list starts at 1)
        const cv::Vec3f& v = model->vtx(vidx);
        os << "v\t" << v[0] << " " << v[1] << " " << v[2] << std::endl;
    }   // end foreach
    return i;
}   // end writeVertices


void writeMaterialUVs( std::ostream& os, const ObjModel* model, int midx, IntIntMap& uvmap)
{
    int i = 0;
    const IntSet& uvids = model->getUVs( midx);
    for ( int uvid : uvids)
    {
        uvmap[uvid] = ++i;
        const cv::Vec2f& uv = model->uv(midx, uvid);
        os << "vt\t" << uv[0] << " " << uv[1] << " " << 0.0 << std::endl;
    }   // end foreach
    os << std::endl;
}   // end writeMaterialUVs


void writeMaterialFaces( std::ostream& os, const ObjModel* model, int midx,
                         const IntIntMap& vmap, const IntIntMap& uvmap, IntSet& rfids)
{
    const IntSet& mfids = model->getMaterialFaceIds( midx);
    for ( int fid : mfids)
    {
        rfids.erase(fid);
        const int* vidxs = model->getFaceVertices(fid);
        const int* fuvs = model->getFaceUVs(fid);
        os << "f\t" << vmap.at(vidxs[0]) << "/" << uvmap.at(fuvs[0]) << " "
                    << vmap.at(vidxs[1]) << "/" << uvmap.at(fuvs[1]) << " "
                    << vmap.at(vidxs[2]) << "/" << uvmap.at(fuvs[2]) << std::endl;
    }   // end foreach
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

        ofs << "# Model has " << model->getNumVertices() << " vertices" << std::endl;
        IntIntMap vmap;
        writeVertices( ofs, model, vmap);

        ofs << std::endl;

        int pmid = 0;   // Pseudo material ID if required.
        IntSet remfids = model->getFaceIds();   // Will hold face IDs not associated with a material
        const IntSet& mids = model->getMaterialIds();
        for ( int mid : mids)
        {
            const std::string mname = getMaterialName( fname, mid);
            ofs << "# " << model->getUVs(mid).size() << " UV coordinates on material '" << mname << "'" << std::endl;
            IntIntMap uvmap;
            writeMaterialUVs( ofs, model, mid, uvmap);
            ofs << std::endl;
            ofs << "# Mesh '" << mname << "' with " << model->getMaterialFaceIds(mid).size() << " faces" << std::endl;
            ofs << "usemtl " << mname << std::endl;
            writeMaterialFaces( ofs, model, mid, vmap, uvmap, remfids);
            pmid = mid+1;
        }   // end foreach

        ofs << std::endl;
        // Not all faces accounted for in materials, so write out the remainder without texture coordinates.
        if ( !remfids.empty())
        {
            const std::string mname = getMaterialName( fname, pmid);
            ofs << "# Mesh '" << mname << "' with " << remfids.size() << " faces" << std::endl;
            ofs << "usemtl " << mname << std::endl;
            for ( int fid : remfids)
            {
                const int* vidxs = model->getFaceVertices(fid);
                ofs << "f\t" << vmap.at(vidxs[0]) << " " << vmap.at(vidxs[1]) << " " << vmap.at(vidxs[2]) << std::endl;
            }   // end foreach
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


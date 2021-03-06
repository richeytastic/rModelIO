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

#include <IDTFExporter.h>
#include <ImageIO.h>   // RFeatures::saveAsTGA
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <boost/filesystem/operations.hpp>
using RModelIO::IDTFExporter;
using RFeatures::ObjModel;
using std::unordered_map;


// public
IDTFExporter::IDTFExporter( bool delOnDtor, bool m9)
    : RModelIO::ObjModelExporter(), _delOnDtor(delOnDtor), _media9(m9)
{
    addSupported( "idtf", "Intermediate Data Text Format");
}   // end ctor


// public
IDTFExporter::~IDTFExporter() { reset();}


// Remove saved files.
// private
void IDTFExporter::reset()
{
    static const std::string istr = "[INFO] RModelIO::IDTFExporter::reset: ";
    if ( _delOnDtor)
    {
        using namespace boost::filesystem;
        path ffile( _idtffile);
        if ( exists( ffile) && is_regular_file( ffile))
        {
            remove( ffile);
            std::cerr << istr << "Removed " << ffile << std::endl;
        }   // end if

        for ( const std::string& tgafile : _tgafiles)
        {
            path ifile( tgafile);
            if ( exists( ifile) && is_regular_file(ifile))
            {
                remove( ifile);
                std::cerr << istr << "Removed " << ifile << std::endl;
            }   // end if
        }   // end foreach
    }   // end _delOnDtor

    _idtffile = "";
    _tgafiles.clear();
}   // end reset


namespace {

struct TB {
    TB(int ntabs=0) : n(ntabs) {}
    int n;
};  // end struct

struct NL {
    NL(int nNewLines=1) : n(std::max(1,nNewLines)) {} // Always at least one new line
    int n;
};  // end struct

std::ostream& operator<<( std::ostream& os, const TB& t)
{
    for ( int i = 0; i < t.n; ++i)
        os << "\t";
    return os;
}   // end operator<<

std::ostream& operator<<( std::ostream& os, const NL& nl)
{
    for ( int i = 0; i < nl.n; ++i)
        os << std::endl;
    return os;
}   // end operator<<


void nodeGroup( std::ostream& os)
{
    TB t(1), tt(2), ttt(3), tttt(4);
    NL n(1);
    os << "NODE \"GROUP\" {" << n;
    os << t << "NODE_NAME \"ModelGroup\"" << n;
    os << t << "PARENT_LIST {" << n;
    os << tt << "PARENT_COUNT 1" << n;
    os << tt << "PARENT 0 {" << n;
    os << ttt << "PARENT_NAME \"<NULL>\"" << n;
    os << ttt << "PARENT_TM {" << n;
    os << tttt << "1.000000 0.000000 0.000000 0.000000" << n;
    os << tttt << "0.000000 1.000000 0.000000 0.000000" << n;
    os << tttt << "0.000000 0.000000 1.000000 0.000000" << n;
    os << tttt << "0.000000 0.000000 0.000000 1.000000" << n;
    os << ttt << "}" << n;  // end PARENT_TM
    os << tt << "}" << n;  // end PARENT 0
    os << t << "}" << n;  // end PARENT_LIST
    os << "}" << n << n; // end NODE "GROUP"
}   // end nodeGroup


void nodeModel( std::ostream& os, int meshID)
{
    TB t(1), tt(2), ttt(3), tttt(4);
    NL n(1);
    os << "NODE \"MODEL\" {" << n;
    os << t << "NODE_NAME \"Mesh" << meshID << "\"" << n;
    os << t << "PARENT_LIST {" << n;
    os << tt << "PARENT_COUNT 1" << n;
    os << tt << "PARENT 0 {" << n;
    os << ttt << "PARENT_NAME \"ModelGroup\"" << n;
    os << ttt << "PARENT_TM {" << n;
    os << tttt << "1.000000 0.000000 0.000000 0.000000" << n;
    os << tttt << "0.000000 1.000000 0.000000 0.000000" << n;
    os << tttt << "0.000000 0.000000 1.000000 0.000000" << n;
    os << tttt << "0.000000 0.000000 0.000000 1.000000" << n;
    os << ttt << "}" << n;  // end PARENT_TM
    os << tt << "}" << n;  // end PARENT 0
    os << t << "}" << n;  // end PARENT_LIST
    os << t << "RESOURCE_NAME \"Mesh" << meshID << "\"" << n;
    os << "}" << n << n; // end NODE "MODEL"
}   // end nodeModel


void nodeLight( std::ostream& os, int lightID, const cv::Vec3f& pos=cv::Vec3f(0,0,0))
{
    TB t(1), tt(2), ttt(3), tttt(4);
    NL n(1);
    os << "NODE \"LIGHT\" {" << n;
    os << t << "NODE_NAME \"Light" << lightID << "\"" << n;
    os << t << "PARENT_LIST {" << n;
    os << tt << "PARENT_COUNT 1" << n;
    os << tt << "PARENT 0 {" << n;
    os << ttt << "PARENT_NAME \"<NULL>\"" << n;
    os << ttt << "PARENT_TM {" << n;
    os << tttt << "1.000000 0.000000 0.000000 " << std::fixed << std::setprecision(6) << pos[0] << n;
    os << tttt << "0.000000 1.000000 0.000000 " << std::fixed << std::setprecision(6) << pos[1] << n;
    os << tttt << "0.000000 0.000000 1.000000 " << std::fixed << std::setprecision(6) << pos[2] << n;
    os << tttt << "0.000000 0.000000 0.000000 1.000000" << n;
    os << ttt << "}" << n;  // end PARENT_TM
    os << tt << "}" << n;  // end PARENT 0
    os << t << "}" << n;  // end PARENT_LIST
    os << t << "RESOURCE_NAME \"AmbientLight" << lightID << "\"" << n;
    os << "}" << n << n; // end NODE "MODEL"
}   // end nodeLight


void resourceLight( std::ostream& os, int lightID)
{
    TB t(1), tt(2), ttt(3), tttt(4);
    NL n(1);
    os << "RESOURCE_LIST \"LIGHT\" {" << n;
    os << t << "RESOURCE_COUNT 1" << n;
    os << t << "RESOURCE 0 {" << n;
    os << tt << "RESOURCE_NAME \"AmbientLight" << lightID << "\"" << n;
    os << tt << "LIGHT_TYPE \"AMBIENT\"" << n;
    os << tt << "LIGHT_COLOR 1.000000 1.000000 1.000000" << n;
    os << tt << "LIGHT_ATTENUATION 1.000000 0.000000 0.000000" << n;
    os << tt << "LIGHT_INTENSITY 1.000000" << n;
    os << t << "}" << n;
    os << "}" << n << n;
}   // end resourceLight


void resourceListShader( std::ostream& os, int nmesh, bool hasTX)
{
    TB t(1), tt(2), ttt(3), tttt(4);
    NL n(1);
    os << "RESOURCE_LIST \"SHADER\" {" << n;
    os << t << "RESOURCE_COUNT " << nmesh << n;
    for ( int i = 0; i < nmesh; ++i)    // One to one mapping of shaders to texture maps
    {
        os << t << "RESOURCE " << i << " {" << n;
        os << tt << "RESOURCE_NAME \"Shader" << i << "\"" << n;
        os << tt << "SHADER_MATERIAL_NAME \"Material0\"" << n;  // All shaders reference same material
        os << tt << "SHADER_ACTIVE_TEXTURE_COUNT " << ( hasTX ? 1 : 0) << n;
        if ( hasTX)
        {
            os << tt << "SHADER_TEXTURE_LAYER_LIST {" << n;
            os << ttt << "TEXTURE_LAYER 0 {" << n;
            os << tttt << "TEXTURE_NAME \"Texture" << i << "\"" << n;
            os << ttt << "}" << n;  // end TEXTURE_LAYER 0
            os << tt << "}" << n;  // end SHADER_TEXTURE_LAYER_LIST
        }   // end if
        os << t << "}" << n;  // end RESOURCE i
    }   // end for
    os << "}" << n << n; // end RESOURCE_LIST "SHADER"
}   // end resourceListShader


void modifierShading( std::ostream& os, int meshID)
{
    TB t(1), tt(2), ttt(3), tttt(4), ttttt(5);
    NL n(1);
    os << "MODIFIER \"SHADING\" {" << n;
    os << t << "MODIFIER_NAME \"Mesh" << meshID << "\"" << n;
    os << t << "PARAMETERS {" << n;
    os << tt << "SHADER_LIST_COUNT 1" << n;
    os << tt << "SHADING_GROUP {" << n;
    os << ttt << "SHADER_LIST 0 {" << n;
    os << tttt << "SHADER_COUNT 1" << n;
    os << tttt << "SHADER_NAME_LIST {" << n;
    os << ttttt << "SHADER 0 NAME: \"Shader" << meshID << "\"" << n;
    os << tttt << "}" << n; // end SHADER_NAME_LIST
    os << ttt << "}" << n; // end SHADER_LIST
    os << tt << "}" << n; // end SHADING_GROUP
    os << t << "}" << n; // end PARAMETERS
    os << "}" << n; // end MODIFIER "SHADING"
}   // end modifierShading


void resourceListMaterial( std::ostream& os)
{
    TB t(1), tt(2);
    NL n(1);
	os << "RESOURCE_LIST \"MATERIAL\" {" << n;
    os << t << "RESOURCE_COUNT 1" << n;
    os << t << "RESOURCE 0 {" << n;
	os << tt << "RESOURCE_NAME \"Material0\"" << n;
	os << tt << "MATERIAL_AMBIENT 1.0 1.0 1.0" << n;
	os << tt << "MATERIAL_DIFFUSE 1.0 1.0 1.0" << n;
	os << tt << "MATERIAL_SPECULAR 0.0 0.0 0.0" << n;
    os << tt << "MATERIAL_EMISSIVE 0.0 0.0 0.0" << n;
	os << tt << "MATERIAL_REFLECTIVITY 0.000000" << n;
	os << tt << "MATERIAL_OPACITY 1.000000" << n;
	os << t << "}" << n; // end RESOURCE 0
	os << "}" << n << n; // end RESOURCE_LIST "MATERIAL"
}   // end resourceListMaterial


void resourceListTexture( std::ostream& os, const std::vector<std::pair<int, std::string> >& mtf)
{
    if ( mtf.empty())
        return;
    TB t(1), tt(2);
    NL n(1);
    os << "RESOURCE_LIST \"TEXTURE\" {" << n;
	os << t << "RESOURCE_COUNT " << mtf.size() << n;
    for ( size_t i = 0; i < mtf.size(); ++i)
    {
        os << t << "RESOURCE " << i << " {" << n;
        os << tt << "RESOURCE_NAME \"Texture" << i << "\"" << n;
        os << tt << "TEXTURE_PATH \"" << mtf[i].second << "\"" << n;
        os << t << "}" << n;    // end RESOURCE i
    }   // end for
	os << "}" << n << n; // end RESOURCE_LIST "TEXTURE"
}   // end resourceListTexture



struct ModelResource
{
    // matID >= 0 if the model has materials.
    ModelResource( const ObjModel* model, bool media9, int matID) : _model(model), _media9(media9)
    {
        // Get repeatable sequence of face IDs and the unique set of texture coords for the material
        const IntSet* fids;
        if ( matID < 0)
            fids = &_model->faces();
        else
            fids = &_model->materialFaceIds(matID);

        _fidv.resize( fids->size());
        int k = 0;
        int vid;
        for ( int fid : *fids)
        {
            _fidv[k++] = fid;
            if ( _model->faceMaterialId(fid) >= 0)
            {
                const int* uvids = _model->faceUVs(fid);
                for ( int i = 0; i < 3; ++i)
                {
                    // Only want to store unique UV offsets.
                    const int key = uvids[i];
                    if ( _uvmap.count(key) == 0)
                    {
                        _uvmap[key] = (int)_uvlist.size();  // Map the array index
                        _uvlist.push_back( &_model->uv( matID, key));
                    }   // end if
                }   // end for
            }   // end if

            const int* vidxs = _model->fvidxs(fid);
            for ( int i = 0; i < 3; ++i)
            {
                vid = vidxs[i];
                if ( _vmap.count(vid) == 0)
                {
                    _vmap[vid] = (int)_vidv.size();  // For mapping to index of this node's list from a ObjPoly.vindices array.
                    _vidv.push_back(vid);
                }   // end if
            }   // end for
        }   // end for
    }   // end ctor

    void writeMesh( std::ostream& os) const
    {
        const bool hasTX = _model->numMats() > 0;
        TB tt(2);
        NL n(1);
        writeHeader(os);
        writeShadingDescriptionList(os);
        writeFacePositionList(os);
        writeFaceNormalList(os);
        writeFaceShadingList(os);
        if ( hasTX)
            writeFaceTextureCoordList(os);
        writePositionList(os);
        writeNormalList(os);
        if ( hasTX)
            writeTextureCoordList(os);
    }   // end writeMesh

private:
    const ObjModel* _model;
    const bool _media9;
    std::vector<int> _fidv;          // Predictable seq. of face IDs
    std::vector<int> _vidv;          // Predictable seq. of vertex IDs
    unordered_map<int,int> _vmap;    // ObjModel vertexID --> MODEL_POSITION_LIST index
    unordered_map<int, int> _uvmap;  // ObjModel uvID --> _uvlist index
    std::vector<const cv::Vec2f*> _uvlist;  // List of texture UVs to output in MODEL_TEXTURE_COORD_LIST


    void writeHeader( std::ostream& os) const
    {
        TB ttt(3);
        NL n(1);
        os << ttt << "FACE_COUNT " << _fidv.size() << n;
        os << ttt << "MODEL_POSITION_COUNT " << _vidv.size() << n;
        os << ttt << "MODEL_NORMAL_COUNT " << (_fidv.size() * 3) << n;
        os << ttt << "MODEL_DIFFUSE_COLOR_COUNT 0" << n;
        os << ttt << "MODEL_SPECULAR_COLOR_COUNT 0" << n;
        os << ttt << "MODEL_TEXTURE_COORD_COUNT " << _uvmap.size() << n;
        os << ttt << "MODEL_BONE_COUNT 0" << n; // No skeleton
        os << ttt << "MODEL_SHADING_COUNT 1" << n;
    }   // end writeHeader


    void writeShadingDescriptionList( std::ostream& os) const
    {
        const bool hasTX = _model->numMats() > 0;
        TB ttt(3), tttt(4), ttttt(5), tttttt(6);
        NL n(1);
        os << ttt << "MODEL_SHADING_DESCRIPTION_LIST {" << n;
        os << tttt << "SHADING_DESCRIPTION 0 {" << n;
        os << ttttt << "TEXTURE_LAYER_COUNT " << (hasTX ? 1 : 0) << n;    // No multi-texturing!
        if ( hasTX)
        {
            os << ttttt << "TEXTURE_COORD_DIMENSION_LIST {" << n;
            os << tttttt << "TEXTURE_LAYER 0 DIMENSION: 2" << n;    // 2D texture map
            os << ttttt << "}" << n; // end TEXTURE_COORD_DIMENSION_LIST
        }   // end if
        os << ttttt << "SHADER_ID 0" << n;
        os << tttt << "}" << n; // end SHADING_DESCRIPTION
        os << ttt << "}" << n;  // end MODEL_SHADING_DESCRIPTION_LIST
    }   // end writeShadingDescriptionList


    // For each face, record the vertex IDs it's composed of - these must be the
    // index of the vertices as given in MODEL_POSITION_LIST, so map using vmap.
    // Collect all face indices into a repeatable list for subsequent nodes (texture)
    void writeFacePositionList( std::ostream& os) const
    {
        os << TB(3) << "MESH_FACE_POSITION_LIST {" << NL(1);
        TB ttt(3), tttt(4);
        NL n(1);
        for ( int fid : _fidv)
        {
            const int* vidxs = _model->fvidxs( fid);
            os << tttt << _vmap.at(vidxs[0]) << " " << _vmap.at(vidxs[1]) << " " << _vmap.at(vidxs[2]) << n;
        }   // end for
        os << ttt << "}" << n;
    }   // end writeFacePositionList


    void writeFaceNormalList( std::ostream& os) const
    {
        os << TB(3) << "MESH_FACE_NORMAL_LIST {" << NL(1);
        TB ttt(3), tttt(4);
        NL n(1);
        int i = 0;
        for ( size_t j = 0; j < _fidv.size(); ++j, i += 3)
            os << tttt << i << " " << (i+1) << " " << (i+2) << n;
        os << ttt << "}" << n;
    }   // end writeFaceNormalList


    // For each face, record the shader ID (as stored in this file)
    void writeFaceShadingList( std::ostream& os) const
    {
        TB ttt(3), tttt(4);
        NL n(1);
        os << ttt << "MESH_FACE_SHADING_LIST {" << n;
        for ( size_t j = 0; j < _fidv.size(); ++j)
            os << tttt << 0 << n;
        os << ttt << "}" << n;  // end MESH_FACE_SHADING_LIST
    }   // end writeFaceShadingList


    int getUVListIndex( int faceId, int uvOrderIndex/*[0,2]*/) const
    {
        assert( _model->faceMaterialId(faceId) >= 0);
        const int uvid = _model->faceUVs(faceId)[uvOrderIndex];
        return _uvmap.at( uvid);
    }   // end getUVListIndex


    // Write out texture coordinates if ObjModel has materials.
    void writeFaceTextureCoordList( std::ostream& os) const
    {
        TB ttt(3), tttt(4), ttttt(5);
        NL n(1);
        const int nf = int(_fidv.size());
        os << ttt << "MESH_FACE_TEXTURE_COORD_LIST {" << n;
        for ( int i = 0; i < nf; ++i)
        {
            const int fid = _fidv[i];
            const int uv0 = getUVListIndex( fid, 0);
            const int uv1 = getUVListIndex( fid, 1);
            const int uv2 = getUVListIndex( fid, 2);
            os << tttt << "FACE " << i << " {" << n;
            os << ttttt << "TEXTURE_LAYER 0 TEX_COORD: " << std::fixed << std::setprecision(6) << uv0 << " " << uv1 << " " << uv2 << n;
            os << tttt << "}" << n; // end FACE i
        }   // end foreach
        os << ttt << "}" << n;  // end MESH_FACE_TEXTURE_COORD_LIST
    }   // end writeFaceTextureCoordList


    // Output mesh positions (mapping the vertex ID to the position of the vertex in this list)
    void writePositionList( std::ostream& os) const
    {
        TB ttt(3), tttt(4);
        NL n(1);
        os << ttt << "MODEL_POSITION_LIST {" << n;
        os << std::fixed << std::setprecision(6);

        if ( _media9)
        {
            for ( int vid : _vidv)
            {
                const cv::Vec3f& v = _model->vtx(vid);
                os << tttt << v[0] << " " << -v[2] << " " << v[1] << n;
            }   // end for
        }   // end if
        else
        {
            for ( int vid : _vidv)
            {
                const cv::Vec3f& v = _model->vtx(vid);
                os << tttt << v[0] << " " << v[1] << " " << v[2] << n;
            }   // end for
        }   // end else

        os << ttt << "}" << n;  // end MODEL_POSITION_LIST
    }   // end writePositionList


    // vertex normals not used
    void writeNormalList( std::ostream& os) const
    {
        const cv::Vec3f nrm(0,0,0);
        TB ttt(3), tttt(4);
        NL n(1);
        os << ttt << "MODEL_NORMAL_LIST {" << n;
        os << std::fixed << std::setprecision(6);
        for ( size_t j = 0; j < _fidv.size(); ++j)
        {
            os << tttt << nrm[0] << " " << nrm[1] << " " << nrm[2] << n;
            os << tttt << nrm[0] << " " << nrm[1] << " " << nrm[2] << n;
            os << tttt << nrm[0] << " " << nrm[1] << " " << nrm[2] << n;
        }   // end for
        os << ttt << "}" << n;  // end MODEL_NORMAL_LIST
    }   // end writeNormalList


    void writeTextureCoordList( std::ostream& os) const
    {
        TB ttt(3), tttt(4);
        NL n(1);
        os << ttt << "MODEL_TEXTURE_COORD_LIST {" << n;
        os << std::fixed << std::setprecision(6);
        for ( const cv::Vec2f* uv : _uvlist)
            os << tttt << std::fixed << (*uv)[0] << " " << (*uv)[1] << " " << 0.0 << " " << 0.0 << n;
        os << ttt << "}" << n;  // end MODEL_TEXTURE_COORD_LIST
    }   // end writeTextureCoordList
};  // end struct



// Write the model data in IDTF format. Only vertex, face, and texture mapping info are stored.
std::string writeFile( const ObjModel* model, bool media9, const std::string& filename, const std::vector<std::pair<int, std::string> >& mtf)
{
    const int nTX = (int)mtf.size();
    const int nmesh = std::max(1,nTX);
    std::string errMsg;
    std::ofstream ofs;
    try
    {
        ofs.open( filename.c_str(), std::ios::out);

        TB t(1), tt(2);
        NL n(1);
        const std::string meshName("Model");

        // File header
        ofs << "FILE_FORMAT \"IDTF\"" << n;
        ofs << "FORMAT_VERSION 100" << n << n;

        nodeGroup( ofs);
        for ( int i = 0; i < nmesh; ++i)
            nodeModel( ofs, i);

        nodeLight( ofs, 1);
        resourceLight( ofs, 1);

        // Multi material models are defined as separate model resources under a single parent node.
        // Model resource list (1 element)
        ofs << "RESOURCE_LIST \"MODEL\" {" << n;
        ofs << t << "RESOURCE_COUNT " << nmesh << n;

        for ( int i = 0; i < nmesh; ++i)
        {
            ofs << t << "RESOURCE " << i << " {" << n;
            ofs << tt << "RESOURCE_NAME \"Mesh" << i << "\"" << n;
            ofs << tt << "MODEL_TYPE \"MESH\"" << n;
            ofs << tt << "MESH {" << n;
            // meshID is the material ID if there's at least one material on the object
            const int matID = nTX > 0 ? mtf[i].first : -1;
            const ModelResource modelResource( model, media9, matID);
            modelResource.writeMesh( ofs);
            ofs << tt << "}" << n;    // end MESH
            ofs << t << "}" << n;    // end RESOURCE
        }   // end for

        ofs << "}" << n << n;

        resourceListShader( ofs, nmesh, nTX > 0);
        resourceListMaterial( ofs);
        resourceListTexture( ofs, mtf);

        // Shading modifiers
        for ( int i = 0; i < nmesh; ++i)
            modifierShading( ofs, i);

        ofs.close();
    }   // end try
    catch ( const std::exception &e)
    {
        errMsg = e.what();
    }   // end catch

    return errMsg;
}   // end writeFile

}   // end namespace



// protected
bool IDTFExporter::doSave( const ObjModel& inmodel, const std::string& filename)
{
    reset();
    // Need to set all the texture map filenames (if present) and save out the textures.
    // Image files are saved adjacent to the model.
    using Path = boost::filesystem::path;
    const Path mpath( filename);
    Path tpath = mpath.parent_path();  // Directory model is being saved in
    tpath /= mpath.stem();             // Use the stem of the save filename as the basis for the texture filenames

    std::vector<std::pair<int, std::string> > mtf;  // Associate the texture filenames with the material ID
    const ObjModel* model = nullptr;
    ObjModel::Ptr nmodel;
    if ( inmodel.numMats() <= 1)
        model = &inmodel;
    else
    {
        std::cerr << "[STATUS] RModelIO::IDTFExporter::doSave: Multi-material model being copied to single material model for export" << std::endl;
        nmodel = inmodel.deepCopy( true);
        nmodel->mergeMaterials();
        model = nmodel.get();
    }   // end else

    const IntSet& mids = model->materialIds();
    for ( int mid : mids)
    {
        // Texture needs to be output in TGA format for conversion to the IDTF intermediate format.
        cv::Mat tx = model->texture(mid);
        if ( tx.empty())
        {
            std::ostringstream eoss;
            eoss << "[ERROR] RModelIO::IDTFExporter::doSave: Material " << mid << " has no texture!";
            setErr(eoss.str());
            return false;
        }   // end else

        std::ostringstream oss;
        oss << tpath.string() << "_M" << mid << ".tga";
        const std::string tgafname = oss.str();
        _tgafiles.push_back( tgafname);    // Record to delete on destruction
        if ( !RFeatures::saveTGA( tx, tgafname))
            return false;
        mtf.push_back( std::pair<int, std::string>( mid, tgafname));
    }   // end foreach

    _idtffile = filename;
    const std::string errMsg = writeFile( model, _media9, filename, mtf);
    if ( !errMsg.empty())
        setErr( "Unable to write IDTF text file! : " + errMsg);
    return errMsg.empty();
}   // end doSave


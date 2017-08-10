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

#include <IDTFExporter.h>
#include <ImageIO.h>   // RFeatures::saveAsTGA
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>
using RModelIO::IDTFExporter;
using RFeatures::ObjModel;


// public
IDTFExporter::IDTFExporter( const ObjModel::Ptr mod, bool delOnDtor)
    : RModelIO::ObjModelExporter(mod), _delOnDtor(delOnDtor)
{
    addSupported( "idtf", "Intermediate Data Text Format");
}   // end ctor


// public
IDTFExporter::~IDTFExporter()
{
    reset();
}   // end dtor


// Remove saved files.
// private
void IDTFExporter::reset()
{
    if ( _delOnDtor)
    {
        using namespace boost::filesystem;
        path ffile( _idtffile);
        if ( exists( ffile) && is_regular_file( ffile))
        {
            remove( ffile);
            std::cout << "Removed IDTF file" << std::endl;
        }   // end if

        BOOST_FOREACH ( const std::string& tgafile, _tgafiles)
        {
            path ifile( tgafile);
            if ( exists( ifile) && is_regular_file(ifile))
            {
                remove( ifile);
                std::cout << "Removed tga file (IDTF conversion)" << std::endl;
            }   // end if
        }   // end foreach
    }   // end _delOnDtor

    _idtffile = "";
    _tgafiles.clear();
}   // end reset


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


void nodeModel( std::ostream& os, const std::string& resourceName)
{
    TB t(1), tt(2), ttt(3), tttt(4);
    NL n(1);
    os << "NODE \"MODEL\" {" << n;
    os << t << "NODE_NAME \"Model01\"" << n;
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
    os << t << "RESOURCE_NAME \"" << resourceName << "\"" << n;
    os << "}" << n << n; // end NODE "MODEL"
}   // end nodeModel


void nodeLight( std::ostream& os, int lightID, const cv::Vec3f& pos)
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
    os << t << "RESOURCE_NAME \"PointLight" << lightID << "\"" << n;
    os << "}" << n << n; // end NODE "MODEL"
}   // end nodeLight


void resourceLight( std::ostream& os, int lightID)
{
    TB t(1), tt(2), ttt(3), tttt(4);
    NL n(1);
    os << "RESOURCE_LIST \"LIGHT\" {" << n;
    os << t << "RESOURCE_COUNT 1" << n;
    os << t << "RESOURCE 0 {" << n;
    os << tt << "RESOURCE_NAME \"PointLight" << lightID << "\"" << n;
    os << tt << "LIGHT_TYPE \"POINT\"" << n;
    os << tt << "LIGHT_COLOR 1.000000 1.000000 1.000000" << n;
    os << tt << "LIGHT_ATTENUATION 1.000000 0.000000 0.000000" << n;
    os << tt << "LIGHT_INTENSITY 1.000000" << n;
    os << t << "}" << n;
    os << "}" << n << n;
}   // end resourceLight


void resourceListShader( std::ostream& os, size_t nTX)
{
    TB t(1), tt(2), ttt(3), tttt(4);
    NL n(1);
    const bool hasTX = nTX > 0;
    nTX = std::max(size_t(1),nTX);    // Need at least one shader!
    os << "RESOURCE_LIST \"SHADER\" {" << n;
    os << t << "RESOURCE_COUNT " << nTX << n;

    for ( size_t i = 0; i < nTX; ++i)    // One to one mapping of shaders to texture maps
    {
        os << t << "RESOURCE " << i << " {" << n;
        os << tt << "RESOURCE_NAME \"ModelShader" << i << "\"" << n;
        os << tt << "SHADER_MATERIAL_NAME \"Mat01\"" << n;  // All shaders reference same material
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


void modifierShading( std::ostream& os)
{
    TB t(1), tt(2), ttt(3), tttt(4), ttttt(5);
    NL n(1);
    os << "MODIFIER \"SHADING\" {" << n;
    os << t << "MODIFIER_NAME \"Model01\"" << n;
    os << t << "PARAMETERS {" << n;
    os << tt << "SHADER_LIST_COUNT " << 1 << n;
    os << tt << "SHADER_LIST_LIST {" << n;
    os << ttt << "SHADER_LIST 0 {" << n;
    os << tttt << "SHADER_COUNT 1" << n;
    os << tttt << "SHADER_NAME_LIST {" << n;
    os << ttttt << "SHADER 0 NAME: \"ModelShader0\"" << n;
    os << tttt << "}" << n; // end SHADER_NAME_LIST
    os << ttt << "}" << n; // end SHADER_LIST
    os << tt << "}" << n; // end SHADER_LIST_LIST
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
	os << tt << "RESOURCE_NAME \"Mat01\"" << n;
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
    ModelResource( const ObjModel::Ptr model, const boost::unordered_map<int,int>& mmap)
        : _model(model), _mmap(mmap)
    {
        // Get repeatable sequence of face IDs and the unique set of texture coords across all materials
        const IntSet& fids = _model->getFaceIds();
        _fidv.resize( fids.size());
        int k = 0;
        BOOST_FOREACH ( int fid, fids)
        {
            _fidv[k++] = fid;
            const int64_t mid = _model->getFaceMaterialId(fid);
            if ( mid >= 0)
            {
                const int* uvids = _model->getFaceUVs(fid);
                for ( int i = 0; i < 3; ++i)
                {
                    // Only want to store unique UV offsets.
                    const int64_t key = (mid << 32) | int64_t(uvids[i]);  // Who doesn't love a bit-shifted disjunction.
                    if ( _uvmap.count(key) == 0)
                    {
                        _uvmap[key] = (int)_uvlist.size();  // Hash the array index
                        _uvlist.push_back( &_model->uv( mid, uvids[i]));
                    }   // end if
                }   // end for
            }   // end if
        }   // end foreach

        k = 0;
        const IntSet& vids = _model->getVertexIds();
        _vidv.resize( vids.size());
        BOOST_FOREACH ( int vid, vids)
        {
            _vidv[k] = vid;
            _vmap[vid] = k++;  // For mapping to index of this node's list from a ObjPoly.vindices array.
        }   // end foreach
    }   // end ctor

    void writeMesh( std::ostream& os, const std::string& resourceName) const
    {
        TB tt(2);
        NL n(1);
        os << tt << "RESOURCE_NAME \"" << resourceName << "\"" << n;
        os << tt << "MODEL_TYPE \"MESH\"" << n;
        os << tt << "MESH {" << n;
        writeHeader(os);
        writeShadingDescriptionList(os);
        writeFacePositionList(os);
        writeFaceNormalList(os);
        writeFaceShadingList(os);
        writeFaceTextureCoordList(os);
        writePositionList(os);
        writeNormalList(os);
        writeTextureCoordList(os);
        os << tt << "}" << n;    // end MESH
    }   // end writeMesh

private:
    const ObjModel::Ptr _model;
    const boost::unordered_map<int,int>& _mmap; // ObjModel material ID --> shader list index
    std::vector<int> _fidv;                     // Predictable seq. of face IDs
    std::vector<int> _vidv;                     // Predictable seq. of vertex IDs
    boost::unordered_map<int,int> _vmap;        // ObjModel vertexID --> MODEL_POSITION_LIST index
    boost::unordered_map<int64_t, int> _uvmap;  // ObjModel mat and uvID --> _uvlist index
    std::vector<const cv::Vec2f*> _uvlist;      // List of texture UVs to output in MODEL_TEXTURE_COORD_LIST


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
        os << ttt << "MODEL_SHADING_COUNT " << int(std::max(size_t(1),_mmap.size())) << n;
    }   // end writeHeader


    void writeShadingDescriptionList( std::ostream& os) const
    {
        TB ttt(3), tttt(4), ttttt(5), tttttt(6);
        NL n(1);
        const bool hasTX = !_mmap.empty();
        const int nTX = std::max(size_t(1),_mmap.size());
        os << ttt << "MODEL_SHADING_DESCRIPTION_LIST {" << n;
        for ( size_t i = 0; i < nTX; ++i)
        {
            os << tttt << "SHADING_DESCRIPTION " << i << " {" << n;
            os << ttttt << "TEXTURE_LAYER_COUNT " << (hasTX ? 1 : 0) << n;    // No multi-texturing!
            if ( hasTX)
            {
                os << ttttt << "TEXTURE_COORD_DIMENSION_LIST {" << n;
                os << tttttt << "TEXTURE_LAYER 0 DIMENSION: 2" << n;    // 2D texture map
                os << ttttt << "}" << n; // end TEXTURE_COORD_DIMENSION_LIST
            }   // end if
            os << ttttt << "SHADER_ID 0" << n;  // Same ID so shading modifier targets all shaders
            os << tttt << "}" << n; // end SHADING_DESCRIPTION
        }   // end for
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
        BOOST_FOREACH ( int fid, _fidv)
        {
            const int* vidxs = _model->getFaceVertices( fid);
            os << tttt << _vmap.at(vidxs[0]) << " " << _vmap.at(vidxs[1]) << " " << _vmap.at(vidxs[2]) << n;
        }   // end foreach
        os << ttt << "}" << n;
    }   // end writeFacePositionList


    void writeFaceNormalList( std::ostream& os) const
    {
        os << TB(3) << "MESH_FACE_NORMAL_LIST {" << NL(1);
        TB ttt(3), tttt(4);
        NL n(1);
        int i = 0;
        BOOST_FOREACH ( int fid, _fidv)
        {
            os << tttt << i << " " << (i+1) << " " << (i+2) << n;
            i += 3;
        }   // end foreach
        os << ttt << "}" << n;
    }   // end writeFaceNormalList


    // For each face, record the shader ID (as stored in this file)
    void writeFaceShadingList( std::ostream& os) const
    {
        TB ttt(3), tttt(4);
        NL n(1);
        os << ttt << "MESH_FACE_SHADING_LIST {" << n;
        int mid;
        BOOST_FOREACH ( int fid, _fidv)
        {
            mid = _model->getFaceMaterialId(fid);
            os << tttt << (mid < 0 ? 0 : _mmap.at(mid)) << n;
        }   // end foreach
        os << ttt << "}" << n;  // end MESH_FACE_SHADING_LIST
    }   // end writeFaceShadingList


    int getUVListIndex( int faceId, int uvOrderIndex/*[0,2]*/) const
    {
        const int64_t mid = _model->getFaceMaterialId(faceId);
        assert( mid >= 0);
        const int64_t uvid = _model->getFaceUVs(faceId)[uvOrderIndex];
        return _uvmap.at( (mid << 32) | uvid);
    }   // end getUVListIndex

    // Write out texture coordinates if ObjModel has materials.
    void writeFaceTextureCoordList( std::ostream& os) const
    {
        if ( _mmap.empty())
            return;

        TB ttt(3), tttt(4), ttttt(5);
        NL n(1);
        int i = 0;
        os << ttt << "MESH_FACE_TEXTURE_COORD_LIST {" << n;
        BOOST_FOREACH ( int fid, _fidv)
        {
            // Faces that don't map to a material are ignored
            if ( _model->getFaceMaterialId( fid) >= 0)
            {
                const int uv0 = getUVListIndex( fid, 0);
                const int uv1 = getUVListIndex( fid, 1);
                const int uv2 = getUVListIndex( fid, 2);
                os << tttt << "FACE " << i << " {" << n;
                os << ttttt << "TEXTURE_LAYER 0 TEX_COORD: " << std::fixed << std::setprecision(6)
                                                             << uv0 << " " << uv1 << " " << uv2 << n;
                os << tttt << "}" << n; // end FACE i
            }   // end if
            i++;
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
        BOOST_FOREACH ( int vid, _vidv)
        {
            const cv::Vec3f& v = _model->vtx(vid);
            os << tttt << v[0] << " " << v[1] << " " << v[2] << n;
        }   // end foreach
        os << ttt << "}" << n;  // end MODEL_POSITION_LIST
    }   // end writePositionList


    // Record the vertex normals
    void writeNormalList( std::ostream& os) const
    {
        const cv::Vec3f nrm(0,0,0);
        TB ttt(3), tttt(4);
        NL n(1);
        os << ttt << "MODEL_NORMAL_LIST {" << n;
        os << std::fixed << std::setprecision(6);
        BOOST_FOREACH ( int fid, _fidv)
        {
            os << tttt << nrm[0] << " " << nrm[1] << " " << nrm[2] << n;
            os << tttt << nrm[0] << " " << nrm[1] << " " << nrm[2] << n;
            os << tttt << nrm[0] << " " << nrm[1] << " " << nrm[2] << n;
        }   // end foreach
        os << ttt << "}" << n;  // end MODEL_NORMAL_LIST
    }   // end writeNormalList


    void writeTextureCoordList( std::ostream& os) const
    {
        if ( _mmap.empty())
            return;

        TB ttt(3), tttt(4);
        NL n(1);
        os << ttt << "MODEL_TEXTURE_COORD_LIST {" << n;
        os << std::fixed << std::setprecision(6);
        BOOST_FOREACH ( const cv::Vec2f* uv, _uvlist)
            os << tttt << std::fixed << (*uv)[0] << " " << (*uv)[1] << " " << 0.0 << " " << 0.0 << n;
        os << ttt << "}" << n;  // end MODEL_TEXTURE_COORD_LIST
    }   // end writeTextureCoordList
};  // end struct



// Write the model data in IDTF format. Only vertex, face, and texture mapping info are stored.
std::string writeFile( const ObjModel::Ptr model, const std::string& filename, const std::vector<std::pair<int, std::string> >& mtf)
{
    const size_t nTX = mtf.size();
    boost::unordered_map<int,int> mmap; // For mapping ObjModel material ID to ascending material index as stored in file.
    for ( size_t i = 0; i < nTX; ++i)  // One to one mapping of shaders to texture maps
        mmap[mtf[i].first] = i;     // Usually this will be i-->i but possible that it won't be in future (if material ID values change)

    std::string errMsg;
    std::ofstream ofs;
    try
    {
        ofs.open( filename.c_str(), std::ios::out);

        TB t(1), tt(2);
        NL n(1);
        const std::string meshName("Model01");

        // File header
        ofs << "FILE_FORMAT \"IDTF\"" << n;
        ofs << "FORMAT_VERSION 100" << n << n;

        nodeModel( ofs, meshName);
        //nodeLight( ofs, 1, cv::Vec3f(0,0,50));
        //resourceLight( ofs, 1);

        // Model resource list (1 element)
        ofs << "RESOURCE_LIST \"MODEL\" {" << n;
        ofs << t << "RESOURCE_COUNT 1" << n;
        ofs << t << "RESOURCE 0 {" << n;
        const ModelResource modelResource( model, mmap);
        modelResource.writeMesh( ofs, meshName);
        ofs << t << "}" << n;
        ofs << "}" << n << n;

        // Shader resource list
        resourceListShader( ofs, nTX);
        resourceListMaterial( ofs);
        resourceListTexture( ofs, mtf);

        // Shading modifier
        modifierShading( ofs);

        ofs.close();
    }   // end try
    catch ( const std::exception &e)
    {
        errMsg = e.what();
    }   // end catch

    return errMsg;
}   // end writeFile



// protected
bool IDTFExporter::doSave( const std::string& filename)
{
    reset();
    // Need to set all the texture map filenames (if present) and save out the textures.
    // Image files are saved adjacent to the model.
    typedef boost::filesystem::path Path;
    const Path mpath( filename);
    Path tpath = mpath.parent_path();  // Directory model is being saved in
    tpath /= mpath.stem();             // Use the stem of the save filename as the basis for the texture filenames

    std::vector<std::pair<int, std::string> > mtf;  // Associate the texture filenames with the material ID
    const ObjModel::Ptr model = _model;
    const IntSet& mids = model->getMaterialIds();
    BOOST_FOREACH ( int mid, mids)
    {
        // Textures need to be output in TGA format for conversion to the IDTF intermediate format.
        const std::vector<cv::Mat>& ambient = model->getMaterialAmbient(mid);
        const std::vector<cv::Mat>& diffuse = model->getMaterialDiffuse(mid);
        const std::vector<cv::Mat>& specular = model->getMaterialSpecular(mid);
        cv::Mat tx;
        if ( !diffuse.empty())
            tx = diffuse[0];
        else if ( !ambient.empty())
            tx = ambient[0];
        else if ( !specular.empty())
            tx = specular[0];
        else
        {
            std::ostringstream eoss;
            eoss << "[ERROR] RModelIO::IDTFExporter::doSave: Material " << mid << " contains no texture maps!";
            setErr(eoss.str());
            assert( !diffuse.empty() || !ambient.empty() || !specular.empty());
            return false;
        }   // end else

        std::ostringstream oss;
        oss << tpath.string() << "_M" << mid << ".tga";
        const std::string tgafname = oss.str();
        _tgafiles.push_back( tgafname);    // Record to delete on destruction
        if ( !RFeatures::saveAsTGA( tx, tgafname))
            return false;
        mtf.push_back( std::pair<int, std::string>( mid, Path(tgafname).filename().string()));  // Store just the filename without path
    }   // end foreach

    _idtffile = filename;
    const std::string errMsg = writeFile( model, filename, mtf);
    if ( !errMsg.empty())
        setErr( "Unable to write IDTF text file! : " + errMsg);
    return errMsg.empty();
}   // end doSave


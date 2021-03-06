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

#include <AssetImporter.h>
#include <FeatureUtils.h>   // RFeatures
#include <FileIO.h>     // rlib
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/importerdesc.h>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
using RModelIO::AssetImporter;
using RFeatures::ObjModel;

namespace {

using uint = unsigned int;


cv::Mat loadImage( const boost::filesystem::path& ppath, const std::string& imgfile)
{
    boost::filesystem::path imgPath = ppath / imgfile;
    cv::Mat m;
    if ( boost::filesystem::exists( imgPath))
    {
        if ( !RFeatures::loadImage( imgPath.string(), m))
            std::cerr << "[ERROR] RFeatures::loadImage(" << imgPath.string() << "): FAILED!" << std::endl;
    }   // end if
    return m;
}   // loadImage


int loadImages( const boost::filesystem::path& ppath, const std::vector<std::string>& imgfiles, std::vector<cv::Mat>& imgs)
{
    imgs.clear();
    for ( const std::string& imgfile : imgfiles)
    {
        cv::Mat m = loadImage( ppath, imgfile);
        if ( m.empty())
        {
            imgs.clear();
            return -1;
        }   // end if
        else
            imgs.push_back(m);
    }   // end for
    return (int)imgs.size();
}   // end loadImages


// The ambient, diffuse, and specular texture files for a material
struct MaterialTextures
{
    // Set the texture filenames from the given material. Provide the directory
    // path to where the image files are located.
    MaterialTextures( const aiMaterial* mat, const boost::filesystem::path& p) : _ppath(p)
    {
        setTextureTypeFiles( mat, aiTextureType_AMBIENT, _ambient);
        setTextureTypeFiles( mat, aiTextureType_DIFFUSE, _diffuse);
        setTextureTypeFiles( mat, aiTextureType_SPECULAR, _specular);
    }   // end ctor

    // Load the ambient, diffuse, and specular texture maps returning
    // the number of each type loaded. Returns -1 on error loading.
    int loadAmbient() { return loadImages( _ppath, _ambient, _amats);}
    int loadDiffuse() { return loadImages( _ppath, _diffuse, _dmats);}
    int loadSpecular() { return loadImages( _ppath, _specular, _smats);}

    const std::vector<cv::Mat>& ambient() const { return _amats;}
    const std::vector<cv::Mat>& diffuse() const { return _dmats;}
    const std::vector<cv::Mat>& specular() const { return _smats;}

private:
    void setTextureTypeFiles( const aiMaterial* mat, aiTextureType txtype, std::vector<std::string>& imgfiles)
    {
        const int n = mat->GetTextureCount( txtype);
        for ( int i = 0; i < n; ++i)
        {
            aiString textureFile;
            mat->GetTexture( txtype, i, &textureFile);
            imgfiles.push_back( textureFile.C_Str());
        }   // end for
    }   // end setTextureTypeFiles

    boost::filesystem::path _ppath; // Parent path for location of texture image files

    // The filenames for the texture maps
    std::vector<std::string> _ambient;
    std::vector<std::string> _diffuse;
    std::vector<std::string> _specular;

    // The corresponding images
    std::vector<cv::Mat> _amats;
    std::vector<cv::Mat> _dmats;
    std::vector<cv::Mat> _smats;
};  // end struct


int setObjectFaces( const aiMesh* mesh, std::vector<int>& fids, int& nonTriangles, ObjModel::Ptr model)
{
    IntSet faceSet;
    const int nfaces = (int)mesh->mNumFaces;
    fids.resize( nfaces);

    int v0, v1, v2;
    int dupFaces = 0; // Count duplicate faces not added
    nonTriangles = 0; // Count number of polygons that aren't triangles
    const aiFace* aifaces = mesh->mFaces;
    for ( int i = 0; i < nfaces; ++i)
    {
        const aiFace& aiface = aifaces[i];
        if ( aiface.mNumIndices != 3)   // Not a triangle?
        {
            nonTriangles++;
            fids[i] = -1;
            continue;
        }   // end if

        const aiVector3D& av0 = mesh->mVertices[aiface.mIndices[0]];
        const aiVector3D& av1 = mesh->mVertices[aiface.mIndices[1]];
        const aiVector3D& av2 = mesh->mVertices[aiface.mIndices[2]];

#ifndef NDEBUG
        // All three vertices must be unique to make a triangle, or it's not necessary (and is counted as a duplicate).
        // This shouldn't ever happen if AssImp is doing its job properly.
        if ( av0 == av1 || av1 == av2 || av2 == av0)
        {
            std::cerr << "[ERROR] RModelIO::AssetImporter::setObjectFaces(): Triple of aiVector3D vertices are not all different!" << std::endl;
            fids[i] = -1;
            dupFaces++;
            continue;
        }   // end if

        v0 = model->addVertex( av0[0], av0[1], av0[2]);  // < 0 returned if can't be added (error)
        if ( v0 < 0)
        {
            std::cerr << "[ERROR] RModelIO::AssetImporter::setObjectFaces(): Unable to add 1st vertex " << cv::Vec3f( av0[0], av0[1], av0[2]) << std::endl;
            fids[i] = -1;
            continue;
        }   // end if

        v1 = model->addVertex( av1[0], av1[1], av1[2]);  // < 0 returned if can't be added (error)
        if ( v1 < 0)
        {
            std::cerr << "[ERROR] RModelIO::AssetImporter::setObjectFaces(): Unable to add 2nd vertex " << cv::Vec3f( av1[0], av1[1], av1[2]) << std::endl;
            fids[i] = -1;
            continue;
        }   // end if

        v2 = model->addVertex( av2[0], av2[1], av2[2]);  // < 0 returned if can't be added (error)
        if ( v2 < 0)
        {
            std::cerr << "[ERROR] RModelIO::AssetImporter::setObjectFaces(): Unable to add 3rd vertex " << cv::Vec3f( av2[0], av2[1], av2[2]) << std::endl;
            fids[i] = -1;
            continue;
        }   // end if

        // Again, since the float values are copied straight over, it should never be the case that the IDs map to the same value.
        if ((v0 == v1) || (v0 == v2) || (v1 == v2))
        {
            std::cerr << "[ERROR] RModelIO::AssetImporter::setObjectFaces(): Triple of vertex IDs are not all different!" << std::endl;
            fids[i] = -1;
            continue;
        }   // end if
#else
        v0 = model->addVertex( av0[0], av0[1], av0[2]);
        v1 = model->addVertex( av1[0], av1[1], av1[2]);
        v2 = model->addVertex( av2[0], av2[1], av2[2]);
#endif

        const int fid = model->addFace( v0, v1, v2);
        if ( faceSet.count(fid))
        {
            fids[i] = -1;
            dupFaces++;
            continue;
        }   // end if

        fids[i] = fid;
        faceSet.insert(fid);
    }   // end for

    return dupFaces;
}   // end setObjectFaces


void setObjectTextureCoordinates( const aiMesh* mesh, int matId, const std::vector<int>& fids, ObjModel::Ptr model)
{
    // Set the ordering of the texture offsets needed for visualisation
    const int nfaces = (int)mesh->mNumFaces;
    assert( (int)fids.size() == nfaces);
    const aiFace *aifaces = mesh->mFaces;
    for ( int i = 0; i < nfaces; ++i)
    {
        if ( fids[i] >= 0)
        {
            const uint* aiFaceVtxIdxs = aifaces[i].mIndices;   // Indices of vertices from the mesh that make this polygon
            const aiVector3D& aiuv0 = mesh->mTextureCoords[0][aiFaceVtxIdxs[0]];
            const aiVector3D& aiuv1 = mesh->mTextureCoords[0][aiFaceVtxIdxs[1]];
            const aiVector3D& aiuv2 = mesh->mTextureCoords[0][aiFaceVtxIdxs[2]];
            const cv::Vec2f uvs[3] = { cv::Vec2f( aiuv0[0], aiuv0[1]), cv::Vec2f( aiuv1[0], aiuv1[1]), cv::Vec2f( aiuv2[0], aiuv2[1])};
            model->setOrderedFaceUVs( matId, fids[i], uvs);
        }   // end if
    }   // end for
}   // end setObjectTextureCoordinates


// Returns -1 if no textures loaded.
int addMaterial( const boost::filesystem::path& ppath, const aiScene *scene, int meshIdx, ObjModel::Ptr model)
{
    const aiMesh* mesh = scene->mMeshes[meshIdx];
    const aiMaterial* aimat = scene->mMaterials[mesh->mMaterialIndex];

    MaterialTextures mat( aimat, ppath);
    cv::Mat tx;
    if ( mat.loadDiffuse())
        tx = mat.diffuse()[0];
    else if ( mat.loadAmbient())
        tx = mat.ambient()[0];
    else if ( mat.loadSpecular())
        tx = mat.specular()[0];

    if ( tx.empty())
    {
        std::cerr << "\tProblem loading image textures!" << std::endl;
        return -1;
    }   // if

    return model->addMaterial( tx);
}   // end addMaterial


ObjModel::Ptr createModel( Assimp::Importer* importer, const boost::filesystem::path& ppath, bool loadTextures, bool failOnNonTriangles)
{
    const aiScene* scene = importer->GetScene();
    const uint nmaterials = scene->mNumMaterials;
    const uint nmeshes = scene->mNumMeshes;
    std::cerr << "Imported " << nmeshes << " mesh and " << nmaterials << " material parts" << std::endl;

    ObjModel::Ptr model = RFeatures::ObjModel::create();

    std::vector<int>* fidxs = new std::vector<int>;

    for ( uint i = 0; i < nmeshes; ++i)
    {
        fidxs->clear();
        const aiMesh* mesh = scene->mMeshes[i];

        std::cerr << "=====================[ MESH " << std::setw(2) << i << " ]=====================" << std::endl;
        if ( mesh->HasFaces() && mesh->HasPositions())
        {
            int nonTriangles = 0;
            const int dupTriangles = setObjectFaces( mesh, *fidxs, nonTriangles, model);
            if ( nonTriangles > 0)
            {
                if ( failOnNonTriangles)
                {
                    std::cerr << "[ERROR] RModelIO::AssetImporter::createModel()"
                              << " failed on discovery of " << nonTriangles
                              << " non-triangular polygons." << std::endl;
                    model = nullptr;
                    break;
                }   // end if
                else
                {
                    std::cerr << "[WARNING] RModelIO::AssetImporter::createModel(): "
                              << nonTriangles << " non-triangular faces found!" << std::endl;
                }   // end if
            }   // end if

            std::cerr << dupTriangles << " / " << mesh->mNumFaces << " triangles are ignored duplicates." << std::endl;

            if ( !loadTextures)
                continue;

            // Each mesh deals with only a single material. Multi material imports are split into
            // several meshes. Each mesh may or may not have texture coordinates.
            if ( mesh->HasTextureCoords(0))
            {
                // New materials are added only if they define a texture.
                const int matId = addMaterial( ppath, scene, i, model);
                if ( matId >= 0)
                    setObjectTextureCoordinates( mesh, matId, *fidxs, model);
            }   // end if
            else
                std::cerr << "Mesh defines no texture coordinates." << std::endl;
        }   // end if
        std::cerr << "===================================================" << std::endl;
    }   // end for

    delete fidxs;
    return model;
}   // end createModel


std::string getImporterSuffix( const Assimp::Importer* importer, size_t i)
{
    const aiImporterDesc* adesc = importer->GetImporterInfo(i);
    std::string exts = adesc->mFileExtensions;  // "ex0 ex1 ex2"
    boost::algorithm::trim(exts);
    return exts;    // Could be empty
}   // end getImporterSuffix


std::string getImporterDescription( const Assimp::Importer* importer, size_t i)
{
    const aiImporterDesc* adesc = importer->GetImporterInfo(i);
    std::string name = adesc->mName;
    boost::algorithm::replace_last(name, "Importer", "");
    boost::algorithm::replace_all(name, "\n", "");
    rlib::removeParentheticalContent( name);
    return name;
}   // end getImporterDescription

}   // end namespace


// public
AssetImporter::AssetImporter( bool loadTextures, bool failOnNonTriangles)
    : RModelIO::ObjModelImporter(),
      _loadTextures(loadTextures), _failOnNonTriangles(failOnNonTriangles)
{
    std::unordered_set<std::string> disallowed;
    disallowed.insert("3d");
    disallowed.insert("assbin");
    disallowed.insert("assxml");    // Unavailable
    disallowed.insert("dae");   // No exporter available
    disallowed.insert("pk3");   // Not available
    disallowed.insert("xml");   // Too generic
    disallowed.insert("cob");
    disallowed.insert("scn");
    disallowed.insert("mesh.xml");  // Too generic
    disallowed.insert("stp");       // Doesn't work
    disallowed.insert("glb");
    disallowed.insert("gltf");
    disallowed.insert("x");
    disallowed.insert("3ds");   // No good for large files

    Assimp::Importer* importer = new Assimp::Importer;
    const size_t n = importer->GetImporterCount();
    boost::char_separator<char> sep(" ");
    for ( size_t i = 0; i < n; ++i)
    {
        const std::string ext = getImporterSuffix( importer, i);
        if ( ext.empty())
            continue;

        const std::string desc = getImporterDescription( importer, i);
        if ( desc.empty())
            continue;

        // ext could have multiple entries, but will be space separated
        boost::tokenizer<boost::char_separator<char> > tokens( ext, sep);
        for ( const std::string& tok : tokens)
        {
            // Only add if not a disallowed file type
            if ( !disallowed.count(tok))
                _available[tok] = desc;
        }   // end foreach
    }   // end for
    delete importer;
}   // end ctor


// public
bool AssetImporter::enableFormat( const std::string& ext)
{
    if ( _available.count(ext) == 0)
        return false;

    const std::string testfname = "tonythetiger." + ext;
    if ( isSupported( testfname))
        return true;

    return addSupported( ext, _available.at(ext));
}   // end enableFormat


// public
ObjModel::Ptr AssetImporter::doLoad( const std::string& fname)
{
    Assimp::Importer* importer = new Assimp::Importer;
    importer->SetPropertyInteger( AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

    // Read the file into the common AssImp format.
    importer->ReadFile( fname, aiProcess_Triangulate
                             | aiProcess_SortByPType
                             | aiProcess_JoinIdenticalVertices
                             | aiProcess_RemoveRedundantMaterials
                             | aiProcess_OptimizeMeshes
                             | aiProcess_FindDegenerates
                             | aiProcess_FindInvalidData
                             //| aiProcess_OptimizeGraph
                             //| aiProcess_FixInfacingNormals
                             //| aiProcess_FindInstances
                             );

    ObjModel::Ptr model;
    if ( !importer->GetScene())
        setErr( "Unable to read 3D scene into importer from " + fname);
    else
    {
        model = createModel( importer, boost::filesystem::path( fname).parent_path(), _loadTextures, _failOnNonTriangles);
        if (model == nullptr)
            setErr( "Unable to translate imported model into standard format!");
        importer->FreeScene();
    }   // end else

    delete importer;
    return model;
}   // end doLoad


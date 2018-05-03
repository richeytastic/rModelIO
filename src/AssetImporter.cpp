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

#include <AssetImporter.h>
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

typedef unsigned int uint;


cv::Mat loadImage( const boost::filesystem::path& ppath, const std::string& imgfile)
{
    boost::filesystem::path imgPath = ppath / imgfile;
    cv::Mat m;
    if ( boost::filesystem::exists( imgPath))
        m = cv::imread( imgPath.string());
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
    }   // end foreach
    return (int)imgs.size();
}   // end loadImages


// The ambient, diffuse, and specular texture files for a material
struct MaterialTextures
{
    // Set the texture filenames from the given material. Provide the directory
    // path to where the image files are located.
    MaterialTextures( const aiMaterial* mat, const boost::filesystem::path& p)
    {
        setTextureTypeFiles( mat, aiTextureType_AMBIENT, _ambient);
        setTextureTypeFiles( mat, aiTextureType_DIFFUSE, _diffuse);
        setTextureTypeFiles( mat, aiTextureType_SPECULAR, _specular);
        _ppath = p;
    }   // end ctor

    // Load the ambient, diffuse, and specular texture maps returning
    // the number of each type loaded. Returns -1 on error loading.
    int loadAmbient() { return loadImages( _ppath, _ambient, _amats);}
    int loadDiffuse() { return loadImages( _ppath, _diffuse, _dmats);}
    int loadSpecular() { return loadImages( _ppath, _specular, _smats);}

    const std::vector<cv::Mat>& getAmbient() const { return _amats;}
    const std::vector<cv::Mat>& getDiffuse() const { return _dmats;}
    const std::vector<cv::Mat>& getSpecular() const { return _smats;}

    const std::vector<std::string>& getAmbientFilenames() const { return _ambient;}
    const std::vector<std::string>& getDiffuseFilenames() const { return _diffuse;}
    const std::vector<std::string>& getSpecularFilenames() const { return _specular;}

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

    // The filenames for the texture maps
    std::vector<std::string> _ambient;
    std::vector<std::string> _diffuse;
    std::vector<std::string> _specular;

    // The corresponding images
    std::vector<cv::Mat> _amats;
    std::vector<cv::Mat> _dmats;
    std::vector<cv::Mat> _smats;

    boost::filesystem::path _ppath; // Parent path for location of texture image files
};  // end struct


int setObjectVertices( const aiMesh* mesh, std::vector<int>& vidxs, IntSet& vset, ObjModel::Ptr model)
{
    const int n = (int)mesh->mNumVertices;
    int dupCount = 0;
    int vid;
    vidxs.resize( n);
    for ( int aiVtxId = 0; aiVtxId < n; ++aiVtxId)
    {
        const aiVector3D v = mesh->mVertices[aiVtxId];
        vid = model->addVertex( v[0], v[1], v[2]);  // < 0 returned if can't be added (error)
        if ( vid < 0)
            return vid;

        vidxs[aiVtxId] = vid;
        if ( vset.count(vid) == 1) // Vertex already present in model
            dupCount++;
        vset.insert(vid);
    }   // end for
    return dupCount;
}   // end setObjectVertices


int setObjectFaces( const aiMesh* mesh, const std::vector<int>& vidxs, std::vector<int>& fids,
                                        int& nonTriangles, IntSet& faceSet, ObjModel::Ptr model)
{
    const int nfaces = (int)mesh->mNumFaces;
    fids.resize( nfaces);

    int dupFaces = 0; // Count duplicate faces not added
    nonTriangles = 0; // Count number of polygons that aren't triangles
    const aiFace* aifaces = mesh->mFaces;
    for ( int i = 0; i < nfaces; ++i)
    {
        const aiFace& aiface = aifaces[i];
        if ( aiface.mNumIndices != 3)
        {
            nonTriangles++;
            fids[i] = -1;
            continue;
        }   // end if

        // Map the aiMesh face vertex indices to the vertex indices set in the model
        const int v0 = vidxs[aiface.mIndices[0]];
        const int v1 = vidxs[aiface.mIndices[1]];
        const int v2 = vidxs[aiface.mIndices[2]];
        const int fid = model->setFace( v0, v1, v2);

        if ( faceSet.count(fid))
        {
            fids[i] = -1;
            dupFaces++;
        }   // end if
        else
        {
            fids[i] = fid;
            faceSet.insert(fid);
        }   // end else
    }   // end for

    return dupFaces;
}   // end setObjectFaces


void setObjectTextureCoordinates( const aiMesh* mesh, int matId, const std::vector<int>& vidxs, const std::vector<int>& fids, ObjModel::Ptr model)
{
    // Set the ordering of the texture offsets needed for visualisation
    const int nfaces = (int)mesh->mNumFaces;
    assert( (int)fids.size() == nfaces);
    const aiFace *aifaces = mesh->mFaces;
    for ( int i = 0; i < nfaces; ++i)
    {
        if ( fids[i] < 0)
            continue;

        const uint* aiFaceVtxIdxs = aifaces[i].mIndices;   // The indices of the vertices from the mesh that make this polygon

        const aiVector3D& aiuv0 = mesh->mTextureCoords[0][aiFaceVtxIdxs[0]];
        const aiVector3D& aiuv1 = mesh->mTextureCoords[0][aiFaceVtxIdxs[1]];
        const aiVector3D& aiuv2 = mesh->mTextureCoords[0][aiFaceVtxIdxs[2]];
        const cv::Vec2f uvs[3] = { cv::Vec2f( aiuv0[0], aiuv0[1]), cv::Vec2f( aiuv1[0], aiuv1[1]), cv::Vec2f( aiuv2[0], aiuv2[1])};
        const int vs[3] = {vidxs[aiFaceVtxIdxs[0]], vidxs[aiFaceVtxIdxs[1]], vidxs[aiFaceVtxIdxs[2]]};
        model->setOrderedFaceUVs( matId, fids[i], vs, uvs);
    }   // end for
}   // end setObjectTextureCoordinates


// private
// Returns -1 if no textures loaded.
int setObjectTextures( const boost::filesystem::path& ppath, const aiScene *scene, int meshIdx, ObjModel::Ptr model)
{
    const aiMesh* mesh = scene->mMeshes[meshIdx];
    const aiMaterial* aimat = scene->mMaterials[mesh->mMaterialIndex];

    MaterialTextures mat( aimat, ppath);
    const int ta = mat.loadAmbient();
    const int td = mat.loadDiffuse();
    const int ts = mat.loadSpecular();
    if ( ta < 0 || td < 0 || ts < 0)
    {
        std::cerr << "\tProblem loading image textures!" << std::endl;
        return -1;
    }   // if

    int objMatId = -1;
    // Only add a material for the object if at least one texture image is defined.
    if ( ta > 0 || td > 0 || ts > 0)
    {
        objMatId = model->addMaterial();

        for ( int i = 0; i < ta; ++i)
            model->addMaterialAmbient( objMatId, mat.getAmbient()[i]);
        for ( int i = 0; i < td; ++i)
            model->addMaterialDiffuse( objMatId, mat.getDiffuse()[i]);
        for ( int i = 0; i < ts; ++i)
            model->addMaterialSpecular( objMatId, mat.getSpecular()[i]);

        if ( ta > 0)
            std::cerr << "\tSet " << ta << " ambient texture map" << (ta > 1 ? "s" : "") << std::endl;
        if ( td > 0)
            std::cerr << "\tSet " << td << " diffuse texture map" << (td > 1 ? "s" : "") << std::endl;
        if ( ts > 0)
            std::cerr << "\tSet " << ts << " specular texture map" << (ts > 1 ? "s" : "") << std::endl;
    }   // end if

    return objMatId;
}   // end setObjectTextures


// private
ObjModel::Ptr createModel( Assimp::Importer* importer, const boost::filesystem::path& ppath, bool loadTextures, bool failOnNonTriangles)
{
    const aiScene* scene = importer->GetScene();
    const uint nmaterials = scene->mNumMaterials;
    const uint nmeshes = scene->mNumMeshes;
    std::cerr << "[INFO] RModelIO::AssetImporter::createModel(): Imported " << nmeshes << " mesh and " << nmaterials << " material parts\n";

    ObjModel::Ptr model = RFeatures::ObjModel::create();

    std::vector<int>* vidxs = new std::vector<int>;
    std::vector<int>* fidxs = new std::vector<int>;
    IntSet vertSet;
    IntSet faceSet;

    for ( uint i = 0; i < nmeshes; ++i)
    {
        vidxs->clear();
        fidxs->clear();
        const aiMesh* mesh = scene->mMeshes[i];

        std::cerr << "  ====================[ MESH " << std::setw(2) << i << " ]====================" << std::endl;
        if ( mesh->HasFaces() && mesh->HasPositions())
        {
            // Returns the duplicate vertices for *just this mesh*
            const int dupVerts = setObjectVertices( mesh, *vidxs, vertSet, model);
            if ( dupVerts < 0)
            {
                std::cerr << "[ERROR] RModelIO::AssetImporter::createModel(): Invalid vertex in model file (NaN)!" << std::endl;
                model = NULL;
                break;
            }   // end if

            int nonTriangles = 0;
            const int dupTriangles = setObjectFaces( mesh, *vidxs, *fidxs, nonTriangles, faceSet, model);
            if ( nonTriangles > 0)
            {
                if ( failOnNonTriangles)
                {
                    std::cerr << "[ERROR] RModelIO::AssetImporter::createModel()"
                              << " failed on discovery of " << nonTriangles
                              << " non-triangular polygons." << std::endl;
                    model = NULL;
                    break;
                }   // end if
                else
                {
                    std::cerr << "[WARNING] RModelIO::AssetImporter::createModel(): " << nonTriangles
                        << " non-triangular faces found! Non-triangular faces are currently not supported."
                        << " Ensure the model is made up of only triangular polygons before importing." << std::endl;
                }   // end if
            }   // end if

            std::cerr << "  " << dupTriangles << " / " << mesh->mNumFaces
                      << " triangles are ignored duplicates." << std::endl;
            std::cerr << "  " << dupVerts << " / " << mesh->mNumVertices
                      << " vertices are ignored duplicates." << std::endl;

            if ( !loadTextures)
                continue;

            // Each mesh deals with only a single material. Multi material imports are split into
            // several meshes. Each mesh may or may not have texture coordinates.
            if ( mesh->HasTextureCoords(0))
            {
                // New materials are added only if they define a texture.
                const int matId = setObjectTextures( ppath, scene, i, model);
                if ( matId >= 0)
                    setObjectTextureCoordinates( mesh, matId, *vidxs, *fidxs, model);
            }   // end if
            else
                std::cerr << "  Mesh defines no texture coordinates - no material set!" << std::endl;
        }   // end if
        std::cerr << "  ===================================================" << std::endl;
    }   // end for

    delete vidxs;
    delete fidxs;
    std::cerr << "  Read " << vertSet.size() << " total vertices, with " << faceSet.size() << " total faces." << std::endl;

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
    // Read the file into the common AssImp format.
    importer->ReadFile( fname, aiProcess_Triangulate
                             | aiProcess_JoinIdenticalVertices
                             | aiProcess_GenSmoothNormals  // Ignored if normals already present
                             | aiProcess_RemoveRedundantMaterials
                             | aiProcess_SortByPType
                             | aiProcess_PreTransformVertices
                             | aiProcess_OptimizeMeshes
                             | aiProcess_FixInfacingNormals
                             | aiProcess_FindDegenerates
                             | aiProcess_FindInstances
                             | aiProcess_FindInvalidData
                             );
    ObjModel::Ptr model;
    if ( !importer->GetScene())
        setErr( "AssetImporter::read(): Unable to read 3D scene into importer from " + fname);
    else
    {
        model = createModel( importer, boost::filesystem::path( fname).parent_path(), _loadTextures, _failOnNonTriangles);
        if (model == NULL)
            setErr( "AssetImporter::read(): Unable to translate imported model into standard format (RFeatures::ObjModel)");
        importer->FreeScene();
    }   // end else

    delete importer;
    return model;
}   // end doLoad


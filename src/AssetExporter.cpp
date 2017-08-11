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

#include <AssetExporter.h>
#include <FileIO.h> // rlib
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <cassert>
using RModelIO::AssetExporter;
using RFeatures::ObjModel;


std::string saveMaterialTextures( aiMaterial* mat, int matId, const ObjModel::Ptr model, const std::string& fname)
{
    const boost::filesystem::path filepath(fname);
    const boost::filesystem::path fstem = filepath.stem();
    /*
    float ka = 0;
    float kd = 0;
    float ks = 0;
    mat->AddProperty<float>( &ka, 1, AI_MATKEY_COLOR_AMBIENT);
    mat->AddProperty<float>( &kd, 1, AI_MATKEY_COLOR_DIFFUSE);
    mat->AddProperty<float>( &ks, 1, AI_MATKEY_COLOR_SPECULAR);
    */
    std::ostringstream oss;
    oss << fstem.string() << "_" << matId;
    const aiString matName( oss.str());
    mat->AddProperty( &matName, AI_MATKEY_NAME);  // newmtl

    const std::vector<cv::Mat>& ambient = model->getMaterialAmbient(matId);
    const std::vector<cv::Mat>& diffuse = model->getMaterialDiffuse(matId);
    const std::vector<cv::Mat>& specular = model->getMaterialSpecular(matId);

    if (!ambient.empty())
    {
        boost::filesystem::path ipath( fstem.string() + "_ambient.png");
        const aiString tfile( ipath.string());
        mat->AddProperty( &tfile, AI_MATKEY_TEXTURE(aiTextureType_AMBIENT,0));  // map_ka
        boost::filesystem::path imgpath = boost::filesystem::path( tfile.C_Str());
        if ( !boost::filesystem::exists(imgpath))   // Save if not already present
        {
            if ( !cv::imwrite( imgpath.string(), ambient[0]))
                return "Cannot save ambient texture to " + imgpath.string();
        }   // end if
    }   // end if

    if (!diffuse.empty())
    {
        boost::filesystem::path ipath( fstem.string() + "_diffuse.png");
        const aiString tfile( ipath.string());
        mat->AddProperty( &tfile, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE,0));  // map_kd
        boost::filesystem::path imgpath = boost::filesystem::path( tfile.C_Str());
        if ( !boost::filesystem::exists(imgpath))   // Save if not already present
        {
            if ( !cv::imwrite( imgpath.string(), diffuse[0]))
                return "Cannot save diffuse texture to " + imgpath.string();
        }   // end if
    }   // end if

    if (!specular.empty())
    {
        boost::filesystem::path ipath( fstem.string() + "_specular.png");
        const aiString tfile( ipath.string());
        mat->AddProperty( &tfile, AI_MATKEY_TEXTURE(aiTextureType_SPECULAR,0)); // map_ks
        boost::filesystem::path imgpath = boost::filesystem::path( tfile.C_Str());
        if ( !boost::filesystem::exists(imgpath))   // Save if not already present
        {
            if ( !cv::imwrite( imgpath.string(), specular[0]))
                return "Cannot save specular texture to " + imgpath.string();
        }   // end if
    }   // end if

    return "";
}   // end setMaterialTextures


// Set the mesh points, texture coords, and face (polygon) info.
void setMesh( aiMesh* mesh, const ObjModel::Ptr model)
{
    const int mid = mesh->mMaterialIndex;
    const IntSet& mfaceIds = model->getMaterialFaceIds( mid);

    const int nverts = 3 * (int)mfaceIds.size();
    mesh->mNumVertices = nverts;
    mesh->mVertices = new aiVector3D[nverts];
    mesh->mNumUVComponents[0] = nverts;
    mesh->mTextureCoords[0] = new aiVector3D[nverts];

    boost::unordered_map<int,int>* objToAssImpVerts = new boost::unordered_map<int,int>;

    int i = 0;
    BOOST_FOREACH ( int fid, mfaceIds)
    {
        const int* vtxs = model->getFaceVertices(fid);
        const int* uvids = model->getFaceUVs(fid);
        for ( int k = 0; k < 3; ++k)
        {
            const cv::Vec3f& v = model->getVertex(vtxs[k]);
            aiVector3D& vertex = mesh->mVertices[i];
            vertex[0] = v[0];
            vertex[1] = v[1];
            vertex[2] = v[2];

            const cv::Vec2f& uv = model->uv( mid, uvids[k]);
            aiVector3D& texture = mesh->mTextureCoords[0][i];
            texture[0] = uv[0];
            texture[1] = uv[1];
            texture[2] = 0; // Ignored

            (*objToAssImpVerts)[vtxs[k]] = i++; // Map for setting of faces
        }   // end for
    }   // end foreach

    // Set the faces in the mesh
    mesh->mNumFaces = (unsigned int)mfaceIds.size();
    mesh->mFaces = new aiFace[mfaceIds.size()];

    i = 0;
    BOOST_FOREACH ( int fid, mfaceIds)
    {
        const int* vtxs = model->getFaceVertices(fid);
        aiFace& face = mesh->mFaces[i++];
        face.mNumIndices = 3;
        face.mIndices = new unsigned int[3];
        face.mIndices[0] = objToAssImpVerts->at(vtxs[0]);
        face.mIndices[1] = objToAssImpVerts->at(vtxs[1]);
        face.mIndices[2] = objToAssImpVerts->at(vtxs[2]);
    }   // end foreach

    delete objToAssImpVerts;
}   // end setMesh


// Since the Assimp library in its current version doesn't really do
// model export very well, we have to guess the internals of the aiScene
// object and HOPE that we're not doubly allocating here which could
// cause a memory leak. Testing deleting the aiScene causes the internals
// to be free'd too so deleting is a simpler process.
aiScene* createScene( int nmat)
{
    aiScene* scene = new aiScene;
    scene->mRootNode = new aiNode;
    scene->mRootNode->mMeshes = new unsigned int[1];
    scene->mRootNode->mMeshes[0] = 0;
    scene->mRootNode->mNumMeshes = 1;

    scene->mNumMaterials = nmat;
    scene->mMaterials = new aiMaterial*[nmat];
    scene->mNumMeshes = nmat;
    scene->mMeshes = new aiMesh*[nmat];
    for ( int i = 0; i < nmat; ++i)
    {
        scene->mMaterials[i] = new aiMaterial;
        scene->mMeshes[i] = new aiMesh;
        scene->mMeshes[i]->mMaterialIndex = (unsigned int)i;
    }   // end for
    return scene;
}   // end createScene


// public
AssetExporter::AssetExporter( const ObjModel::Ptr model) : RModelIO::ObjModelExporter( model)
{
    boost::unordered_set<std::string> disallowed;
    disallowed.insert("3d");
    disallowed.insert("assbin");
    disallowed.insert("assxml");
    disallowed.insert("dae");
    disallowed.insert("pk3");
    disallowed.insert("xml");
    disallowed.insert("cob");
    disallowed.insert("scn");
    disallowed.insert("mesh.xml");
    disallowed.insert("stp");

    boost::unordered_set<std::string> descSet;  // Don't add same descriptions more than once.
    Assimp::Exporter exporter;
    const size_t n = exporter.GetExportFormatCount();
    for ( size_t i = 0; i < n; ++i)
    {
        const aiExportFormatDesc* efd = exporter.GetExportFormatDescription(i);
        const std::string ext = efd->fileExtension;
        const std::string desc = efd->description;

        if ( ext.empty() || desc.empty() || descSet.count(desc) || disallowed.count(ext))
            continue;

        const bool addedOkay = addSupported( ext, desc);
        assert(addedOkay);
        descSet.insert(desc);
    }   // end for
}   // end ctor


// protected
bool AssetExporter::doSave( const std::string& fname)
{
    const ObjModel::Ptr omodel = _model;
    const int nmat = (int)omodel->getNumMaterials();
    aiScene* scene = createScene( nmat);
    std::string txSaveErr = "";
    for ( int i = 0; i < nmat; ++i)
    {
        aiMesh* mesh = scene->mMeshes[i];
        setMesh( mesh, omodel);
        aiMaterial* mat = scene->mMaterials[i];
        txSaveErr = saveMaterialTextures( mat, i, omodel, fname);
        if (!txSaveErr.empty())
            break;
    }   // end for

    // Check for error saving textures
    if (!txSaveErr.empty())
    {
        setErr( "AssetExporter::write( " + fname + "): " + txSaveErr);
        delete scene;
        return false;
    }   // end if

    bool savedOkay = false;
    Assimp::Exporter exporter;
    if ( exporter.Export( scene, rlib::getExtension(fname), fname) == AI_SUCCESS)
        savedOkay = true;
    else
        setErr( "AssetExporter::write( " + fname + "): " + "Cannot save model! Assimp::Exporter error: " + exporter.GetErrorString());

    delete scene;
    return savedOkay;
}   // end doSave

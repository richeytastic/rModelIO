#include <AssetExporter.h>
#include <IOUtils.h>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cassert>
#include <iostream>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>

using RModelIO::AssetExporter;
using RFeatures::ObjModel;
using RFeatures::ObjPoly;
typedef ObjModel::Ptr OM;
typedef unsigned int uint;


AssetExporter* AssetExporter::s_instance(NULL);


// public static
void AssetExporter::free()
{
    if ( s_instance)
        delete s_instance;
    s_instance = NULL;
}   // end dtor


// private static
AssetExporter* AssetExporter::get()
{
    if ( !s_instance)
        s_instance = new AssetExporter();
    return s_instance;
}   // end get


// private
AssetExporter::AssetExporter()
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

        _exportFormats[ext] = desc;
        descSet.insert(desc);
    }   // end for
}   // end ctor


// private
AssetExporter::~AssetExporter()
{
}   // end dtor


std::string setMaterialTextureFilenames( aiMaterial* mat, int matId,
                                         const ObjModel::Material& omat, const std::string& fname)
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

    if (!omat.ambient.empty())
    {
        boost::filesystem::path ipath( fstem.string() + "_ambient.png");
        const aiString tfile( ipath.string());
        mat->AddProperty( &tfile, AI_MATKEY_TEXTURE(aiTextureType_AMBIENT,0));  // map_ka
        boost::filesystem::path imgpath = boost::filesystem::path( tfile.C_Str());
        if ( !boost::filesystem::exists(imgpath))   // Save if not already present
        {
            if ( !cv::imwrite( imgpath.string(), omat.ambient))
                return "Cannot save ambient texture to " + imgpath.string();
        }   // end if
    }   // end if

    if (!omat.diffuse.empty())
    {
        boost::filesystem::path ipath( fstem.string() + "_diffuse.png");
        const aiString tfile( ipath.string());
        mat->AddProperty( &tfile, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE,0));  // map_kd
        boost::filesystem::path imgpath = boost::filesystem::path( tfile.C_Str());
        if ( !boost::filesystem::exists(imgpath))   // Save if not already present
        {
            if ( !cv::imwrite( imgpath.string(), omat.diffuse))
                return "Cannot save diffuse texture to " + imgpath.string();
        }   // end if
    }   // end if

    if (!omat.specular.empty())
    {
        boost::filesystem::path ipath( fstem.string() + "_specular.png");
        const aiString tfile( ipath.string());
        mat->AddProperty( &tfile, AI_MATKEY_TEXTURE(aiTextureType_SPECULAR,0)); // map_ks
        boost::filesystem::path imgpath = boost::filesystem::path( tfile.C_Str());
        if ( !boost::filesystem::exists(imgpath))   // Save if not already present
        {
            if ( !cv::imwrite( imgpath.string(), omat.specular))
                return "Cannot save specular texture to " + imgpath.string();
        }   // end if
    }   // end if

    return "";
}   // end setTextureFileInScene


// Set the mesh points, texture coords, and face (polygon) info.
void setMesh( aiMesh* mesh, const OM model)
{
    const ObjModel::Material& mat = model->getMaterial( mesh->mMaterialIndex);
    const int nfaces = mat.txOffsets.size();

    const int nverts = 3 * nfaces;
    mesh->mNumVertices = nverts;
    mesh->mVertices = new aiVector3D[nverts];
    mesh->mNumUVComponents[0] = nverts;
    mesh->mTextureCoords[0] = new aiVector3D[nverts];

    boost::unordered_map<int,int>* objToAssImpVerts = new boost::unordered_map<int,int>;

    int i = 0;
    typedef std::pair<int, cv::Vec6f> TUV;
    BOOST_FOREACH ( const TUV& tuv, mat.txOffsets)
    {
        const int fid = tuv.first;
        const cv::Vec6f& tx = tuv.second;
        const cv::Vec3i& vtxs = mat.faceVertexOrder.at(fid);

        for ( int k = 0; k < 3; ++k)
        {
            const cv::Vec3f& v = model->getVertex(vtxs[k]);
            aiVector3D& vertex = mesh->mVertices[i];
            vertex[0] = v[0];
            vertex[1] = v[1];
            vertex[2] = v[2];

            aiVector3D& texture = mesh->mTextureCoords[0][i];
            texture[0] = tx[2*k+0];
            texture[1] = tx[2*k+1];
            texture[2] = 0; // Ignored

            (*objToAssImpVerts)[vtxs[k]] = i++; // Map for setting of faces
        }   // end for
    }   // end foreach

    // Set the faces in the mesh
    mesh->mNumFaces = nfaces;
    mesh->mFaces = new aiFace[nfaces];

    i = 0;
    typedef std::pair<int, cv::Vec3i> FV;
    BOOST_FOREACH ( const FV& fv, mat.faceVertexOrder)
    {
        const cv::Vec3i& vtxs = fv.second;
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
        scene->mMeshes[i]->mMaterialIndex = (uint)i;
    }   // end for
    return scene;
}   // end createScene


// public static
bool AssetExporter::write( const std::string& fname, const OM& omodel)
{
    std::string& errStr = AssetExporter::get()->_errStr;
    errStr = "";    // Clear error string

    // Get the extension from the filename.
    const std::string ext = RModelIO::getExtension(fname);
    if ( ext.empty())
    {
        errStr = "AssetExporter::write( " + fname + "): Can't get extension from filename!";
        std::cerr << errStr << std::endl;
        return false;
    }   // end if

    if ( !AssetExporter::get()->_exportFormats.count(ext))
    {
        errStr = "AssetExporter::write( " + fname + "): Invalid extension!";
        std::cerr << errStr << std::endl;
        return false;
    }   // end if

    const int nmat = (int)omodel->getNumMaterials();
    aiScene* scene = createScene( nmat);
    std::string txSaveErr = "";
    for ( int i = 0; i < nmat; ++i)
    {
        aiMesh* mesh = scene->mMeshes[i];
        setMesh( mesh, omodel);
        aiMaterial* mat = scene->mMaterials[i];
        txSaveErr = setMaterialTextureFilenames( mat, i, omodel->getMaterial(i), fname);
        if (!txSaveErr.empty())
            break;
    }   // end for

    // Check for error saving textures
    if (!txSaveErr.empty())
    {
        errStr = "AssetExporter::write( " + fname + "): " + txSaveErr;
        std::cerr << "[ERROR] " << errStr << std::endl;
        delete scene;
        return false;
    }   // end if

    bool savedOkay = false;
    Assimp::Exporter exporter;
    if ( exporter.Export( scene, ext, fname) == AI_SUCCESS)
        savedOkay = true;
    else
    {
        errStr = "AssetExporter::write( " + fname + "): "
               + "Cannot save model! Assimp::Exporter error: " + exporter.GetErrorString();
        std::cerr << "[ERROR] " << errStr << std::endl;
    }   // end else

    delete scene;
    return savedOkay;
}   // end write


// public static
const std::string& AssetExporter::getError()
{
    return AssetExporter::get()->_errStr;
}   // end getError


// public static
const boost::unordered_map<std::string, std::string>& AssetExporter::getExportFormats()
{
    return AssetExporter::get()->_exportFormats;
}   // end getExportFormats

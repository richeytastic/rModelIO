#include <ObjModelImporter.h>
using RModelIO::ObjModelImporter;


RFeatures::ObjModel::Ptr ObjModelImporter::load( const std::string& fname)
{
    setErr(""); // Clear error
    if ( !isSupported( fname))
    {
        setErr( fname + " has an unsupported file extension for importing!");
        return RFeatures::ObjModel::Ptr();
    }   // end if

    return doLoad( fname);  // virtual
}   // end load

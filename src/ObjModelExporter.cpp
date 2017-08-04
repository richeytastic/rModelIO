#include <ObjModelExporter.h>
using RModelIO::ObjModelExporter;
using RFeatures::ObjModel;


// public
ObjModelExporter::ObjModelExporter( const ObjModel::Ptr m) : _model(m)
{
}   // end ctor


// public
bool ObjModelExporter::save( const std::string& fname)
{
    setErr(""); // Clear error
    if ( !isSupported( fname))
    {
        setErr( fname + " has an unsupported file extension for exporting!");
        return false;
    }   // end if

    return doSave( _model, fname);    // virtual
}   // end save

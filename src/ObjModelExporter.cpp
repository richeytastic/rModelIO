#include <ObjModelExporter.h>
using RModelIO::ObjModelExporter;
using RFeatures::ObjModel;


// public
ObjModelExporter::ObjModelExporter( const ObjModel::Ptr m)
{
}   // end ctor


// public
bool ObjModelExporter::save( const std::string& fname)
{
    return this->doSave( _model, fname);    // virtual
}   // end save


// protected
void ObjModelExporter::setErrMsg( const char* err)
{
    _errStr = err;
}   // end setErrMsg

/**
 * Simple common interface to ObjModel exporters.
 */

#ifndef RMODELIO_OBJ_MODEL_EXPORTER_H
#define RMODELIO_OBJ_MODEL_EXPORTER_H

#include "rModelIO_Export.h"
#include <ObjModel.h>
#include <string>

namespace RModelIO
{

class rModelIO_EXPORT ObjModelExporter
{
public:
    explicit ObjModelExporter( const RFeatures::ObjModel::Ptr);
    ObjModelExporter(){}
    virtual ~ObjModelExporter(){}

    // Returns true on success.
    bool save( const std::string& filename);

    // Get the last error generated from save attempt;
    const std::string& getError() const { return _errStr;}

protected:
    virtual void setErrMsg( const char*);
    virtual bool doSave( const RFeatures::ObjModel::Ptr, const std::string& filename) = 0;

private:
    const RFeatures::ObjModel::Ptr _model;
    ObjModelExporter( const ObjModelExporter&); // NO COPY
    void operator=( const ObjModelExporter&);   // NO COPY
    std::string _errStr;
};  // end class

}   // end namespace

#endif

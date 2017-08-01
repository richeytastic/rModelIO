#ifndef RMODELIO_LAS_IMPORTER_H
#define RMODELIO_LAS_IMPORTER_H

#include "rModelIO_Export.h"
#include <string>
#include <exception>
#include <opencv2/opencv.hpp>
#include <PointCloud.h>

namespace RModelIO
{

class rModelIO_EXPORT LASException : public std::exception
{
public:
    LASException( const std::string &err) : _err(err){}
    virtual ~LASException() throw(){}
    virtual const char* what() const throw(){ return _err.c_str();}
    virtual std::string error() const throw(){ return _err;}
    virtual std::string errStr() const throw(){ return _err;}
private:
    std::string _err;
}; // end class


class rModelIO_EXPORT LASImporter
{
public:
    LASImporter( const std::string& fname);

    // Other info available after this function called.
    RFeatures::PointCloud::Ptr read() throw (LASException);

    double getMinTime() const { return _minTime;}
    double getMaxTime() const { return _maxTime;}

    cv::Vec3d getOffset() const { return _offset;}
    cv::Vec3d getScale() const { return _scale;}

    double getMaxX() const { return _maxX;}
    double getMinX() const { return _minX;}
    double getMaxY() const { return _maxY;}
    double getMinY() const { return _minY;}
    double getMaxZ() const { return _maxZ;}
    double getMinZ() const { return _minZ;}

private:
    const std::string _fname;
    double _minTime, _maxTime;
    double _minX, _maxX, _minY, _maxY, _minZ, _maxZ;
    cv::Vec3d _offset, _scale;
};  // end class

}   // end namespace

#endif

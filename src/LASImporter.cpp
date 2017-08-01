#include "LASImporter.h"
using RModelIO::LASImporter;
using RModelIO::LASException;
using RFeatures::PointCloud;
#include <fstream>
#include <liblas/liblas.hpp>
#include <liblas/reader.hpp>
typedef liblas::Point LASPoint;
typedef liblas::Color LASColour;


void testPointBounds( const cv::Vec3d& p, double& minX, double& maxX, double& minY, double& maxY, double& minZ, double& maxZ)
{
    if (p[0] < minX) minX = p[0];
    if (p[0] > maxX) maxX = p[0];
    if (p[1] < minY) minY = p[1];
    if (p[1] > maxY) maxY = p[1];
    if (p[2] < minZ) minZ = p[2];
    if (p[2] > maxZ) maxZ = p[2];
}   // end testPointBounds


cv::Vec3d readPointCoords( const LASPoint& pt, const cv::Vec3d& scale, const cv::Vec3d& offset)
{
    //const cv::Vec3d p = cv::Vec3d( pt.GetX(), pt.GetY(), pt.GetZ()).mul(_scale) + _offset;
    cv::Vec3d p( pt.GetX(), pt.GetY(), pt.GetZ()); // Original LAS point
    p -= offset;   // Ensure points are relative to offset
    p = p.mul(scale);  // Scaled point
    return p;
}   // end readPointCoords


cv::Vec3b readPointColour( const LASPoint& pt)
{
    const LASColour lasColour = pt.GetColor();
    const uint8_t r = int(255 * lasColour.GetRed() / 65536 + 0.5);
    const uint8_t g = int(255 * lasColour.GetGreen() / 65536 + 0.5);
    const uint8_t b = int(255 * lasColour.GetBlue() / 65536 + 0.5);
    return cv::Vec3b(r,g,b);
}   // end readPointColour


void readPointTime( const LASPoint& pt, double& minTime, double& maxTime)
{
    const double pointTime = pt.GetTime();
    if ( pointTime < minTime)
        minTime = pointTime;
    if ( pointTime > maxTime)
        maxTime = pointTime;
}   // end readPointTime


// 20 bytes
cv::Vec3d readPointFormat0( const LASPoint& pt, const cv::Vec3d& scale, const cv::Vec3d& offset, PointCloud::Ptr pcloud)
{
    const cv::Vec3d p = readPointCoords( pt, scale, offset);
    pcloud->add( p[0], p[1], p[2], 255, 255, 255);
    return p;
}   // end readPointFormat0

// 28 bytes (same as version 0 but with time)
cv::Vec3d readPointFormat1( const LASPoint& pt, const cv::Vec3d& scale, const cv::Vec3d& offset, PointCloud::Ptr pcloud)
{
    const cv::Vec3d p = readPointCoords( pt, scale, offset);
    pcloud->add( p[0], p[1], p[2], 255, 255, 255);
    return p;
}   // end readPointFormat1

// 26 bytes (same as version 0 but with r, g, b)
cv::Vec3d readPointFormat2( const LASPoint& pt, const cv::Vec3d& scale, const cv::Vec3d& offset, PointCloud::Ptr pcloud)
{
    const cv::Vec3d p = readPointCoords( pt, scale, offset);
    const cv::Vec3b colour = readPointColour(pt);
    pcloud->add( p[0], p[1], p[2], colour[0], colour[1], colour[2]);
    return p;
}   // end readPointFormat2

// 34 bytes (same as version 2 but with time)
cv::Vec3d readPointFormat3( const LASPoint& pt, const cv::Vec3d& scale, const cv::Vec3d& offset, PointCloud::Ptr pcloud)
{
    const cv::Vec3d p = readPointCoords( pt, scale, offset);
    const cv::Vec3b colour = readPointColour(pt);
    pcloud->add( p[0], p[1], p[2], colour[0], colour[1], colour[2]);
    return p;
}   // end readPointFormat3


// public
LASImporter::LASImporter( const string& fname)
    : _fname(fname), _minTime(DBL_MAX), _maxTime(0), _maxX(-DBL_MAX), _minX(DBL_MAX), _maxY(-DBL_MAX),
     _minY(DBL_MAX), _maxZ(-DBL_MAX), _minZ(DBL_MAX), _offset(0,0,0), _scale(0,0,0)
{
}   // end ctor


// public
PointCloud::Ptr LASImporter::read() throw (LASException)
{
    std::ifstream ifs;
    ifs.open( _fname.c_str(), std::ios::in | std::ios::binary);
    if ( !ifs.good())
        return PointCloud::Ptr();

    liblas::ReaderFactory readerFactory;    // ReaderFactory takes into account compressed files
    liblas::Reader reader = readerFactory.CreateWithStream( ifs);
    const liblas::Header& header = reader.GetHeader();
    const liblas::PointFormatName pointFormat = header.GetDataFormatId();

    _offset = cv::Vec3d( header.GetOffsetX(), header.GetOffsetY(), header.GetOffsetZ());
    _scale = cv::Vec3d( header.GetScaleX(), header.GetScaleY(), header.GetScaleZ());

    PointCloud::Ptr pcloud = PointCloud::create();  // Unstructured!

    while ( reader.ReadNextPoint())
    {
        const LASPoint &pt = reader.GetPoint();

        cv::Vec3d p;
        switch ( pointFormat)
        {
            case liblas::ePointFormat0:
                p = readPointFormat0( pt, _scale, _offset, pcloud);
                break;
            case liblas::ePointFormat1:
                readPointTime(pt, _minTime, _maxTime);
                p = readPointFormat1( pt, _scale, _offset, pcloud);
                break;
            case liblas::ePointFormat2:
                p = readPointFormat2( pt, _scale, _offset, pcloud);
                break;
            case liblas::ePointFormat3:
                readPointTime(pt, _minTime, _maxTime);
                p = readPointFormat3( pt, _scale, _offset, pcloud);
                break;
            default:
                throw LASException( "Unknown point format!");
                break;
        }   // end switch

        testPointBounds(p, _minX, _maxX, _minY, _maxY, _minZ, _maxZ);
    }   // end while

    ifs.close();
    return pcloud;
}   // end read


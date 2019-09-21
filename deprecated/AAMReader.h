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

#ifndef RMODELIO_AAM_READER_H
#define RMODELIO_AAM_READER_H

#include "rModelIO_Export.h"
#include <string>
#include <PointCloud.h> // RFeatures
#include <Panorama.h>   // RFeatures


namespace RModelIO
{

// Describes a capture location in AAM data
struct rModelIO_EXPORT CapturePoint
{
    std::string imgFile;     // Associated image for (camera 1)
    double timestamp;   // Time at image capture
    cv::Vec3d pos;      // Vehicle IMU location
    double yaw, pitch, roll;    // Degrees of vehicle heading
};  // end struct

rModelIO_EXPORT ostream& operator<<( ostream& os, const CapturePoint& m);


class rModelIO_EXPORT AAMException : public std::runtime_error
{
public:
    explicit AAMException( const std::string& msg) : std::runtime_error(msg) {}
};  // end class


class rModelIO_EXPORT AAMInfoReader
{
public:
    AAMInfoReader( const std::string& metaFile) throw (AAMException);
    ~AAMInfoReader();

    const std::vector<const CapturePoint*>& getCapturePoints() const;
    int getNumCapturePoints() const { return (int)_capturePoints.size();}

private:
    std::vector<const CapturePoint*> _capturePoints;
};  // end class


class rModelIO_EXPORT AAMReader
{
public:
    // Throws Exception if unable to read given file.
    AAMReader( const AAMInfoReader& aamInfo, const std::string& lasFile) throw (AAMException);

    const RFeatures::PointCloud::Ptr getLASCloud() const;

    // Number of panoramas that may be produced from the LAS data.
    // If the LAS data doesn't contain any capture points, 0 is returned.
    int getNumPanos() const;

    // Creates a panorama for the given index in range [0,getNumPanos() )
    // NULL is returned if the panorama for this location lacks sufficient
    // points in one of the faces. NULL is returned if idx is out of range.
    RFeatures::Panorama::Ptr createPanorama( int idx) const;

private:
    const AAMInfoReader& _cps; // The AAM capture points info
    RFeatures::PointCloud::Ptr _pcloud;   // The LAS data
    cv::Vec3d _offset, _scale;
    std::vector<int> _cIndices;
};  // end class


}   // end namespace

#endif

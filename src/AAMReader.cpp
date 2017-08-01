#include <AAMReader.h>
#include <LASImporter.h>
using RModelIO::AAMInfoReader;
using RModelIO::AAMReader;
using RModelIO::CapturePoint;
using RModelIO::AAMException;

#include <PointCloudTools.h>
using RFeatures::PointCloudTools;
#include <FeatureUtils.h>

#include <fstream>


// AAM CAMERA_ATTITUDE_CORRECTIONS(Yaw, Pitch, Roll) in degrees and PRINCIPAL_POINT(XoYoZo)
cv::Vec3d getAAMCamParams( int cam)
{
    cv::Vec3d cameraAttitude(0,0,0);
    switch ( cam)
    {
        case 1:
            cameraAttitude = cv::Vec3d( 107.7908, -1.1822, -0.4762); // Heading, pitch, roll
            break;
        case 2:
            cameraAttitude = cv::Vec3d( 180.8575, -2.3991, -0.1401);
            break;
        case 3:
            cameraAttitude = cv::Vec3d( 251.774, -2.5614, 0.6525);
            break;
        case 4:
            cameraAttitude = cv::Vec3d( 324.627, -2.7791, -0.4384);
            break;
        case 5:
            cameraAttitude = cv::Vec3d( 35.9506, -1.4644, -0.6238);
            break;
        default:
            std::cerr << "INVALID CAMERA!" << std::endl;
    }   // end swtich

    return cameraAttitude;
}   // end getAAMCamParams



ostream& RModelIO::operator<<( ostream& os, const CapturePoint& m)
{
    os << std::fixed;
    os << "Absolute position (XYZ): " << m.pos[0] << ", " << m.pos[1] << ", " << m.pos[2] << std::endl;
    os << "Vehicle yaw, pitch, roll: " << m.yaw << ", " << m.pitch << ", " << m.roll << std::endl;
    //os << "Camera yaw, pitch, roll: " << m.camYaw << ", " << m.camPitch << ", " << m.camRoll << std::endl;
    os << "Timestamp: " << m.timestamp << std::endl;
    os << "Image_file: " << m.imgFile << std::endl;
    return os;
}   // end operator<<



void tokenize( const std::string& ln, char sep, std::vector<std::string>& toks)
{
    const int lnSz = (int)ln.size();
    std::string curTok;
    for ( int i = 0; i < lnSz; ++i)
    {
        if ( ln[i] == sep)
        {
            toks.push_back(curTok);
            curTok = "";
        }   // end if
        else
            curTok += ln[i];
    }   // end for
    if ( !curTok.empty())
        toks.push_back(curTok);
}   // end tokenize


double cnvDbl( const std::string& tok)
{
    double d;
    std::istringstream ss(tok);
    ss >> d;
    return d;
}   // end cnvDbl



int readMetaData( const std::string& metaFile, std::vector<const CapturePoint*>& capturePoints)
{
    //const cv::Vec3d camParams = getAAMCamParams( camera);
   
    int readCount = 0;
    try
    {
        std::vector<std::string> toks;
        std::string ln;
        std::ifstream ifs( metaFile.c_str());
        while ( std::getline( ifs, ln) && !ln.empty())
        {
            if ( ln.size() < 60)    // Ignores first line containing column headers
                continue;

            toks.clear();
            tokenize( ln, '|', toks);

            CapturePoint* cp = new CapturePoint;
            cp->imgFile = toks[0];
            cp->timestamp = cnvDbl(toks[1]);

            const cv::Vec3d imupos( cnvDbl(toks[2]), cnvDbl(toks[3]), cnvDbl(toks[4]));   // IMU position
            cp->pos = imupos;

            cp->yaw = cnvDbl( toks[5]);    // Heading of vehicle
            cp->roll = cnvDbl( toks[6]);   // Roll of vehicle
            cp->pitch = cnvDbl( toks[7]);  // Pitch of vehicle

            capturePoints.push_back(cp);
            readCount++;
        }   // end while
        ifs.close();

        /*
        // Camera direction is sum of vehicle orientation plus camera orientation (camera 3)
        cp.camYaw = camParams[0];
        cp.camPitch = camParams[1];
        cp.camRoll = camParams[2];
        */
    }   // end try
    catch ( const std::exception&)
    {
        readCount = -1;
    }   // end catch

    return readCount;
}   // end readMetaData


AAMInfoReader::AAMInfoReader( const std::string& metaFile) throw (AAMException)
{
    const int readCount = readMetaData( metaFile, _capturePoints);
    if ( readCount < 0)
    {
        std::ostringstream oss;
        oss << "AAMInfoReader ERROR: Unable to read in AAM meta data from " << metaFile;
        throw AAMException( oss.str());
    }   // end if
}   // end ctor


AAMInfoReader::~AAMInfoReader()
{
    const int csz = (int)_capturePoints.size();
    for ( int i = 0; i < csz; ++i)
        delete _capturePoints[i];
}   // end dtor


const std::vector<const CapturePoint*>& AAMInfoReader::getCapturePoints() const
{
    return _capturePoints;
}   // end getCapturePoints



AAMReader::AAMReader( const AAMInfoReader& aamInfo, const std::string& lasFile) throw (AAMException)
    : _cps(aamInfo)
{
    RModelIO::LASImporter importerLAS( lasFile);
    _pcloud = importerLAS.read();
    if ( _pcloud == NULL)
    {
        std::ostringstream oss;
        oss << "AAMReader ERROR: Unable to read LAS data from " << lasFile;
        throw AAMException( oss.str());
    }   // end if

    _offset = importerLAS.getOffset();
    _scale = importerLAS.getScale();

    // Find the indices into the capture points array that's relevant for this LAS file
    const double minTime = importerLAS.getMinTime();
    const double maxTime = importerLAS.getMaxTime();
    const double minX = importerLAS.getMinX();
    const double minY = importerLAS.getMinY();
    const double minZ = importerLAS.getMinZ();
    const double maxX = importerLAS.getMaxX();
    const double maxY = importerLAS.getMaxY();
    const double maxZ = importerLAS.getMaxZ();
    /*
    cerr << "Scale: " << _scale[0] << ", " << _scale[1] << ", " << _scale[2] << endl;
    cerr << "X range: " << minX << " - " << maxX << endl;
    cerr << "Y range: " << minY << " - " << maxY << endl;
    cerr << "Z range: " << minZ << " - " << maxZ << endl;
    */

    const std::vector<const CapturePoint*>& cps = _cps.getCapturePoints();
    const int numCPs = (int)cps.size();
    for ( int i = 0; i < numCPs; ++i)
    {
        const CapturePoint* cp = cps[i];
        // Is the timestamp okay?
        if ( cp->timestamp >= minTime && cp->timestamp <= maxTime)
        {
            const cv::Vec3d pos = (cp->pos - _offset).mul(_scale);
            // Is this capture point within the LAS data?
            if ( pos[0] >= minX && pos[0] <= maxX && pos[1] >= minY && pos[1] <= maxY && pos[2] >= minZ && pos[2] <= maxZ)
                _cIndices.push_back(i);
        }   // end if
    }   // end for
}   // end ctor



const PointCloud::Ptr AAMReader::getLASCloud() const
{
    return _pcloud;
}   // end getLASCloud



int AAMReader::getNumPanos() const
{
    return (int)_cIndices.size();
}   // end getNumPanos



RFeatures::Panorama::Ptr AAMReader::createPanorama( int idx) const
{
    RFeatures::Panorama::Ptr pano;
    if ( idx < 0 || idx >= _cIndices.size())
        return pano;

    const std::vector<const CapturePoint*>& capturePoints = _cps.getCapturePoints();
    const CapturePoint& cp = *capturePoints[ _cIndices[idx]];

    // Scale position to account for point cloud data scaling
    const cv::Vec3d pos = (cp.pos - _offset).mul(_scale);

    const cv::Vec3d frv = RFeatures::applyYawRollPitch( cv::Vec3d(0,1,0), cp.yaw, cp.pitch, cp.roll);
    const cv::Vec3d upv = RFeatures::applyYawRollPitch( cv::Vec3d(0,0,1), cp.yaw, cp.pitch, cp.roll);

    std::vector<PointCloud::Ptr> pcs = PointCloudTools::createFaces( frv, upv, pos, _pcloud, _scale);
    if ( pcs.size() == 0)
        return pano;

    pano = RFeatures::Panorama::Ptr( new RFeatures::Panorama( pos, upv, cp.yaw, cp.pitch, cp.roll));
    assert( pcs.size() == 4);
    // Interpolate the point clouds to remove missing info
    pcs[0] = PointCloudTools::interpolate( pcs[0]);
    pcs[1] = PointCloudTools::interpolate( pcs[1]);
    pcs[2] = PointCloudTools::interpolate( pcs[2]);
    pcs[3] = PointCloudTools::interpolate( pcs[3]);

    // Build a view for each point cloud
    const cv::Vec3d rightVec = frv.cross(upv);
    pano->setFront( PointCloudTools::createView( pcs[0], pos, frv, upv));  // Front
    pano->setLeft( PointCloudTools::createView( pcs[1], pos, -rightVec, upv)); // Left
    pano->setRear( PointCloudTools::createView( pcs[2], pos, -frv, upv));  // Rear
    pano->setRight( PointCloudTools::createView( pcs[3], pos, rightVec, upv));

    return pano;
}   // end createPanorama

#include <LaTeXExporter.h>      // rModelIO
#include <AssetImporter.h>      // rModelIO
#include <ModelViewer.h>        // FaceTools
#include <TestUtils.h>          // TestUtils
#include <cstdlib>
#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>
using RFeatures::ObjModel;


// Return the distance of the most extreme vertex on the model from it's average position.
float getMaxRad( const ObjModel::Ptr model)
{
    cv::Vec3f mpos(0,0,0);  // Will be the mean pos
    const IntSet& vidxs = model->getVertexIds();
    BOOST_FOREACH ( int vidx, vidxs)
        mpos += model->vtx(vidx);
    mpos *= 1.f/vidxs.size();    // Mean pos

    double maxl2sq = 0.0;
    BOOST_FOREACH ( int vidx, vidxs)
        maxl2sq = std::max( RFeatures::l2sq( model->vtx(vidx) - mpos), maxl2sq);
    return float(sqrt(maxl2sq));
}   // end getMaxRad


RFeatures::CameraParams showModel( int argc, char** argv, const ObjModel::Ptr model)
{
    QApplication qapp(argc,argv);
    FaceTools::ModelViewer mviewer;
    mviewer.enableFloodLights(true);
    const float rad = getMaxRad(model);
    mviewer.setCamera( cv::Vec3f(0,0,0), cv::Vec3f(0,0,1), cv::Vec3f(0,1,0), 4*rad);
    mviewer.fitCamera( rad);
    mviewer.add(model);
    mviewer.updateRender();
    mviewer.show();
    qapp.exec();
    return mviewer.getCamera();
}   // end showModel


int main( int argc, char** argv)
{
    RFeatures::ObjModel::Ptr model;
    std::string outfile = "tst.pdf";
    if ( argc == 1)
        model = TestUtils::createCube( 20, "tx0.png");
    else
    {
        model = TestUtils::loadModel( argv[1], true, true); // Load texture and clean
        if ( model == NULL)
        {
            std::cerr << "Failed to open model from " << argv[1] << "! Exiting..." << std::endl;
            exit(EXIT_FAILURE);
        }   // end if
        outfile = boost::filesystem::path( argv[1]).stem().string() + ".pdf";
    }   // end if

    // Show model debug info (in detail if model faces < 20)
    model->showDebug( model->getNumFaces() < 20);

    // Show the model and return the camera parameters when user exits the viewer.
    const RFeatures::CameraParams camera = showModel( argc, argv, model);

    int rval = EXIT_SUCCESS;
#ifdef IDTF_CONVERTER
    std::cout << "Converting to U3D using camera parameters:" << std::endl;
    std::cout << camera << std::endl;
    RModelIO::LaTeXExporter exporter(model, camera, 150, 150);
    exporter.setFigureCaption( "Example embedding of 3D face in PDF.");
    if ( !exporter.save( outfile))
    {
        std::cerr << exporter.err() << std::endl;
        rval = EXIT_FAILURE;
    }   // end if
#else
    std::cout << "IDTF_CONVERTER not set - can't produce U3D output!" << std::endl;
#endif
    return rval;
}   // end main

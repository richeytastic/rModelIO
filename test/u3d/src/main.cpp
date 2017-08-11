#include <PDFGenerator.h>      // rModelIO
#include <U3DExporter.h>       // rModelIO
#include <AssetImporter.h>     // rModelIO
#include <ModelViewer.h>       // FaceTools
#include <TestUtils.h>         // TestUtils
#include <cstdlib>
#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>
using RFeatures::ObjModel;
using RFeatures::CameraParams;
using RModelIO::PDFGenerator;

#ifndef IDTF_CONVERTER
#define IDTF_CONVERTER ""
#endif
#ifndef PDFLATEX_PROCESSOR
#define PDFLATEX_PROCESSOR ""
#endif



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


bool writeDoc( PDFGenerator& pdfgen, const std::string& texfile, const ObjModel::Ptr model, const CameraParams& cam)
{
    bool writtenOkay = false;
    std::ofstream os;
    try
    {
        os.open( texfile.c_str(), std::ios::out);
        os << "\\documentclass{article}" << std::endl;
        os << "\\usepackage[a4paper]{geometry}" << std::endl;
        os << "\\usepackage{a4wide}" << std::endl;
        os << "\\usepackage{media9}" << std::endl;
        os << "\\usepackage[parfill]{parskip}" << std::endl;
        os << "\\usepackage[colorlinks=true,urlcolor=red]{hyperref}" << std::endl;
        os << std::endl;
        os << "\\begin{document}" << std::endl;
        os << "\\title{Example PDF embedded U3D object}" << std::endl;
        os << "\\author{\\href{mailto:r.l.palmer@curtin.edu.au}{Richard Palmer}}" << std::endl;
        os << "\\maketitle" << std::endl;
        os << "\\thispagestyle{empty}" << std::endl;
        os << std::endl;

        os << pdfgen.getModelInserter( model, 150, 150, cam, "Example U3D") << std::endl;

        os << "\\end{document}" << std::endl;
        os.flush();
        writtenOkay = true;
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << "[EXCEPTION!] Exception writing to file " << texfile << std::endl;
        std::cerr << e.what() << std::endl;
        writtenOkay = false;
    }   // end catch
    os.close();
    return writtenOkay;
}   // end writeDoc


int main( int argc, char** argv)
{
    RModelIO::U3DExporter::IDTFConverter = IDTF_CONVERTER;
    PDFGenerator::pdflatex = PDFLATEX_PROCESSOR;
    if ( std::string(IDTF_CONVERTER).empty() || std::string(PDFLATEX_PROCESSOR).empty())
    {
        std::cerr << "pdflatex and/or IDTFConverter not set! Exiting..." << std::endl;
        return EXIT_FAILURE;
    }   // end if

    std::cerr << "IDTFConverter: " << IDTF_CONVERTER << std::endl;
    std::cerr << "pdflatex:      " << PDFLATEX_PROCESSOR << std::endl;

    RFeatures::ObjModel::Ptr model;
    std::string pdffile = "tst.pdf";
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
        pdffile = boost::filesystem::path( argv[1]).stem().string() + ".pdf";
    }   // end if

    //model->showDebug( model->getNumFaces() < 20); // Show model debug info (in detail if model faces < 20)

    // Show the model and return the camera parameters when user exits the viewer.
    const RFeatures::CameraParams camera = showModel( argc, argv, model);

    int rval = EXIT_SUCCESS;
    std::cout << "Converting to U3D using camera parameters:" << std::endl;
    std::cout << camera << std::endl;

    const std::string texfile = boost::filesystem::path(pdffile).stem().string() + ".tex";
    PDFGenerator pdfgen;
    writeDoc( pdfgen, texfile, model, camera);
    if ( !pdfgen( texfile, true))
        rval = EXIT_FAILURE;
    return rval;
}   // end main

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

#include <LaTeXExporter.h>
#include <U3DExporter.h>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <fstream>
#include <iomanip>
#include <cerrno>
#include <cstdio>
using RModelIO::LaTeXExporter;
using RModelIO::U3DExporter;
using RFeatures::ObjModel;
using RFeatures::CameraParams;


// public
LaTeXExporter::LaTeXExporter( const ObjModel::Ptr model, const CameraParams& cam, float fw, float fh, bool actOnOpen)
    : RModelIO::ObjModelExporter( model),
      _cam(cam),
      _fw(fw), _fh(fh),
      _activeOnOpen(actOnOpen),
      _pdflatex()
{
#ifdef PDFLATEX_PROCESSOR
    addSupported( "pdf", "Portable Document Format");
    std::cout << "PDFLATEX_PROCESSOR defined as " << PDFLATEX_PROCESSOR << std::endl;
    _pdflatex = PDFLATEX_PROCESSOR;
#else
    std::cerr << "[WARNING] RModelIO::LaTeXExporter: PDF export unavailable; pdflatex not defined!" << std::endl;
#endif
}   // end ctor


// public
LaTeXExporter::~LaTeXExporter()
{
}   // end dtor


// public
void LaTeXExporter::setFigureCaption( const std::string& fcap)
{
    _figCaption = fcap;
}   // end setFigureCaption


// public
void LaTeXExporter::setFigureLabel( const std::string& flab)
{
    _figLabel = flab;
}   // end setFigureLabel


// public
bool LaTeXExporter::insert( std::ostream& os, const std::string& u3dfile) const
{
    typedef boost::filesystem::path Path;
    if ( boost::algorithm::to_lower_copy( Path(u3dfile).extension().string()) != ".u3d")
    {
        std::cerr << "[ERROR] RModelIO::LaTeXExporter::insert: " << u3dfile << " must have .u3d extension!" << std::endl;
        return false;
    }   // end if

    const cv::Vec3f& coo = _cam.focus;
    cv::Vec3f c2c = _cam.pos - coo;
    const float roo = cv::norm(c2c);
    cv::normalize( c2c, c2c); // Doesn't need normalizing, but such things are habitual...

    const std::string caption = _figCaption.empty() ? u3dfile : _figCaption;

    os << "\\begin{figure}[!ht]" << std::endl;
    os << "\\centering" << std::endl;
    os << "\\includemedia[" << std::endl;
    os << "\twidth=" << _fw << "mm," << std::endl;
    os << "\theight=" << _fh << "mm," << std::endl;
    os << "\tkeepaspectratio," << std::endl;
    os << "\tactivate=" << (_activeOnOpen ? "pageopen" : "click") << "," << std::endl;
    os << "\tplaybutton=plain,    \% plain | fancy (default) | none" << std::endl;
    os << "\t3Dlights=Hard," << std::endl;
    os << "\t3Dcoo=" << coo[0] << " " << coo[1] << " " << coo[2] << ",         \% centre of orbit of the camera (x y z)" << std::endl;
    os << "\t3Dc2c=" << c2c[0] << " " << c2c[1] << " " << c2c[2] << ",         \% direction to camera from coo" << std::endl;
    os << "\t3Droll=0,            \% clockwise roll in degrees around optical axis" << std::endl;
    os << "\t3Droo=" << std::left << std::fixed << std::setprecision(3) << roo << ",        % radius of obrbit" << std::endl;
    os << "\t3Daac=" << std::left << std::fixed << std::setprecision(3) << _cam.fov << ",        \% perspective fov in degrees" << std::endl;
    os << "\t3Dbg=1 1 1           \% background colour of scene (r g b) \\in [0,1] (can't set transparency if set)" << std::endl;
    os << "\t]{}{" << u3dfile << "}" << std::endl;
    os << "\\caption{``" << caption << "''}" << std::endl;
    if ( !_figLabel.empty())
        os << "\\label{" << _figLabel << "}" << std::endl;
    os << "\\end{figure}" << std::endl;

    return true;
}   // end insert



// private
bool LaTeXExporter::writeLaTeX( const std::string& texfile, const std::string& u3dfile) const
{
    bool writtenOkay = false;
    std::ofstream os;
    try
    {
        os.open( texfile.c_str(), std::ios::out);
        os << "\\documentclass[a4paper]{article}" << std::endl;
        os << "\\usepackage{media9}" << std::endl;
        os << "\\usepackage[a4paper]{geometry}" << std::endl;
        os << "\\usepackage{a4wide}" << std::endl;
        os << "\\usepackage[parfill]{parskip}" << std::endl;
        os << "\\usepackage[colorlinks=true,urlcolor=red]{hyperref}" << std::endl;
        os << std::endl;
        os << "\\begin{document}" << std::endl;
        os << "\\title{Example PDF embedded U3D object}" << std::endl;
        os << "\\author{\\href{mailto:r.l.palmer@curtin.edu.au}{Richard Palmer}}}" << std::endl;
        os << "\\maketitle" << std::endl;
        os << "\\thispagestyle{empty}" << std::endl;
        os << std::endl;
        insert( os, u3dfile);
        os << std::endl;
        os << "\\end{document}" << std::endl;
        os.flush();
        writtenOkay = true;
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << "Exception writing to file " << texfile << ": " << e.what() << std::endl;
        writtenOkay = false;
    }   // end catch
    os.close();
    return writtenOkay;
}   // end writeLaTeX



// public static
bool LaTeXExporter::runPDFLaTeX( const std::string& texfile)
{
    std::string pdflatex;
#ifdef PDFLATEX_PROCESSOR
    pdflatex = PDFLATEX_PROCESSOR;
#endif
    if ( pdflatex.empty())
    {
        std::cerr << "[ERROR] RModelIO::LaTeXExporter::runPDFLaTeX: PDF export unavailable; pdflatex not defined!" << std::endl;
        return false;
    }   // end if

    bool success = false;
    const std::string cmd = pdflatex + " -interaction batchmode " + texfile;
    try
    {
        // PDFLaTeX typically has to run at twice.
        std::cerr << "Attempting to generate PDF from " << texfile << std::endl;
        FILE* pipe = popen( cmd.c_str(), "r");
        if (!pipe)
            std::cerr << "[ERROR] RModelIO::LaTeXExporter::runPDFLaTeX: unable to open pipe; failed to execute \"" << cmd << "\"" << std::endl;
        else if ( pclose(pipe) < 0)
        {
            const int errsv = errno;
            std::cerr << "[ERROR] RModelIO::LaTeXExporter::runPDFLaTeX: unable to close pipe; failed to execute \"" << cmd << "\"" << std::endl;
            std::cerr << "errno: " << errsv << std::endl;
        }   // end if
        else
            success = true;
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << "[EXCEPTION!] RModelIO::LaTeXExporter::runPDFLaTeX: failed to execute \"" << cmd << "\"" << std::endl;
        std::cerr << e.what() << std::endl;
        success = false;
    }   // end ctch
    return success;
}   // end runPDFLaTeX


void cleanUpFiles( const std::string& texfile)
{
}   // end cleanUpFiles


// protected
bool LaTeXExporter::doSave( const std::string& filename)
{
    typedef boost::filesystem::path Path;
    Path fname(filename);
    if (!fname.has_extension())
        fname += ".pdf";

    const bool generatePDF = !_pdflatex.empty() && boost::algorithm::to_lower_copy(fname.extension().string()) == ".pdf";
    const std::string texfile = fname.stem().string() + ".tex";
    const std::string u3dfile = fname.stem().string() + ".u3d";

    U3DExporter u3dExporter( _model);
    if ( !u3dExporter.save( u3dfile))
    {
        setErr( "Failed to convert object to U3D!");
        return false;
    }   // end if

    if (!writeLaTeX( texfile, u3dfile))
    {
        setErr( "Failed to write LaTeX file to " + texfile);
        return false;
    }   // end if

    if (generatePDF)
    {
        if ( !runPDFLaTeX( texfile))
        {
            setErr( "Failed to generate PDF from " + texfile + " using " + _pdflatex);
            return false;
        }   // end if

        cleanUpFiles( texfile);
    }   // end if

    return true;
}   // end doSave

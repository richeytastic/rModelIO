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

/**
 * Provides functionality to dump RFeatures::ObjModels into PDF via LaTeX
 * and U3D model format conversion.
 *
 * Given an existing output stream, function operator<< takes a LaTeXU3DInserter
 * instance and writes a LaTeX figure for parsing by the media9 package at that
 * location in the stream. The LaTeXU3DInserter object may be set with an ObjModel
 * instance, which is internally converted to a U3D model - the filepath of which
 * is inserted into the LaTeX stream. On destruction of this object, all U3D
 * objects produced in this way are removed from the filesystem.
 *
 * After all figure insertions have been completed and the .tex file saved,
 * its path can be passed to a PDFGenerator instance for running pdflatex
 * which uses the media9 package to embed the U3D models found in the .tex
 * file. A suitable pdflatex program must be set in static variable pdflatex.
 * Ensure the media9 package is included in the LaTeX by adding:
 * "\usepackage{media9}" somewhere after the documentclass declaration.
 *
 * The operator function returns the filepath to the generated pdf file
 * (which will be adjacent in the filesystem to texfile), or an empty string
 * on error. Optionally, the PDFGenerator can be set to remove all generated
 * files from the pdflatex process on destruction. In addition, the .tex
 * file passed to the generate function can be set for removal (but only
 * upon successful generation/output of pdflatex).
 *
 * Currently, support for viewing 3D PDFs is limited with Adobe Reader being
 * the only PDF viewer that I've come across that reliably shows 3D content.
 * Hopefully other readers with 3D model viewing plugins will be developed
 * (especially for Linux!)
 * 
 * -BUGS-
 * A constant camera up vector is not being respected (problem likely
 * with the media9 package) which causes the camera to be rotated to
 * have an up vector in the -X direction when passing in a camera location
 * which is not on the +Z axis (currently don't know how camera behaves
 * when set at other locations). For the moment, the camera position
 * (as provided in the CameraParams object) is ignored and the camera
 * position is set as the camera focus plus (0,0,1)*D where D is the
 * distance between the provided (but ignored) position, and the focus.
 * This might well mean that a focus at (0,0,0) is also necessary to
 * circumvent the issue (models should ideally be transformed to this
 * point anyway in most cases).
 *
 * Richard Palmer
 * August 2017
 */

#ifndef RMODELIO_PDF_GENERATOR_H
#define RMODELIO_PDF_GENERATOR_H

#include "rModelIO_Export.h"
#include <CameraParams.h>
#include <ObjModel.h>
#include <iostream>
#include <string>
#include <vector>


namespace RModelIO
{

class rModelIO_EXPORT PDFGenerator
{
public:
    static std::string pdflatex;    // Set with filepath of pdflatex before use.

    // Set remGen to true to remove files generated by pdflatex whether it
    // succeeds or fails, but never if pdflatex fails within a debug build.
    explicit PDFGenerator( bool remGen=true);
    virtual ~PDFGenerator();

    // Run pdflatex against texfile - returns false if pdflatex fails or is not defined.
    // On returning true, file created is texfile with extension replaced with .pdf.
    // Set removeTexfileOnSuccess to delete texfile on success (unless debug build is
    // active in which case the input texfile is never deleted).
    bool operator()( const std::string& texfile, bool removeTexfileOnSuccess=false);


    struct LaTeXU3DInserter;

    // New figure inserter.
    LaTeXU3DInserter& getModelInserter( const RFeatures::ObjModel::Ptr,
                                        float figWidthMM, float figHeightMM, // Width and height of figure in mm
                                        const RFeatures::CameraParams& cam,  // ONLY pos-focus distance used - see above!
                                        const std::string& figCaption="",    // Caption ignored if empty
                                        const std::string& figLabel="",      // Label not written if empty
                                        bool activate3DContentOnOpen=true);  // Activate 3D model on open (true) or click (false)

    struct rModelIO_EXPORT LaTeXU3DInserter
    {
        void setDimensions( float figWidthMM, float figHeightMM);
        void setCamera( const RFeatures::CameraParams& cam);
        void setCaption( const std::string&);
        void setLabel( const std::string&);
        void setActivateOnOpen( bool);

        // Set the object to export into the figure, either as the filepath of
        // an already existing U3D model (in which case the file must have the
        // extension ".u3d" or false is returned), or as a RFeatures::ObjModel
        // instance whereupon conversion will be conducted and the U3D model
        // saved in a temporary file location. If conversion to U3D fails,
        // false is returned. Note that in the second case, the temporary
        // file (the produced U3D file) is deleted upon destruction of this
        // object, so any PDF generation using the same LaTeX file must be
        // conducted while this object is still alive.
        // NB RModelIO::U3DExporter::getConverter() must return a valid path
        // to the IDTFConverter program in order for the second version to work
        // (see RModelIO::U3DExporter for details).
        bool setModel( const std::string& u3dfilename);
        bool setModel( const RFeatures::ObjModel::Ptr);

    private:
        float _fw;
        float _fh;
        RFeatures::CameraParams _cam;
        std::string _figCap;
        std::string _figLab;
        bool _actOnOpen;
        std::string _u3dfile;
        std::vector<std::string> _delfiles;

        LaTeXU3DInserter( float figWidthMM, float figHeightMM, const RFeatures::CameraParams& cam,
                          const std::string& figCaption="", const std::string& figLabel="",
                          bool activate3DContentOnOpen=true);
        virtual ~LaTeXU3DInserter();

        friend class PDFGenerator;
        friend std::ostream& operator<<( std::ostream&, const LaTeXU3DInserter&);
    };  // end class

private:
    const bool _remGen;
    std::vector<LaTeXU3DInserter*> _inserters;
};  // end class


std::ostream& operator<<( std::ostream& texOutputStream, const RModelIO::PDFGenerator::LaTeXU3DInserter&);


}   // end namespace


#endif

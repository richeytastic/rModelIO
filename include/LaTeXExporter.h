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
 * Provides functionality to dump RFeatures::ObjModels into LaTeX.
 *
 * Calling ObjModelExporter::save({file}.pdf) on this exporter creates a basic
 * LaTeX file which, if the filename ends with .pdf, will be compiled with the
 * LaTeX processor defined by PDFLATEX_PROCESSOR; the resulting document
 * being saved to {file}.pdf.
 *
 * If PDFLATEX_PROCESSOR is not defined, or if the filename is passed in as
 * {file}.tex, the created LaTeX will be saved at the given location with the
 * converted U3D format model saved alongside as {file}.u3d.
 *
 * NB) PDF embedding by the pdflatex processor requires the media9 package.
 *
 * insert( std::ostream& os, const std::string& u3dfilename) does NOT do any
 * model conversion (and so doesn't need PDFLATEX_PROCESSOR defined), but
 * simply inserts the given filename as part of a LaTeX figure element
 * into the provided stream.
 *
 * Richard Palmer
 * August 2017
 */

#ifndef RMODELIO_LaTeX_EXPORTER_H
#define RMODELIO_LaTeX_EXPORTER_H

#include "ObjModelExporter.h"
#include <CameraParams.h>

namespace RModelIO
{

class rModelIO_EXPORT LaTeXExporter : public ObjModelExporter
{
public:
    LaTeXExporter( const RFeatures::ObjModel::Ptr,
                   const RFeatures::CameraParams& cam,
                   float figWidthMM, float figHeightMM,
                   bool activate3DContentOnOpen=true);
    virtual ~LaTeXExporter();

    // The figure caption will be set to the u3dfilename if not set
    void setFigureCaption( const std::string&);

    // No label for the figure will be written unless set here first.
    void setFigureLabel( const std::string&);

    bool insert( std::ostream& os, const std::string& u3dfilename) const;

    // Run pdflatex (or whatever PDFLATEX_PROCESSOR defines) against texfile.
    static bool runPDFLaTeX( const std::string& texfile);

protected:
    virtual bool doSave( const std::string& filename);

private:
    const RFeatures::CameraParams& _cam;
    const float _fw, _fh;
    const bool _activeOnOpen;
    std::string _pdflatex;
    std::string _figCaption;
    std::string _figLabel;
    bool writeLaTeX( const std::string&, const std::string&) const;
};  // end class

}   // end namespace

#endif

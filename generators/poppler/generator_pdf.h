/***************************************************************************
 *   Copyright (C) 2004-2008 by Albert Astals Cid <aacid@kde.org>          *
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_GENERATOR_PDF_H_
#define _OKULAR_GENERATOR_PDF_H_

#include "synctex/synctex_parser.h"

#include <poppler-qt5.h>

#include <qbitarray.h>
#include <qpointer.h>

#include <core/document.h>
#include <core/generator.h>
#include <interfaces/configinterface.h>
#include <interfaces/printinterface.h>
#include <interfaces/saveinterface.h>

namespace Okular {
class ObjectRect;
class SourceReference;
}

class PDFOptionsPage;
class PopplerAnnotationProxy;

/**
 * @short A generator that builds contents from a PDF document.
 *
 * All Generator features are supported and implented by this one.
 * Internally this holds a reference to xpdf's core objects and provides
 * contents generation using the PDFDoc object and a couple of OutputDevices
 * called Okular::OutputDev and Okular::TextDev (both defined in gp_outputdev.h).
 *
 * For generating page contents we tell PDFDoc to render a page and grab
 * contents from out OutputDevs when rendering finishes.
 *
 */
class PDFGenerator : public Okular::Generator, public Okular::ConfigInterface, public Okular::PrintInterface, public Okular::SaveInterface
{
    Q_OBJECT
    Q_INTERFACES( Okular::ConfigInterface )
    Q_INTERFACES( Okular::PrintInterface )
    Q_INTERFACES( Okular::SaveInterface )

    public:
        PDFGenerator( QObject *parent, const QVariantList &args );
        virtual ~PDFGenerator();

        static const int PDFDebug = 4710;

        // [INHERITED] load a document and fill up the pagesVector
        Okular::Document::OpenResult loadDocumentWithPassword( const QString & fileName, QVector<Okular::Page*> & pagesVector, const QString & password );
        Okular::Document::OpenResult loadDocumentFromDataWithPassword( const QByteArray & fileData, QVector<Okular::Page*> & pagesVector, const QString & password );
        void loadPages(QVector<Okular::Page*> &pagesVector, int rotation=-1, bool clear=false);
        // [INHERITED] document information
        Okular::DocumentInfo generateDocumentInfo( const QSet<Okular::DocumentInfo::Key> &keys ) const;
        const Okular::DocumentSynopsis * generateDocumentSynopsis();
        Okular::FontInfo::List fontsForPage( int page );
        const QList<Okular::EmbeddedFile*> * embeddedFiles() const;
        PageSizeMetric pagesSizeMetric() const { return Pixels; }

        // [INHERITED] document information
        bool isAllowed( Okular::Permission permission ) const;

        // [INHERITED] perform actions on document / pages
        QImage image( Okular::PixmapRequest *page );

        // [INHERITED] print page using an already configured kprinter
        bool print( QPrinter& printer );

        // [INHERITED] reply to some metadata requests
        QVariant metaData( const QString & key, const QVariant & option ) const;

        // [INHERITED] reparse configuration
        bool reparseConfig();
        void addPages( KConfigDialog * );

        // [INHERITED] text exporting
        Okular::ExportFormat::List exportFormats() const;
        bool exportTo( const QString &fileName, const Okular::ExportFormat &format );

        // [INHERITED] print interface
        QWidget* printConfigurationWidget() const;

        // [INHERITED] save interface
        bool supportsOption( SaveOption ) const;
        bool save( const QString &fileName, SaveOptions options, QString *errorText );
        Okular::AnnotationProxy* annotationProxy() const;

    protected:
        bool doCloseDocument();
        Okular::TextPage* textPage( Okular::Page *page );

    protected slots:
        void requestFontData(const Okular::FontInfo &font, QByteArray *data);
        const Okular::SourceReference * dynamicSourceReference( int pageNr, double absX, double absY );
        Okular::Generator::PrintError printError() const;

    private:
        Okular::Document::OpenResult init(QVector<Okular::Page*> & pagesVector, const QString &password);

        // create the document synopsis hieracy
        void addSynopsisChildren( QDomNode * parentSource, QDomNode * parentDestination );
        // fetch annotations from the pdf file and add they to the page
        void addAnnotations( Poppler::Page * popplerPage, Okular::Page * page );
        // fetch the transition information and add it to the page
        void addTransition( Poppler::Page * popplerPage, Okular::Page * page );
        // fetch the form fields and add them to the page
        void addFormFields( Poppler::Page * popplerPage, Okular::Page * page );
        // load the source references from a pdfsync file
        void loadPdfSync( const QString & fileName, QVector<Okular::Page*> & pagesVector );
        // init the synctex parser if a synctex file exists
        void initSynctexParser( const QString& filePath );
        // search document for source reference
        void fillViewportFromSourceReference( Okular::DocumentViewport & viewport, const QString & reference ) const;

        Okular::TextPage * abstractTextPage(const QList<Poppler::TextBox*> &text, double height, double width, int rot);

        void resolveMediaLinkReferences( Okular::Page *page );
        void resolveMediaLinkReference( Okular::Action *action );

        bool setDocumentRenderHints();

        // poppler dependant stuff
        Poppler::Document *pdfdoc;


        // misc variables for document info and synopsis caching
        bool docSynopsisDirty;
        Okular::DocumentSynopsis docSyn;
        mutable bool docEmbeddedFilesDirty;
        mutable QList<Okular::EmbeddedFile*> docEmbeddedFiles;
        int nextFontPage;
        PopplerAnnotationProxy *annotProxy;
        QHash<Okular::Annotation*, Poppler::Annotation*> annotationsHash;

        QBitArray rectsGenerated;

        QPointer<PDFOptionsPage> pdfOptionsPage;
        
        synctex_scanner_t synctex_scanner;
        
        PrintError lastPrintError;
};

#endif

/* kate: replace-tabs on; indent-width 4; */

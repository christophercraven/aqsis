// Aqsis
// Copyright � 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Implements the default display devices for Aqsis.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include <aqsis.h>

#include <logging.h>
#include <logging_streambufs.h>
#include "sstring.h"

#include <tiffio.h>

using namespace Aqsis;

#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <fstream>
#include <float.h>

#include "ndspy.h"


#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#define	ZFILE_HEADER		"Aqsis ZFile" VERSION_STR
#else // AQSIS_SYSTEM_WIN32
#define ZFILE_HEADER "Aqsis ZFile" VERSION
#endif // !AQSIS_SYSTEM_WIN32
#define	SHADOWMAP_HEADER	"Shadow"

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#include	<version.h>
#endif

#include "display.h"

// From displayhelpers.c
#ifdef __cplusplus
extern "C" {
#endif
PtDspyError DspyReorderFormatting(int formatCount, PtDspyDevFormat *format, int outFormatCount, const PtDspyDevFormat *outFormat);
PtDspyError DspyFindStringInParamList(const char *string, char **result, int n, const UserParameter *p);
PtDspyError DspyFindIntInParamList(const char *string, int *result, int n, const UserParameter *p);
PtDspyError DspyFindMatrixInParamList(const char *string, float *result, int n, const UserParameter *p);
#ifdef __cplusplus
}
#endif

#define INT_MULT(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )
#define INT_PRELERP(p, q, a, t) ( (p) + (q) - INT_MULT( a, p, t) )

START_NAMESPACE( Aqsis )



void SaveAsShadowMap(const std::string& filename, SqDisplayInstance* image)
{
    TqChar version[ 80 ];
    TqInt twidth = 32;
    TqInt tlength = 32;

    const char* mode = (image->m_append)? "a" : "w";

    // Save the shadowmap to a binary file.
    if ( filename.compare( "" ) != 0 )
    {
        TIFF * pshadow = TIFFOpen( filename.c_str(), mode );
		if( pshadow != NULL )
		{
			// Set common tags
			TIFFCreateDirectory( pshadow );

	#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
			sprintf( version, "%s %s", STRNAME, VERSION_STR );
	#else
			sprintf( version, "%s %s", STRNAME, VERSION );
	#endif
			TIFFSetField( pshadow, TIFFTAG_SOFTWARE, ( uint32 ) version );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, image->m_matWorldToCamera );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, image->m_matWorldToScreen );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_TEXTUREFORMAT, SHADOWMAP_HEADER );
			TIFFSetField( pshadow, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );

			// Write the floating point image to the directory.
		#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
			sprintf( version, "%s %s", STRNAME, VERSION_STR );
		#else
			sprintf( version, "%s %s", STRNAME, VERSION );
		#endif
			TIFFSetField( pshadow, TIFFTAG_SOFTWARE, ( uint32 ) version );
			TIFFSetField( pshadow, TIFFTAG_IMAGEWIDTH, image->m_width );
			TIFFSetField( pshadow, TIFFTAG_IMAGELENGTH, image->m_height );
			TIFFSetField( pshadow, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
			TIFFSetField( pshadow, TIFFTAG_BITSPERSAMPLE, 32 );
			TIFFSetField( pshadow, TIFFTAG_SAMPLESPERPIXEL, image->m_iFormatCount );
			TIFFSetField( pshadow, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
			TIFFSetField( pshadow, TIFFTAG_TILEWIDTH, twidth );
			TIFFSetField( pshadow, TIFFTAG_TILELENGTH, tlength );
			TIFFSetField( pshadow, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
			TIFFSetField( pshadow, TIFFTAG_COMPRESSION, image->m_compression );


			TqInt tsize = twidth * tlength;
			TqInt tperrow = ( image->m_width + twidth - 1 ) / twidth;
			TqFloat* ptile = static_cast<TqFloat*>( _TIFFmalloc( tsize * sizeof( TqFloat ) ) );

			if ( ptile != NULL )
			{
				TqInt ctiles = tperrow * ( ( image->m_width + tlength - 1 ) / tlength );
				TqInt itile;
				for ( itile = 0; itile < ctiles; itile++ )
				{
					TqInt x = ( itile % tperrow ) * twidth;
					TqInt y = ( itile / tperrow ) * tlength;
					TqFloat* ptdata = reinterpret_cast<TqFloat*>(image->m_data) + ( ( y * image->m_width ) + x ) * image->m_iFormatCount;
					// Clear the tile to black.
					memset( ptile, 0, tsize * sizeof( TqFloat ) );
					for ( TqUlong i = 0; i < tlength; i++ )
					{
						for ( TqUlong j = 0; j < twidth; j++ )
						{
							if ( ( x + j ) < image->m_width && ( y + i ) < image->m_height )
							{
								TqInt ii;
								for ( ii = 0; ii < image->m_iFormatCount; ii++ )
									ptile[ ( i * twidth * image->m_iFormatCount ) + ( ( ( j * image->m_iFormatCount ) + ii ) ) ] = ptdata[ ( ( j * image->m_iFormatCount ) + ii ) ];
							}
						}
						ptdata += ( image->m_width * image->m_iFormatCount );
					}
					TIFFWriteTile( pshadow, ptile, x, y, 0, 0 );
				}
				TIFFWriteDirectory( pshadow );

			}

			TIFFClose( pshadow );
		}
    }
}


void WriteTIFF(const std::string& filename, SqDisplayInstance* image)
{
    uint16 photometric = PHOTOMETRIC_RGB;
    uint16 config = PLANARCONFIG_CONTIG;

    // Set common tags
    // If in "shadowmap" mode, write as a shadowmap.
	if( image->m_imageType == Type_Shadowmap )
	{
		SaveAsShadowMap(filename, image);
		return;
	}
    else if( image->m_imageType == Type_ZFile )
    {
        std::ofstream ofile( filename.c_str(), std::ios::out | std::ios::binary );
        if ( ofile.is_open() )
        {
            // Save a file type and version marker
            ofile << ZFILE_HEADER;

            // Save the xres and yres.
            ofile.write( reinterpret_cast<char* >( &image->m_width ), sizeof( image->m_width ) );
            ofile.write( reinterpret_cast<char* >( &image->m_height ), sizeof( image->m_height ) );

            // Save the transformation matrices.
            ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 0 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 1 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 2 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 3 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );

            ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 0 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 1 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 2 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 3 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );

            // Now output the depth values
            ofile.write( reinterpret_cast<char*>( image->m_data ), sizeof( TqFloat ) * ( image->m_width * image->m_height ) );
            ofile.close();
        }
		return;
    }
	
	TIFF* pOut = TIFFOpen( filename.c_str(), "w" );

    if ( pOut )
    {
        // Write the image to a tiff file.
        char version[ 80 ];

        int ExtraSamplesTypes[ 1 ] = {EXTRASAMPLE_ASSOCALPHA};

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
        sprintf( version, "%s %s", STRNAME, VERSION_STR );
#else
        sprintf( version, "%s %s", STRNAME, VERSION );
#endif

        bool use_logluv = false;

        TIFFSetField( pOut, TIFFTAG_SOFTWARE, ( uint32 ) version );
        TIFFSetField( pOut, TIFFTAG_IMAGEWIDTH, ( uint32 ) image->m_width );
        TIFFSetField( pOut, TIFFTAG_IMAGELENGTH, ( uint32 ) image->m_height );
        TIFFSetField( pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
        TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, image->m_iFormatCount );

        // Write out an 8 bits per pixel integer image.
        if ( image->m_format == PkDspyUnsigned8 )
        {
            TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 8 );
            TIFFSetField( pOut, TIFFTAG_PLANARCONFIG, config );
            TIFFSetField( pOut, TIFFTAG_COMPRESSION, image->m_compression );
            if ( image->m_compression == COMPRESSION_JPEG )
                TIFFSetField( pOut, TIFFTAG_JPEGQUALITY, image->m_quality );
            //if (description != "")
            //TIFFSetField(TIFFTAG_IMAGEDESCRIPTION, description);
            TIFFSetField( pOut, TIFFTAG_PHOTOMETRIC, photometric );
            TIFFSetField( pOut, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize( pOut, 0 ) );

            if ( image->m_iFormatCount == 4 )
                TIFFSetField( pOut, TIFFTAG_EXTRASAMPLES, 1, ExtraSamplesTypes );

            // Set the position tages in case we aer dealing with a cropped image.
            //TIFFSetField( pOut, TIFFTAG_XPOSITION, ( float ) g_CWXmin );
            //TIFFSetField( pOut, TIFFTAG_YPOSITION, ( float ) g_CWYmin );

            TqInt row;
            for ( row = 0; row < image->m_height; row++ )
            {
                if ( TIFFWriteScanline( pOut, reinterpret_cast<void*>(reinterpret_cast<char*>(image->m_data) + ( row * image->m_lineLength )), row, 0 ) < 0 )
                    break;
            }
            TIFFClose( pOut );
        }
        else
        {
            // Write out a floating point image.
            TIFFSetField( pOut, TIFFTAG_STONITS, ( double ) 1.0 );

            //			if(/* user wants logluv compression*/)
            //			{
            //				if(/* user wants to save the alpha channel */)
            //				{
            //					warn("SGI LogLuv encoding does not allow an alpha channel"
            //							" - using uncompressed IEEEFP instead");
            //				}
            //				else
            //				{
            //					use_logluv = true;
            //				}
            //
            //				if(/* user wants LZW compression*/)
            //				{
            //					warn("LZW compression is not available with SGI LogLuv encoding\n");
            //				}
            //			}

            if ( use_logluv )
            {
                /* use SGI LogLuv compression */
                TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
                TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 16 );
                TIFFSetField( pOut, TIFFTAG_COMPRESSION, COMPRESSION_SGILOG );
                TIFFSetField( pOut, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_LOGLUV );
                TIFFSetField( pOut, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_FLOAT );
            }
            else
            {
                /* use uncompressed IEEEFP pixels */
                TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
                TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 32 );
                TIFFSetField( pOut, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
                TIFFSetField( pOut, TIFFTAG_COMPRESSION, image->m_compression );
            }

            TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, image->m_iFormatCount );

            if ( image->m_iFormatCount == 4 )
                TIFFSetField( pOut, TIFFTAG_EXTRASAMPLES, 1, ExtraSamplesTypes );
            // Set the position tages in case we aer dealing with a cropped image.
            //TIFFSetField( pOut, TIFFTAG_XPOSITION, ( float ) g_CWXmin );
            //TIFFSetField( pOut, TIFFTAG_YPOSITION, ( float ) g_CWYmin );
            TIFFSetField( pOut, TIFFTAG_PLANARCONFIG, config );

            TqInt	linelen = image->m_width * image->m_iFormatCount;
            TqInt row = 0;
            for ( row = 0; row < image->m_height; row++ )
            {
                if ( TIFFWriteScanline( pOut, reinterpret_cast<void*>(reinterpret_cast<float*>(image->m_data) + ( row * linelen )), row, 0 ) < 0 )
                    break;
            }
            TIFFClose( pOut );
        }
    }
}



END_NAMESPACE( Aqsis )


PtDspyError DspyImageOpen(PtDspyImageHandle * image,
								   const char *drivername,
								   const char *filename,
								   int width,
								   int height,
								   int paramCount,
								   const UserParameter *parameters,
								   int iFormatCount,
								   PtDspyDevFormat *format,
								   PtFlagStuff *flagstuff)
{
	SqDisplayInstance* pImage;
	
	pImage = new SqDisplayInstance;	

	if(pImage)
	{
		// Store the instance information so that on re-entry we know which display is being referenced.
		pImage->m_height = height;
		pImage->m_width = width;
		// Determine the display type from the list that we support.
		if(strcmp(drivername, "file")==0 || strcmp(drivername, "tiff")==0)
			pImage->m_imageType = Type_File;
		else if(strcmp(drivername, "framebuffer")==0)
			pImage->m_imageType = Type_Framebuffer;
		else if(strcmp(drivername, "zfile")==0)
			pImage->m_imageType = Type_ZFile;
		else if(strcmp(drivername, "zframebuffer")==0)
			pImage->m_imageType = Type_ZFramebuffer;
		else if(strcmp(drivername, "shadow")==0)
			pImage->m_imageType = Type_Shadowmap;
		pImage->m_iFormatCount = iFormatCount;

		*image = pImage;

		pImage->m_filename = new char[strlen(filename)+1];
		strcpy(pImage->m_filename, filename);

		// Scan the formats table to see what the widest channel format specified is.
		TqInt widestFormat = PkDspySigned8;
		TqInt i;
		for(i=0; i<iFormatCount; i++)
			if(format[i].type < widestFormat)
				widestFormat = format[i].type;

		if(widestFormat == PkDspySigned8) widestFormat = PkDspyUnsigned8;
		else if(widestFormat == PkDspySigned16) widestFormat = PkDspyUnsigned16;
		else if(widestFormat == PkDspySigned32)	widestFormat = PkDspyUnsigned32;

		// Create and initialise a byte array if rendering 8bit image, or we are in framebuffer mode
		if(pImage->m_imageType == Type_Framebuffer)
		{
			pImage->m_data = new unsigned char[ pImage->m_width * pImage->m_height * pImage->m_iFormatCount ];
			pImage->m_entrySize = pImage->m_iFormatCount * sizeof(char);

			// Initialise the display to a checkerboard to show alpha
			for (TqInt i = 0; i < pImage->m_height; i ++) 
			{
				for (TqInt j = 0; j < pImage->m_width; j++)
				{
					int     t       = 0;
					unsigned char d = 255;

					if ( ( (pImage->m_height - 1 - i) & 31 ) < 16 ) t ^= 1;
					if ( ( j & 31 ) < 16 ) t ^= 1;

					if ( t )
					{
						d      = 128;
					}
					reinterpret_cast<unsigned char*>(pImage->m_data)[pImage->m_iFormatCount * (i*pImage->m_width + j) ] = d;
					reinterpret_cast<unsigned char*>(pImage->m_data)[pImage->m_iFormatCount * (i*pImage->m_width + j) + 1] = d;
					reinterpret_cast<unsigned char*>(pImage->m_data)[pImage->m_iFormatCount * (i*pImage->m_width + j) + 2] = d;
				}
			}
			widestFormat = PkDspyUnsigned8;

			pImage->m_theWindow = new Fl_Window(pImage->m_width, pImage->m_height);
			pImage->m_uiImageWidget = new Fl_FrameBuffer_Widget(0,0, pImage->m_width, pImage->m_height, pImage->m_iFormatCount, reinterpret_cast<unsigned char*>(pImage->m_data));
			pImage->m_theWindow->resizable(pImage->m_uiImageWidget);
			pImage->m_theWindow->end();
			pImage->m_theWindow->show();
		}
        else
		{
			// Determine the appropriate format to save into.
			if(widestFormat == PkDspyUnsigned8)
			{
				pImage->m_data = malloc( pImage->m_width * pImage->m_height * pImage->m_iFormatCount * sizeof(unsigned char));
				pImage->m_entrySize = pImage->m_iFormatCount * sizeof(char);
			}
			else if(widestFormat == PkDspyUnsigned16)
			{
				pImage->m_data = malloc( pImage->m_width * pImage->m_height * pImage->m_iFormatCount * sizeof(unsigned short));
				pImage->m_entrySize = pImage->m_iFormatCount * sizeof(short);
			}
			else if(widestFormat == PkDspyUnsigned32)
			{
				pImage->m_data = malloc( pImage->m_width * pImage->m_height * pImage->m_iFormatCount * sizeof(unsigned long));
				pImage->m_entrySize = pImage->m_iFormatCount * sizeof(long);
			}
			else if(widestFormat == PkDspyFloat32)
			{
				pImage->m_data = malloc( pImage->m_width * pImage->m_height * pImage->m_iFormatCount * sizeof(float));
				pImage->m_entrySize = pImage->m_iFormatCount * sizeof(float);
			}
		}
		pImage->m_lineLength = pImage->m_entrySize * pImage->m_width;
		pImage->m_format = widestFormat;

		// If we are recieving "rgba" data, ensure that it is in the correct order.
		PtDspyDevFormat outFormat[] = 
		{
			{"r", widestFormat},
			{"g", widestFormat},
			{"b", widestFormat},
			{"a", widestFormat},
		};
		DspyReorderFormatting(iFormatCount, format, MIN(iFormatCount,4), outFormat);

		// Extract any important data from the user parameters.
		char* compression;
		if( DspyFindStringInParamList("compression", &compression, paramCount, parameters ) == PkDspyErrorNone )
		{
            if ( strstr( compression, "none" ) != 0 )
                pImage->m_compression = COMPRESSION_NONE;
            else if ( strstr( compression, "lzw" ) != 0 )
                pImage->m_compression = COMPRESSION_LZW;
            else if ( strstr( compression, "deflate" ) != 0 )
                pImage->m_compression = COMPRESSION_DEFLATE;
            else if ( strstr( compression, "jpeg" ) != 0 )
                pImage->m_compression = COMPRESSION_JPEG;
            else if ( strstr( compression, "packbits" ) != 0 )
                pImage->m_compression = COMPRESSION_PACKBITS;
		}
		int quality;
		if( DspyFindIntInParamList("quality", &quality, paramCount, parameters ) == PkDspyErrorNone )
			pImage->m_quality = quality;
		// Extract the transformation matrices if they are there.
		DspyFindMatrixInParamList( "NP", reinterpret_cast<float*>(pImage->m_matWorldToScreen), paramCount, parameters );
		DspyFindMatrixInParamList( "Nl", reinterpret_cast<float*>(pImage->m_matWorldToCamera), paramCount, parameters );
	}
	else
		return(PkDspyErrorNoMemory);

	return(PkDspyErrorNone);	
}


PtDspyError DspyImageData(PtDspyImageHandle image,
								   int xmin,
								   int xmaxplus1,
								   int ymin,
								   int ymaxplus1,
								   int entrysize,
								   const unsigned char *data)
{
	SqDisplayInstance* pImage;
	pImage = reinterpret_cast<SqDisplayInstance*>(image);

	const unsigned char* pdatarow = data;
	TqInt bucketlinelen = entrysize * (xmaxplus1 - xmin);

	if( pImage && data && xmin >= 0 && ymin >= 0 && xmaxplus1 <= pImage->m_width && ymaxplus1 <= pImage->m_height )
	{
		TqInt y;
		for ( y = ymin; y < ymaxplus1; y++ )
		{
			// Copy a whole row at a time, as we know it is being sent in the proper format and order.
			TqInt so = ( y * pImage->m_lineLength ) + ( xmin * pImage->m_entrySize );
			memcpy(reinterpret_cast<char*>(pImage->m_data)+so, reinterpret_cast<const void*>(pdatarow), bucketlinelen);
			pdatarow += bucketlinelen;
		}
	}	

	if(pImage->m_imageType == Type_Framebuffer)
	{
		pImage->m_uiImageWidget->damage(1);
		Fl::check();
	}
	return(PkDspyErrorNone);	
}


PtDspyError DspyImageClose(PtDspyImageHandle image)
{
	SqDisplayInstance* pImage;
	pImage = reinterpret_cast<SqDisplayInstance*>(image);
	
	// Write the image to disk
	if( pImage->m_imageType == Type_File || 
		pImage->m_imageType == Type_ZFile || 
		pImage->m_imageType == Type_Shadowmap )
		WriteTIFF( pImage->m_filename, pImage);

	// Delete the image structure.
	free(pImage->m_data);
	delete[](pImage->m_filename);
	delete(pImage);

	return(PkDspyErrorNone);	
}


PtDspyError DspyImageQuery(PtDspyImageHandle image,
									PtDspyQueryType type,
									int size,
									void *data)
{
	SqDisplayInstance* pImage;
	pImage = reinterpret_cast<SqDisplayInstance*>(image);

    PtDspyOverwriteInfo overwriteInfo;
    PtDspySizeInfo sizeInfo;

	if(size <= 0 || !data)
		return PkDspyErrorBadParams;

	switch (type)
	{
		case PkOverwriteQuery:
			if (size > sizeof(overwriteInfo))
				size = sizeof(overwriteInfo);
			overwriteInfo.overwrite = 1;
			overwriteInfo.interactive = 0;
			memcpy(data, &overwriteInfo, size);
			break;
		case PkSizeQuery:
			if (size > sizeof(sizeInfo))
				size = sizeof(sizeInfo);
			if(pImage)
			{
				if(!pImage->m_width || !pImage->m_height)
				{
					pImage->m_width = 640;
					pImage->m_height = 480;
				}
				sizeInfo.width = pImage->m_width;
				sizeInfo.height = pImage->m_height;
				sizeInfo.aspectRatio = 1.0f;
			}
			else
			{
				sizeInfo.width = 640;
				sizeInfo.height = 480;
				sizeInfo.aspectRatio = 1.0f;
			}
			memcpy(data, &sizeInfo, size);
			break;
		default:
			return PkDspyErrorUnsupported;
    }

	return(PkDspyErrorNone);	
}

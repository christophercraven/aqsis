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


/** \file	log.h
		\brief The Log class
		\author Matth�us G. Chajdas (Matthaeus@darkside-conflict.net)
*/


//? Is .h included already?
#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED 1

#include <stdio.h>
#include <log4cpp/Portability.hh>
#include <iostream>
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>

#include "imtable.h"
#include "mtable.h"		//!TODO: Remove this as soon as IqMessageTable is used everywhere

#include "ilog.h"
#include "aqsis.h"

#define	 _qShareName	BUILD_LIBAQSISTYPES
#include "share.h"

namespace log4cpp
{

	class AqLayout : public Layout  
	{
	public:
		AqLayout();
		virtual ~AqLayout();

		virtual std::string format(const LoggingEvent& event);


	};

	class AqFileLayout : public Layout  
	{
	public:
		AqFileLayout();
		virtual ~AqFileLayout();

		virtual std::string format(const LoggingEvent& event);


	};
}

START_NAMESPACE( Aqsis )

class _qShareC CqLog	: public IqLog
{
	public:
		_qShareM CqLog( char* name = "AqsisLog", bool noConsoleOutput = false );
		_qShareM virtual ~CqLog();

		_qShareM void addFileLog( std::string filename = "render.log", std::string name = "AqsisFileLog" );
		_qShareM void removeFileLog( std::string name );
		
		_qShareM void setMessageTable( IqMessageTable* pTable );
		_qShareM IqMessageTable* getMessageTable();

		_qShareM void log( const char* priority, const char* stringFormat, ... );
		_qShareM void log( const char* priority, const CqString &stringFormat );
		
		/** Log an error message
		 */
		_qShareM void error( int table, int error_id );
		_qShareM void error( const char* stringFormat, ... );
		_qShareM void error( const CqString &string );

		/** Log an warning
		 */
		_qShareM void warn( int table, int error_id );
		_qShareM void warn( const char* stringFormat, ... );
		_qShareM void warn( const CqString &string );

		/** Log an critical error message
		 */
		_qShareM void critical( int table, int error_id );
		_qShareM void critical( const char* stringFormat, ... );
		_qShareM void critical( const CqString &string );

		/** Log an notice message
		 */
		_qShareM void notice( int table, int error_id );
		_qShareM void notice( const char* stringFormat, ... );
		_qShareM void notice( const CqString &string );
		
		/** Log an info message
		 */
		_qShareM void info( int table, int error_id );
		_qShareM void info( const char* stringFormat, ... );
		_qShareM void info( const CqString &string );
		
		/** Log an fatal error
		 */
		_qShareM void fatal( int table, int error_id );
		_qShareM void fatal( const char* stringFormat, ... );
		_qShareM void fatal( const CqString &string );
		
		/** Log an debug message
		 */
		_qShareM void debug( const char* stringFormat, ... );
		_qShareM void debug( const CqString &string );

		_qShareM const char* getError( int table, int error_id );

		enum {	BASIC_ERROR_TABLE = 0,
				RI_ERROR_TABLE } m_ErrorTables;

		enum {	UNKNOW_ERROR = 0,
				RI_COLOR_SAMPLES_INVALID,
				RI_RELATIVE_DETAIL_INVALID,
				RI_UNKNOWN_SYMBOL,
				RI_AREA_LIGHT_UNSUPPORTED,
				RI_INTERIOR_UNSUPPORTED,							// 5
				RI_EXTERIOR_UNSUPPORTED,
				RI_DETAIL_RANGE_INVALID,
				RI_GEOMETRIC_APPROX_UNSUPPORTED,
				RI_PERSPECTIVE_BAD_FOV,
				RI_DEFORMATION_UNSUPPORTED,							//10
				RI_TRANSFORM_POINTS_UNSUPPORTED,					
				RI_BLOBBY_V_UNSUPPORTED,
				RI_CURVES_V_UNKNOWN_WRAP_MODE,
				RI_CURVES_V_UNKNOWN_TYPE,
				RI_PROC_DELAYED_READ_ARCHIVE_UNSUPPORTED,			//15
				RI_PROC_RUN_PROGRAM_UNSUPPORTED,					
				RI_PROC_DYNAMIC_LOAD_UNSUPPORTED,
				RI_PROCEDURAL_UNKNOWN_SUBDIV,
				RI_GEOMETRY_V_UNKNOWN,
				RI_OBJECT_BEGIN_UNSUPPORTED,						//20
				RI_OBJECT_END_UNSUPPORTED,						
				RI_MAKE_BUMP_UNSUPPORTED,
				RI_ONLY_UNIFORM_OPTIONS,
				RI_ONLY_UNIFORM_ATTRIBUTES,
				RI_INVALID_SHADING_INTERPOLATION,
				RI_DEGENRATE_POLYGON,								//25
				RI_MAKE_CUBE_ENV_WRONG_SIZE,
				RI_SDS_NONMANIFOLD
															} m_RiErrorTable;	

				
	private:
		
		void createFileLog( std::string filename, std::string name );
		void createCOUTLog( std::string name = "AqsisConsoleLog" );

		void log2( log4cpp::Priority::Value priority, const char* stringFormat, va_list va );
		void log2( log4cpp::Priority::Value priority, const char* string );

		log4cpp::Appender* m_pAppender;
		log4cpp::Appender* m_pFileAppender;
		log4cpp::AqLayout* m_pLayout;
		log4cpp::AqFileLayout* m_pFileLayout;
		log4cpp::Category* m_pRoot;

		IqMessageTable* m_pTable;
};

END_NAMESPACE( Aqsis )

#endif


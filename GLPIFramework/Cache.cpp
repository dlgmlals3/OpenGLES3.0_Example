#ifdef _WIN32
#endif

#include "Cache.h"

#include <string.h>
#include <fstream>
using std::ifstream;
#include <sstream>
using std::ostringstream;
#include "GLutils.h"

/*!
	Open/Extract a file from disk and load it in cachePtr.

	\param[in] filename The file to load in cachePtr.
	\param[in] relative_path Determine if the filename is an absolute or relative path.

	\return Return a CACHE structure pointer if the file is found and loaded, instead will return
	NULL.
*/
CACHE *reserveCache( char *filename, unsigned char relative_path )
{
    char fpath[ MAX_PATH ] = {""},
         fname[ MAX_PATH ] = {""};

    unzFile		    uf;
    unz_file_info   fi;
    unz_file_pos    fp;
    strcpy( fpath, getenv( "FILESYSTEM" ) );
    uf = unzOpen( fpath );

    if( !uf ) return NULL;

    if( relative_path ) sprintf( fname, "assets/%s", filename );
    else strcpy( fname, filename );

    unzGoToFirstFile( uf );

    CACHE *cachePtr = ( CACHE * ) calloc( 1, sizeof( CACHE ) );

    unzGetFilePos( uf, &fp );

    if( unzLocateFile( uf, fname, 1 ) == UNZ_OK )
    {
        unzGetCurrentFileInfo(  uf,
                               &fi,
                                cachePtr->filename,
                                MAX_PATH,
                                NULL, 0,
                                NULL, 0 );

        if( unzOpenCurrentFilePassword( uf, NULL ) == UNZ_OK )
        {
            cachePtr->position = 0;
            cachePtr->size	 = fi.uncompressed_size;
            cachePtr->buffer   = ( unsigned char * ) realloc( cachePtr->buffer, fi.uncompressed_size + 1 );
            cachePtr->buffer[ fi.uncompressed_size ] = 0;

            while( unzReadCurrentFile( uf, cachePtr->buffer, fi.uncompressed_size ) > 0 ){}

            unzCloseCurrentFile( uf );

            unzClose( uf );
            return cachePtr;
        }
    }

    unzClose( uf );

    LOGI("Error: as");
    return NULL;
}

/*!
	Delete initialized CACHE.

	\param[in,out] cachePtr A valid CACHE structure object.

	\return Return a NULL CACHE pointer.
*/
CACHE *freeCache( CACHE *cachePtr )
{
	if( cachePtr->buffer ) free( cachePtr->buffer );

	free( cachePtr );
	return NULL;
}
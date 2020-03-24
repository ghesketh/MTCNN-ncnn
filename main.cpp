//
// main.cpp
//
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <intrin.h> // stb_image
#endif // _MSC_VER

#include "main.h"

#include <chrono>

#ifdef _WIN32
#define SDL_MAIN_HANDLED
//#include <SDL_image.h>
#else
//#include <SDL2/SDL_image.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
//
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
//
#include "stb_image.h"
//
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

typedef struct client_options
{
	char * pBitmap ;

	float fScaleStep ;

	float fSizeMax ;
	float fSizeMin ;

} client_options_t ;


int SetOptions( client_options_t * pOptions, int iArgs, char * pArgs[] )
{
	int iResult = 0 ;

	do
	{
		memset( pOptions, 0, sizeof( *( pOptions ) ) ) ;

		pOptions->fSizeMin = -1.f ;

		if( 2 > iArgs )
		{
			iResult = -1 ;
			break ;
		}

		for( int iArg = 1 ; iArg < iArgs ; ++( iArg ) )
		{
			if( '-' == pArgs[ iArg ][ 0 ] )
			{
				switch( pArgs[ iArg ][ 1 ] )
				{
					case 'n':
						if( ++( iArg ) < iArgs )
						{
							pOptions->fSizeMin = strtof( pArgs[ iArg ], nullptr ) ;
						}
						break ;

					case 's':
						if( ++( iArg ) < iArgs )
						{
							pOptions->fScaleStep = strtof( pArgs[ iArg ], nullptr ) ;
						}
						break ;

					case 'x':
						if( ++( iArg ) < iArgs )
						{
							pOptions->fSizeMax = strtof( pArgs[ iArg ], nullptr ) ;
						}
						break ;

					default:
						break ;

				} // switch

			}
			else
			{
				pOptions->pBitmap = pArgs[ iArg ] ;
			}

		} // for ... iArg

	}
	while( 0 ) ;

	return iResult ;

} // SetOptions()


int main( int iArgs, char * pArgs[] )
{
	int iResult = 0 ;

	do
	{
		if( 2 > iArgs )
		{
			iResult = -400 ;
			break ;
		}

		detect_options_t DetectOptions ;

		client_options_t ClientOptions ;
		//
		SetOptions( &( ClientOptions ), iArgs, pArgs ) ;

		DetectOptions.pPixels = stbi_load( ClientOptions.pBitmap, &( DetectOptions.iPixelW ), &( DetectOptions.iPixelH ), &( DetectOptions.iPixelD ), 0 ) ;

		//fprintf( stderr, ". stbi_load( %s )\n", pArgs[ 1 ] ) ;

		if( nullptr == DetectOptions.pPixels || 3 != DetectOptions.iPixelD )
		{
			iResult = -400 ;
			break ;
		}

		do
		{
			DetectOptions.iStride = ( DetectOptions.iPixelD * DetectOptions.iPixelW ) ;

			DetectOptions.fScaleStep = ClientOptions.fScaleStep ;

			DetectOptions.fSizeMax = ClientOptions.fSizeMax ;
			DetectOptions.fSizeMin = ClientOptions.fSizeMin ;

			DetectOptions.pMethod = client_exec ;

			DetectOptions.pOpaque = client_init( ClientOptions.pBitmap ) ;

			auto pDetect = detect_init() ;

			if( nullptr == pDetect )
			{
				iResult = -500 ;
				break ;
			}

			do
			{
				const auto start = std::chrono::steady_clock::now() ;
				//
				iResult = detect_exec( pDetect, &( DetectOptions ) ) ;
				//
				const auto milli = std::chrono::duration< double, std::milli >( std::chrono::steady_clock::now() - start ).count() ;
				//
				printf( "\n" "%.3f ms" "\n", milli ) ;
			}
			while( 0 ) ;

			detect_exit( pDetect ) ;

			client_exit( DetectOptions.pOpaque ) ;

		}
		while( 0 ) ;

		free( DetectOptions.pPixels ) ;

	}
	while( 0 ) ;

	return iResult ;

} // main()

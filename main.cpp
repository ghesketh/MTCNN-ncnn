//
// main.cpp
//
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <intrin.h> // stb_image
#endif // _MSC_VER

#include "mtcnn.h"

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

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#if( defined( SDL_MAJOR_VERSION ) && 2 == SDL_MAJOR_VERSION )
#ifdef _MSC_VER
#pragma comment( lib, "SDL2" )
#pragma comment( lib, "SDL2_image" )
#endif
#endif // SDL

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


#if( defined( SDL_MAJOR_VERSION ) && 2 == SDL_MAJOR_VERSION )
class Canvas
{
private:

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#endif

	struct
	{
		SDL_Renderer * pRenderer = nullptr ;
		SDL_Surface * pSurface = nullptr ;
		SDL_Window * pWindow = nullptr ;

		float fScale ;

	} _ ;

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

public:
	~Canvas()
	{
		if( nullptr != _.pRenderer )
		{
			SDL_DestroyRenderer( _.pRenderer ) ;
		}

		if( nullptr != _.pWindow )
		{
			SDL_DestroyWindow( _.pWindow ) ;
		}

		SDL_Quit() ;

		if( nullptr != _.pSurface )
		{
			SDL_FreeSurface( _.pSurface ) ;
		}

		IMG_Quit() ;

	} // ~Canvas()


	Canvas( const char * pFileName )
	{
		int iResult ;

		do
		{
			iResult = IMG_Init( IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF ) ;

			if( 0 == iResult )
			{
				fprintf( stderr, "! IMG_Init() : %i" "\n", iResult ) ;
				break ;
			}

			_.pSurface = IMG_Load( pFileName ) ;

			if( nullptr == _.pSurface )
			{
				fprintf( stderr, "! IMG_Load( %s )" "\n", pFileName ) ;
				break ;
			}

			iResult = SDL_Init( SDL_INIT_VIDEO ) ;

			if( 0 != iResult )
			{
				fprintf( stderr, "! SDL_Init() : %i" "\n", iResult ) ;
				break ;
			}

			SDL_Rect Target ;

			iResult = SDL_GetDisplayBounds( 0, &( Target ) ) ;

			if( 0 != iResult )
			{
				fprintf( stderr, "! SDL_GetDisplayBounds() : %i" "\n", iResult ) ;
				break ;
			}

			float fSource = ( float( _.pSurface->w ) / float ( _.pSurface->h ) ) ;

			float fTarget = ( float( Target.w ) / float ( Target.h ) ) ;

			if( fSource > fTarget )
			{
				// aspect ratio of bitmap is wider than aspect ratio of screen
				//
				Target.h = decltype( Target.h ) ( Target.w / fSource ) ;

				_.fScale = ( float( Target.h ) / float( _.pSurface->h ) ) ;
			}
			else
			{
				// aspect ratio of screen is wider than aspect ratio of bitmap
				//
				Target.w = decltype( Target.w ) ( Target.h * fSource ) ;

				_.fScale = ( float( Target.w ) / float( _.pSurface->w ) ) ;
			}

			Target.x = 0 ;
			Target.y = 0 ;

			_.pWindow = SDL_CreateWindow( pFileName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Target.w, Target.h, SDL_WINDOW_BORDERLESS ) ;

			if( nullptr == _.pWindow )
			{
				fprintf( stderr, "! SDL_CreateWindow()" "\n" ) ;
				break ;
			}

			_.pRenderer = SDL_CreateRenderer( _.pWindow, -1, 0 ) ;

			if( nullptr == _.pRenderer )
			{
				fprintf( stderr, "! SDL_CreateWindow()" "\n" ) ;
				break ;
			}

			do
			{
				SDL_Texture * pTexture = SDL_CreateTextureFromSurface( _.pRenderer, _.pSurface ) ;

				if( nullptr == pTexture )
				{
					fprintf( stderr, "! SDL_CreateTextureFromSurface()" "\n" ) ;
					break ;
				}

				do
				{
					iResult = SDL_RenderCopy( _.pRenderer, pTexture, nullptr, &( Target ) ) ;

					if( 0 != iResult )
					{
						fprintf( stderr, "! SDL_RenderCopy() : %i" "\n", iResult ) ;
						break ;
					}

					SDL_RenderPresent( _.pRenderer ) ;

				}
				while( 0 ) ;

				SDL_DestroyTexture( pTexture ) ;

			}
			while( 0 ) ;

			iResult = SDL_SetRenderDrawColor( _.pRenderer, 0, 255, 0, 255 ) ;

			if( 0 != iResult )
			{
				fprintf( stderr, "! SDL_SetRenderDrawColor() : %i" "\n", iResult ) ;
				break ;
			}

		}
		while( 0 ) ;

	} // Canvas()


	Canvas * draw( int32_t x, int32_t y, int32_t w, int32_t h )
	{
		int iResult ;

		do
		{
			if( nullptr == _.pRenderer )
			{
				break ;
			}

			SDL_Rect Rect ;
			//
			Rect.x = int32_t( lround( x * _.fScale ) ) ;
			Rect.y = int32_t( lround( y * _.fScale ) ) ;
			Rect.w = int32_t( lround( w * _.fScale ) ) ;
			Rect.h = int32_t( lround( h * _.fScale ) ) ;

			iResult = SDL_RenderDrawRect( _.pRenderer, &( Rect ) ) ;

			if( 0 != iResult )
			{
				fprintf( stderr, "! SDL_RenderDrawRect() : %i" "\n", iResult ) ;
				break ;
			}

		}
		while( 0 ) ;

		return this ;

	} // draw()


	Canvas * show( void )
	{
		do
		{
			if( nullptr == _.pRenderer )
			{
				break ;
			}

			SDL_RenderPresent( _.pRenderer ) ;

		}
		while( 0 ) ;

		return this ;

	} // show()


	Canvas * wait( void )
	{
		SDL_Event Event ;

		do
		{
			SDL_WaitEvent( &( Event ) ) ;

		}
		while( SDL_KEYDOWN != Event.type && SDL_QUIT != Event.type ) ;

		return this ;

	} // wait()

} ; // class Canvas

#endif // SDL


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
//
static void ObjectCallback( Object_t * pObject, void * pOpaque = nullptr )
{
	static int iResult = 0 ;

	do
	{
		printf( "\n" "Object[ %4i ]" "\n", iResult ) ;
		//
		printf( "\t" "fEyeR = %8.2f, %8.2f" "\n", double( pObject->Points[ 0 ].X ), double( pObject->Points[ 0 ].Y ) ) ;
		printf( "\t" "fEyeL = %8.2f, %8.2f" "\n", double( pObject->Points[ 1 ].X ), double( pObject->Points[ 1 ].Y ) ) ;
		//
		printf( "\t" "fNose = %8.2f, %8.2f" "\n", double( pObject->Points[ 2 ].X ), double( pObject->Points[ 2 ].Y ) ) ;
		//
		printf( "\t" "fLipR = %8.2f, %8.2f" "\n", double( pObject->Points[ 3 ].X ), double( pObject->Points[ 3 ].Y ) ) ;
		printf( "\t" "fLipL = %8.2f, %8.2f" "\n", double( pObject->Points[ 4 ].X ), double( pObject->Points[ 4 ].Y ) ) ;

		++( iResult ) ;

#if( defined( SDL_MAJOR_VERSION ) && 2 == SDL_MAJOR_VERSION )

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

		auto pCanvas = ( Canvas * ) pOpaque ;

		pCanvas->draw( pObject->Rect.L, pObject->Rect.T, pObject->Rect.R - pObject->Rect.L + 1, pObject->Rect.B - pObject->Rect.T - 1 ) ;
		pCanvas->show()->wait() ;

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif // SDL

	}
	while( 0 ) ;

} // ObjectCallback()

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


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

			DetectOptions.pMethod = ObjectCallback ;
#if( defined( SDL_MAJOR_VERSION ) && 2 == SDL_MAJOR_VERSION )
			Canvas canvas( ClientOptions.pBitmap ) ;

			DetectOptions.pOpaque = &( canvas ) ;
#else
			DetectOptions.pOpaque = nullptr ;
#endif

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

#if( defined( SDL_MAJOR_VERSION ) && 2 == SDL_MAJOR_VERSION )
			canvas.wait() ;
#endif // SDL

		}
		while( 0 ) ;

		free( DetectOptions.pPixels ) ;

	}
	while( 0 ) ;

	return iResult ;

} // main()

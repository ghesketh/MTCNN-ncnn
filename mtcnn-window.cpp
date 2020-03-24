//
// mtcnn-stdout.c
//
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif // _MSC_VER

#include "main.h"

#include <stdio.h>

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#include <SDL_image.h>
#else
#include <SDL2/SDL_image.h>
#endif
//
#ifdef _MSC_VER
#pragma comment( lib, "SDL2" )
#pragma comment( lib, "SDL2_image" )

#pragma comment( lib, "mtcnn" )
#endif

class client_t
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
	~client_t()
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

	} // ~client_t()


	client_t( const char * pFileName )
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

	} // client_t()


	client_t * draw( int32_t x, int32_t y, int32_t w, int32_t h )
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


	client_t * show( void )
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


	client_t * wait( void )
	{
		SDL_Event Event ;

		do
		{
			SDL_WaitEvent( &( Event ) ) ;

		}
		while( SDL_KEYDOWN != Event.type && SDL_QUIT != Event.type ) ;

		return this ;

	} // wait()

} ; // class client_t


void client_exec( Object_t * pObject, void * pOpaque )
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

	auto pClient = ( client_t * ) pOpaque ;

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

	pClient->draw( pObject->Rect.L, pObject->Rect.T, pObject->Rect.R - pObject->Rect.L + 1, pObject->Rect.B - pObject->Rect.T - 1 ) ;
	pClient->show() ;

} // client_exec()


void client_exit( void * pOpaque )
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

	auto pClient = ( client_t * ) pOpaque ;

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

	pClient->wait() ;

	delete pClient ;

} // client_exit()


void * client_init( void * pOpaque )
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

	auto pFilename = ( char * ) pOpaque ;

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

	return ( void * ) new client_t( pFilename ) ;

} // client_init()

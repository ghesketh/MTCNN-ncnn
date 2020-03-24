//
// mtcnn.h
//
#pragma once

#include <stdint.h>

namespace MTCNN
{
	template< typename TYPE >
	struct POINT
	{
		TYPE X ;
		TYPE Y ;

	} ;

	template< typename TYPE >
	struct RECT
	{
		TYPE L ;
		TYPE T ;
		TYPE R ;
		TYPE B ;

	} ;

} // namespace MTCNN


typedef struct
{
	MTCNN::RECT< int32_t > Rect ;
	MTCNN::RECT< float > Bias ;
	MTCNN::POINT< float > Points[ 5 ] ;

	int32_t area ;
	float score ;

} Object_t ;


extern "C"
{
	typedef void ( * ObjectCallback_t )( Object_t * pObject, void * pOpaque ) ;
}


typedef struct
{
	ObjectCallback_t pMethod ;
	void * pOpaque ;

	unsigned char * pPixels ;

	int32_t iPixelD ;
	int32_t iPixelH ;
	int32_t iPixelW ;
	int32_t iStride ;

	float fScaleStep ;

	float fSizeMax ;
	float fSizeMin ;

} detect_options_t ;

typedef struct Detect_t * detect_handle_t ;

int detect_exec( detect_handle_t pDetect, detect_options_t * pOptions ) ;

detect_handle_t detect_exit( detect_handle_t pDetect ) ;

detect_handle_t detect_init() ;

//
// mtcnn-stdout.c
//
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif // _MSC_VER

#include "main.h"

#include <stdio.h>

#ifdef _MSC_VER
#pragma comment( lib, "libmtcnn" )
#endif

typedef struct
{
} client_t ;


void client_exec( Object_t * pObject, void * )
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

	}
	while( 0 ) ;

} // client_exec()


void client_exit( void * )
{
	return ;

} // client_exit()


void * client_init( void * )
{
	return nullptr ;

} // client_init()

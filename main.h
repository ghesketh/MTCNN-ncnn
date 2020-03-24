//
// main.h
//
#pragma once

#include "mtcnn.h"

extern "C"
{
	extern void * client_init( void * ) ;
	extern void client_exec( Object_t *, void * ) ;
	extern void client_exit( void * ) ;
}

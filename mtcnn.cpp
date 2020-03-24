//
// mtcnn.cpp
//
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif // _MSC_VER

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>

#else
#include <unistd.h>

#endif

#include "mtcnn.h"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

#include <ncnn/net.h>
//
#ifdef _MSC_VER
#pragma comment( lib, "ncnn" )
#endif // _MSC_VER

#define MTCNN_OBJECT_CALLBACKS

extern "C"
{
	extern const unsigned char mtcnn_p_model[] ;
	extern const unsigned char mtcnn_r_model[] ;
	extern const unsigned char mtcnn_o_model[] ;

	extern const char mtcnn_p_param[] ;
	extern const char mtcnn_r_param[] ;
	extern const char mtcnn_o_param[] ;

} // extern "C"


typedef std::vector< Object_t > Objects_t ;


namespace MTCNN
{
	const float fModelSize = 12.f ;

	enum IOU_MODEL
	{
		Min,
		IoU

	} ; // IOU_MODEL

	template< typename TYPE >
	TYPE MAX( TYPE a, TYPE b )
	{
		return ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) ) ;
	}

	template< typename TYPE >
	TYPE MIN( TYPE a, TYPE b )
	{
		return ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) ) ;
	}

	template< typename TYPE >
	struct SIZE
	{
		TYPE W ;
		TYPE H ;

	} ;

//#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
//#define MIN( a, b ) ( ( ( a ) > ( b ) ) ? ( b ) : ( a ) )

} // namespace MTCNN


typedef struct Detect_t
{
	ncnn::Net mtcnn_p, mtcnn_r, mtcnn_o ;

	// default to defaults
	//
	float fScaleStep = -1.f ;

	float fSizeMax = -1.f ;
	float fSizeMin = -1.f ;

} Detect_t ;


// IncreasingScore()
//
static bool IncreasingScore( Object_t & a, Object_t & b )
{
	return ( a.score < b.score ) ;

} // IncreasingScore()


// Intersection()
//
template< typename TYPE >
static TYPE Intersection( MTCNN::RECT< TYPE > A, MTCNN::RECT< TYPE > B )
{
	MTCNN::RECT< TYPE > I ;
	//
	I.L = MTCNN::MAX< TYPE >( A.L, B.L ) ;
	I.T = MTCNN::MAX< TYPE >( A.T, B.T ) ;
	I.R = MTCNN::MIN< TYPE >( A.R, B.R ) ;
	I.B = MTCNN::MIN< TYPE >( A.B, B.B ) ;

	return ( MTCNN::MAX< TYPE >( 0, ( I.R - I.L + 1 ) ) * MTCNN::MAX< TYPE >( 0, ( I.B - I.T + 1 ) ) ) ;

} // Intersection()


// SuppressNonMax()
//
static void SuppressNonMax( Objects_t & Objects, const float fOverlapMax, MTCNN::IOU_MODEL Model = MTCNN::IOU_MODEL::IoU )
//void MTCNN::SuppressNonMax( std::vector< Object_t > & Objects, const float fOverlapMax, MTCNN::IOU_MODEL Model )
{
	typedef std::multimap< float, size_t > ScoreObjects_t ;
	typedef ScoreObjects_t::value_type ScoreObject_t ;

	ScoreObjects_t ScoreObjects ;

	std::vector< size_t > MaxObjects ;

	size_t uScoreObjects ;

	size_t uObjects ;

	size_t uMaxObjects = 0 ;

	int32_t iIntersection ;

	float IOU = 0 ;

	uObjects = Objects.size() ;

	if( 0 == uObjects )
	{
		return ;
	}

	MaxObjects.resize( uObjects ) ;

	// sort Objects by increasing score
	//
	std::sort( Objects.begin(), Objects.end(), IncreasingScore ) ;

	// build 1:N lookup table from (common) score to object index(es)
	//
	for( size_t uObject = 0 ; uObject < uObjects ; ++( uObject ) )
	{
		//fprintf( stderr, "ScoreObject : %f --> %lu\n", double( Objects[ uObject ].score ), uObject ) ;
		ScoreObjects.insert( ScoreObject_t( Objects[ uObject ].score, uObject ) ) ;
	}

	while( 0 != ( uScoreObjects = ScoreObjects.size() ) )
	{
		// index of object with (shared) highest score
		//
		size_t uTail = ScoreObjects.rbegin()->second ;

		Object_t & TailObject = Objects[ uTail ] ;

		//fprintf( stderr, "Tail : %lu --> %f @ %4li, %4li - %4li, %4li\n", uTail, double( TailObject.score ), TailObject.Rect.L, TailObject.Rect.T, TailObject.Rect.R, TailObject.Rect.B ) ;

		MaxObjects[ ( uMaxObjects )++ ] = uTail ;

		auto ScoreObject = ScoreObjects.begin() ;

		while( ScoreObject != ScoreObjects.end() )
		{
			size_t uHead = ScoreObject->second ;

			Object_t & HeadObject = Objects[ uHead ] ;

			//fprintf( stderr, "Head : %f @ %4li, %4li - %4li, %4li\n", double( HeadObject.score ), HeadObject.Rect.L, HeadObject.Rect.T, HeadObject.Rect.R, HeadObject.Rect.B ) ;

			iIntersection = Intersection< int32_t >( HeadObject.Rect, TailObject.Rect ) ;

			if( 0 == iIntersection )
			{
				++( ScoreObject ) ;
				//ScoreObject = ScoreObjects.erase( ScoreObject ) ;
				continue ;
			}

			//fprintf( stderr, "I = %4li @ %3lu\n", lIntersection, uHead ) ;

			switch( Model )
			{
				case MTCNN::IOU_MODEL::Min :
					IOU = ( float( iIntersection ) / MTCNN::MIN< int32_t >( HeadObject.area, TailObject.area ) ) ;
					break ;

				default :
					IOU = ( float( iIntersection ) / float( HeadObject.area + TailObject.area - iIntersection ) ) ;
					break ;

			} // switch ... Model

			if( IOU > fOverlapMax )
			{
				//fprintf( stderr, "IOU = %11.9f @ %3lu\n", double( IOU ), uHead ) ;

				ScoreObject = ScoreObjects.erase( ScoreObject ) ;
			}
			else
			{
				++( ScoreObject ) ;
			}

		} // while ... ScoreObject
	}

	MaxObjects.resize( uMaxObjects ) ;

	std::vector< Object_t > tmp_ ;
	//
	tmp_.resize( uMaxObjects ) ;

	for( size_t uMaxObject = 0 ; uMaxObject < uMaxObjects ; ++( uMaxObject ) )
	{
		tmp_[ uMaxObject ] = Objects[ MaxObjects[ uMaxObject ] ] ;
	}

	Objects = tmp_ ;

} // SuppressNonMax()


// Refine()
//
static bool Refine( Object_t & Object, MTCNN::RECT< int32_t > & ROI )
{
	bool bResult = false ;

	MTCNN::RECT< float > fRect ;
	MTCNN::SIZE< float > fSize ;

	float fMaxSide ;

	fSize.W = float( Object.Rect.R - Object.Rect.L + 1 ) ;
	fSize.H = float( Object.Rect.B - Object.Rect.T + 1 ) ;

	//fprintf( stderr, "fSize = %6.2f, %6.2f" "\n", double( fSize.cx ), double( fSize.cy ) ) ;

	fRect.L = ( Object.Rect.L + ( Object.Bias.L * fSize.W ) ) ;
	fRect.T = ( Object.Rect.T + ( Object.Bias.T * fSize.H ) ) ;
	fRect.R = ( Object.Rect.R + ( Object.Bias.R * fSize.W ) ) ;
	fRect.B = ( Object.Rect.B + ( Object.Bias.B * fSize.H ) ) ;

	//fprintf( stderr, "fRect = %6.2f, %6.2f - %6.2f, %6.2f" "\n", double( fRect.L ), double( fRect.T ), double( fRect.R ), double( fRect.B ) ) ;

	fSize.W = ( fRect.R - fRect.L + 1 ) ;
	fSize.H = ( fRect.B - fRect.T + 1 ) ;

	//fprintf( stderr, "fSize = %6.2f, %6.2f" "\n", double( fSize.cx ), double( fSize.cy ) ) ;

	fMaxSide = MTCNN::MAX< float >( fSize.H, fSize.W ) ;

	fRect.L += ( ( fSize.W - fMaxSide ) * 0.5f ) ;
	fRect.T += ( ( fSize.H - fMaxSide ) * 0.5f ) ;

	//fprintf( stderr, "fRect = %6.2f, %6.2f - %6.2f, %6.2f" "\n", double( fRect.L ), double( fRect.T ), double( fRect.R ), double( fRect.B ) ) ;

	Object.Rect.L = int32_t( roundf( fRect.L ) ) ;
	Object.Rect.T = int32_t( roundf( fRect.T ) ) ;

	Object.Rect.R = int32_t( roundf( fRect.L + fMaxSide - 1 ) ) ;
	Object.Rect.B = int32_t( roundf( fRect.T + fMaxSide - 1 ) ) ;

	//fprintf( stderr, "Rect = %4i, %4i - %4i, %4i\n", Object.Rect.L, Object.Rect.T, Object.Rect.R, Object.Rect.B ) ;

	if( Object.Rect.L < ROI.L )
	{
		Object.Rect.L = ROI.L ;
	}
	if( Object.Rect.T < ROI.T )
	{
		Object.Rect.T = ROI.T ;
	}
	if( Object.Rect.R > ROI.R )
	{
		Object.Rect.R = ROI.R ;
	}
	if( Object.Rect.B > ROI.B )
	{
		Object.Rect.B = ROI.B ;
	}

	Object.area = ( ( Object.Rect.R - Object.Rect.L ) * ( Object.Rect.B - Object.Rect.T ) ) ;

	return bResult ;

} // Refine()


// ProposalProducer()
//
#ifdef MTCNN_OBJECT_CALLBACKS
static int ProposalProducer( ObjectCallback_t Callback, void * pOpaque, ncnn::Net & Engine, const ncnn::Mat & Bitmap, float fScale )

#else
static int ProposalProducer( Objects_t & Objects, ncnn::Net & Engine, ncnn::Mat & Bitmap, float fScale )

#endif
{
	int iResult = 0 ;

	const float fQualityMin = 0.8f ;

	const int32_t stride = 2 ;
	const int32_t cellsize = 12 ;

	ncnn::Extractor Inference = Engine.create_extractor() ;
	//
	Inference.set_light_mode( true ) ;
	Inference.input( "data", Bitmap ) ;

	ncnn::Mat Scalars, Vectors ;
	//
	Inference.extract( "prob1", Scalars ) ;
	Inference.extract( "conv4-2", Vectors ) ;

	float * pScores = Scalars.channel( 1 ) ;

	for( int32_t row = 0 ; row < Scalars.h ; ++( row ) )
	{
		for( int32_t col = 0 ; col < Scalars.w ; ++( col ) )
		{
			float fScore = *( pScores ) ;

			++( pScores ) ;

			if( fQualityMin > fScore )
			{
				continue ;
			}

#ifdef MTCNN_OBJECT_CALLBACKS
			Object_t Object ;

#else
			Object_t & Object = Objects.emplace_back() ;

#endif

			Object.score = fScore ;

			// TODO : why "+1" ?
			//
			Object.Rect.L = int32_t( roundf( ( stride * col + 1 ) * fScale ) ) ;
			Object.Rect.T = int32_t( roundf( ( stride * row + 1 ) * fScale ) ) ;
			Object.Rect.R = int32_t( roundf( ( stride * col + 1 + cellsize ) * fScale ) ) ;
			Object.Rect.B = int32_t( roundf( ( stride * row + 1 + cellsize ) * fScale ) ) ;

			Object.area = ( ( Object.Rect.R - Object.Rect.L ) * ( Object.Rect.B - Object.Rect.T ) ) ;

			const int32_t iPixel = ( row * Scalars.w + col ) ;

			Object.Bias.L = Vectors.channel( 0 )[ iPixel ] ;
			Object.Bias.T = Vectors.channel( 1 )[ iPixel ] ;
			Object.Bias.R = Vectors.channel( 2 )[ iPixel ] ;
			Object.Bias.B = Vectors.channel( 3 )[ iPixel ] ;

#ifdef MTCNN_OBJECT_CALLBACKS
			Callback( &( Object ), pOpaque ) ;

#else

#endif

			++( iResult ) ;

		} // for ... col

	} // for ... row

	return iResult ;

} // ProposalProducer()


// ScrutinyProducer()
//
#ifdef MTCNN_OBJECT_CALLBACKS
static void ScrutinyProducer( ObjectCallback_t Callback, void * pOpaque, ncnn::Net & Engine, ncnn::Mat & Bitmap, Object_t & Object )

#else
static void ScrutinyProducer( Objects_t & Objects, ncnn::Net & Engine, ncnn::Mat & Bitmap, Object_t & Object )

#endif
{
	const float fQualityMin = 0.8f ;

	do
	{
		ncnn::Extractor Forward = Engine.create_extractor() ;
		//
		Forward.set_light_mode( true ) ;
		Forward.input( "data", Bitmap ) ;

		ncnn::Mat Scalars, Vectors ;
		//
		Forward.extract( "prob1", Scalars ) ;
		Forward.extract( "conv5-2", Vectors ) ;

		Object.score = float( Scalars[ 1 ] ) ;

		if( fQualityMin > Object.score )
		{
			break ;
		}

		Object.Bias.L = float( Vectors[ 0 ] ) ;
		Object.Bias.T = float( Vectors[ 1 ] ) ;
		Object.Bias.R = float( Vectors[ 2 ] ) ;
		Object.Bias.B = float( Vectors[ 3 ] ) ;

		Object.area = ( ( Object.Rect.R - Object.Rect.L ) * ( Object.Rect.B - Object.Rect.T ) ) ;

#ifdef MTCNN_OBJECT_CALLBACKS
		if( nullptr != Callback )
		{
			Callback( &( Object ), pOpaque ) ;
		}

#else
		Objects.emplace_back( Object ) ;

#endif

	}
	while( 0 ) ;

} // ScrutinyProducer()


// LandmarkProducer()
//
#ifdef MTCNN_OBJECT_CALLBACKS
static void LandmarkProducer( ObjectCallback_t Callback, void * pOpaque, ncnn::Net & Engine, ncnn::Mat & Bitmap, Object_t & Object )

#else
static void LandmarkProducer( Objects_t & Objects, ncnn::Net & Engine, ncnn::Mat & Bitmap, Object_t & Object )

#endif
{
	const float fQualityMin = 0.6f ;

	do
	{
		ncnn::Extractor Forward = Engine.create_extractor() ;
		//
		Forward.set_light_mode( true ) ;
		Forward.input( "data", Bitmap ) ;

		ncnn::Mat Scores, Bias, Points ;
		//
		Forward.extract( "prob1", Scores ) ;
		Forward.extract( "conv6-2", Bias ) ;
		Forward.extract( "conv6-3", Points ) ;

		Object.score = Scores[ 1 ] ;

		if( fQualityMin > Object.score )
		{
			break ;
		}

		Object.Bias.L = float( Bias[ 0 ] ) ;
		Object.Bias.T = float( Bias[ 1 ] ) ;
		Object.Bias.R = float( Bias[ 2 ] ) ;
		Object.Bias.B = float( Bias[ 3 ] ) ;

		int32_t iPoint, iPoints = 5 ;

		for( iPoint = 0 ; iPoint < iPoints ; ++( iPoint ) )
		{
			Object.Points[ iPoint ].X = ( Object.Rect.L + ( Object.Rect.R - Object.Rect.L ) * Points[ 0 + iPoint ] ) ;
			Object.Points[ iPoint ].Y = ( Object.Rect.T + ( Object.Rect.B - Object.Rect.T ) * Points[ 5 + iPoint ] ) ;

		} // for ... iPoint

#ifdef MTCNN_OBJECT_CALLBACKS
		if( nullptr != Callback )
		{
			Callback( &( Object ), pOpaque ) ;
		}

#else
		Objects.emplace_back( Object ) ;

#endif

	}
	while( 0 ) ;

} // LandmarkProducer()


static void ObjectConsumer( Object_t * pObject, void * pOpaque )
{
	auto pObjects = static_cast< Objects_t * >( pOpaque ) ;
	//
	pObjects->push_back( *( pObject ) ) ;

} // ObjectConsumer()


class ImagePyramid_t
{
private:
	const ncnn::Mat & Mat ;
	std::unordered_map< float, ncnn::Mat > Map ;

public:
	~ImagePyramid_t()
	{
	}

	ImagePyramid_t( const ncnn::Mat & Image ) : Mat( Image )
	{
		Map[ 1.f ] = Mat ;
	}

	const ncnn::Mat & at( float fScale )
	{
		auto bImage = ( 0 != Map.count( fScale ) ) ;

		auto & Image = Map[ fScale ] ;

		do
		{
			if( bImage )
			{
				fprintf( stderr, "= scale = %8.6f" "\n", double( fScale ) ) ;
				break ;
			}

			auto iImageH = int( ceil( Mat.h * fScale ) ) ;
			auto iImageW = int( ceil( Mat.w * fScale ) ) ;

			ncnn::resize_bilinear( Mat, Image, iImageW, iImageH ) ;

			fprintf( stderr, "  scale = %8.6f" "\n", double( fScale ) ) ;
		}
		while( 0 ) ;

		return Image ;

	} // at()

} ; // class ImagePyramid_t


// Detect()
//
static int32_t Detect( ObjectCallback_t Callback, void * pOpaque, ncnn::Mat & Bitmap, Detect_t * pDetect )
{
	int iResult = 0 ;

	do
	{
		const float fOverlapMax[ 3 ] = { 0.5f, 0.7f, 0.7f } ;

		const float fMean[ 3 ] = { 127.5, 127.5, 127.5 } ;
		const float fNorm[ 3 ] = { 0.0078125, 0.0078125, 0.0078125 } ; // ( 1 / 128 )

		MTCNN::RECT< int32_t > ROI = { 0, 0, ( Bitmap.w - 1 ), ( Bitmap.h - 1 ) } ;

		Bitmap.substract_mean_normalize( fMean, fNorm ) ;

		auto fSide = float( MTCNN::MIN< int32_t >( Bitmap.h, Bitmap.w ) ) ;

		if( !( 0.f < pDetect->fScaleStep ) || !( 1 > pDetect->fScaleStep ) )
		{
			pDetect->fScaleStep = 0.709f ;
		}

		if( 0 > pDetect->fSizeMax )
		{
			pDetect->fSizeMax = roundf( 0.48f * fSide ) ;
		}
		else if( 0.f == pDetect->fSizeMax )
		{
			pDetect->fSizeMax = fSide ;
		}
		else if( 1 > pDetect->fSizeMax )
		{
			pDetect->fSizeMax = roundf( pDetect->fSizeMax * fSide ) ;
		}

		if( 0 > pDetect->fSizeMin )
		{
			pDetect->fSizeMin = 48.0f ;
		}
		else if( 0.f == pDetect->fSizeMin )
		{
			pDetect->fSizeMin = MTCNN::fModelSize ;
		}
		else if( 1 > pDetect->fSizeMin )
		{
			pDetect->fSizeMin = roundf( pDetect->fSizeMin * fSide ) ;
		}

		//fprintf( stderr, "SizeMax = %.0f" "\n", double( _.fSizeMax ) ) ;
		//fprintf( stderr, "SizeMin = %.0f" "\n", double( _.fSizeMin ) ) ;

		auto fScaleMax = float( MTCNN::fModelSize / pDetect->fSizeMin ) ;
		auto fScaleMin = float( MTCNN::fModelSize / pDetect->fSizeMax ) ;

		ImagePyramid_t Images( Bitmap ) ;

		size_t uObject, uObjects ;

		Objects_t Objects ;

		//
		// PROPOSAL PHASE
		//
#ifdef _WIN32
		//concurrency::concurrent_vector< Object_t > ProposalObjects ;
#else
#endif

		Objects_t ProposalObjects ;

		// image pyramid
		//
		// ? parallelize with concurrent vector
		//
		for( auto fScale = fScaleMax ; fScale > fScaleMin ; fScale *= pDetect->fScaleStep )
		{
			/*
			ncnn::Mat BitmapScaled ;

			auto iBitmapH = int32_t( ceil( Bitmap.h * fScale ) ) ;
			auto iBitmapW = int32_t( ceil( Bitmap.w * fScale ) ) ;

			//fprintf( stderr, "\n" "scale = %f" "\n", double( fScale ) ) ;
			//fprintf( stderr, "resize = %i x %i" "\n", iBitmapW, iBitmapH ) ;

			ncnn::resize_bilinear( Bitmap, BitmapScaled, iBitmapW, iBitmapH ) ;
			*/

			Objects.clear() ;

#ifdef MTCNN_OBJECT_CALLBACKS
			ProposalProducer( ObjectConsumer, &( Objects ), pDetect->mtcnn_p, Images.at( fScale ), ( 1.f / fScale ) ) ;
			//ProposalProducer( ObjectConsumer, &( Objects ), pDetect->mtcnn_p, BitmapScaled, ( 1.f / fScale ) ) ;

#else
			ProposalProducer( Objects, pDetect->mtcnn_p, BitmapScaled, ( 1.f / fScale ) ) ;

#endif

			uObjects = Objects.size() ;

			//printf( "\n" "ProposalObjects = %lu after ProposalProducer()" "\n", uObjects ) ;

			if( 0 == uObjects )
			{
				continue ;
			}

			// intra-scale regression
			//
			SuppressNonMax( Objects, fOverlapMax[ 0 ] ) ;

			uObjects = Objects.size() ;

			//printf( "ProposalObjects = %lu after SuppressNonMax()" "\n", uObjects ) ;

			if( 0 == uObjects )
			{
				continue ;
			}

			ProposalObjects.reserve( uObjects + ProposalObjects.size() ) ;

			for( uObject = 0 ; uObject < uObjects ; ++( uObject ) )
			{
				ProposalObjects.push_back( Objects[ uObject ] ) ;
			}

		} // for ... fScale

		uObjects = ProposalObjects.size() ;

		//printf( "\n" "ProposalObjects = %lu after PNet()" "\n", uObjects ) ;

		if( 0 == uObjects )
		{
			break ;
		}

		SuppressNonMax( ProposalObjects, fOverlapMax[ 0 ] ) ;

		uObjects = ProposalObjects.size() ;

		//printf( "\n" "ProposalObjects = %lu after SuppressNonMax()" "\n", uObjects ) ;

		if( 0 == uObjects )
		{
			break ;
		}

		uObject = uObjects ;

		while( 0 < uObject )
		{
			--( uObject ) ;

			// allow removing objects not fully contained in ROI
			//
			if( Refine( ProposalObjects[ uObject ], ROI ) )
			{
				ProposalObjects.pop_back() ;
			}

		} // while ... uObject

		uObjects = ProposalObjects.size() ;

		//printf( "\n" "ProposalObjects = %lu after refine()" "\n", uObjects ) ;

		if( 0 == uObjects )
		{
			break ;
		}

		/*
		for( uObject = 0 ; uObject < uObjects ; ++( uObject ) )
		{
			auto & Object = ProposalObjects[ uObject ] ;

			fprintf( stderr, "[ %4lu ] score = %11.9f @ %4i, %4i - %4i, %4i" "\n", uObject, double( Object.score ), Object.Rect.L, Object.Rect.T, Object.Rect.R, Object.Rect.B ) ;

		} // for ... uObject
		 */
		// loose
		// strict
		//
		// SCRUTINY PHASE
		//
		Objects_t ScrutinyObjects ;

		for( uObject = 0 ; uObject < uObjects ; ++( uObject ) )
		{
			auto & Object = ProposalObjects[ uObject ] ;

			float fScale = ( 24.f / ( 1 + Object.Rect.B - Object.Rect.T ) ) ;

			auto & Image = Images.at( fScale ) ;

			MTCNN::POINT< int > Point = { int( roundf( ( fScale * ( Object.Rect.L + Object.Rect.R ) ) / 2 ) ), int( roundf( ( fScale * ( Object.Rect.T + Object.Rect.B ) ) / 2 ) ) } ;

			MTCNN::RECT< int > Rect = { Point.X - 12, Point.Y - 12, Point.X + 11, Point.Y + 11 } ;

			ncnn::Mat Size24 ;
			//
			ncnn::copy_cut_border( Image, Size24, Rect.T, Image.h - Rect.B, Rect.L, Image.w - Rect.R ) ;

			/*
			ncnn::Mat Square ;

			ncnn::copy_cut_border( Bitmap, Square, int32_t( Object.Rect.T ), int32_t( Bitmap.h - Object.Rect.B ), int32_t( Object.Rect.L ), int32_t( Bitmap.w - Object.Rect.R ) ) ;

			// TODO : try resize_bilinear_c3()
			//
			ncnn::Mat Size24 ;
			//
			ncnn::resize_bilinear( Square, Size24, 24, 24 ) ;
			*/

#ifdef MTCNN_OBJECT_CALLBACKS
			ScrutinyProducer( ObjectConsumer, &( ScrutinyObjects ), pDetect->mtcnn_r, Size24, Object ) ;
#else
			ScrutinyProducer( ScrutinyObjects, pDetect->mtcnn_r, Size24, Object ) ;
#endif
		} // for ... uObject

		uObjects = ScrutinyObjects.size() ;

		//printf( "\n" "ScrutinyObjects = %lu after RNet()" "\n", uObjects ) ;

		if( 0 == uObjects )
		{
			break ;
		}

		SuppressNonMax( ScrutinyObjects, fOverlapMax[ 1 ] ) ;

		uObjects = ScrutinyObjects.size() ;

		//printf( "\n" "ScrutinyObjects = %lu after SuppressNonMax()" "\n", uObjects ) ;

		if( 0 == uObjects )
		{
			break ;
		}

		uObject = uObjects ;

		while( 0 < uObject )
		{
			--( uObject ) ;

			// allow removing objects not fully contained in ROI
			//
			if( Refine( ScrutinyObjects[ uObject ], ROI ) )
			{
				ScrutinyObjects.pop_back() ;
			}

		} // while ... uObject

		//refine( ScrutinyObjects, ROI ) ;

		uObjects = ScrutinyObjects.size() ;

		//printf( "\n" "ScrutinyObjects = %lu after refine()" "\n", uObjects ) ;

		if( 0 == uObjects )
		{
			break ;
		}

		/*
		for( uObject = 0 ; uObject < uObjects ; ++( uObject ) )
		{
			auto & Object = ScrutinyObjects[ uObject ] ;

			fprintf( stderr, "[ %4lu ] score = %11.9f @ %4i, %4i - %4i, %4i" "\n", uObject, double( Object.score ), Object.Rect.L, Object.Rect.T, Object.Rect.R, Object.Rect.B ) ;

		} // for ... uObject
		 */

		//
		// LANDMARK PHASE
		//
		Objects_t LandmarkObjects ;

		for( uObject = 0 ; uObject < uObjects ; ++( uObject ) )
		{
			auto & Object = ScrutinyObjects[ uObject ] ;

			ncnn::Mat Square ;

			// Rect probably changed...
			//
			ncnn::copy_cut_border( Bitmap, Square, int32_t( Object.Rect.T ), int32_t( Bitmap.h - Object.Rect.B ), int32_t( Object.Rect.L ), int32_t( Bitmap.w - Object.Rect.R ) ) ;

			// TODO : try resize_bilinear_c3()
			//
			ncnn::Mat Size48 ;
			//
			ncnn::resize_bilinear( Square, Size48, 48, 48 ) ;

#ifdef MTCNN_OBJECT_CALLBACKS
			LandmarkProducer( ObjectConsumer, &( LandmarkObjects ), pDetect->mtcnn_o, Size48, Object ) ;

#else
			LandmarkProducer( LandmarkObjects, pDetect->mtcnn_o, Size48, Object ) ;

#endif

		} // for ... uObject

		uObjects = LandmarkObjects.size() ;
		//
		//printf( "\n" "LandmarkObjects = %lu after ONet()" "\n", uObjects ) ;

		if( 0 == uObjects )
		{
			break ;
		}

		uObject = uObjects ;

		while( 0 < uObject )
		{
			--( uObject ) ;

			// allow removing objects not fully contained in ROI
			//
			if( Refine( LandmarkObjects[ uObject ], ROI ) )
			{
				LandmarkObjects.pop_back() ;
			}

		} // while ... uObject

		//refine( LandmarkObjects, ROI ) ;

		uObjects = LandmarkObjects.size() ;
		//
		//printf( "\n" "LandmarkObjects = %lu after refine()" "\n", uObjects ) ;

		if( 0 == uObjects )
		{
			break ;
		}

		SuppressNonMax( LandmarkObjects, fOverlapMax[ 2 ], MTCNN::IOU_MODEL::Min ) ;

		uObjects = LandmarkObjects.size() ;
		//
		//printf( "\n" "LandmarkObjects = %lu after SuppressNonMax()" "\n", uObjects ) ;

		if( 0 == uObjects )
		{
			break ;
		}

		/*
		for( uObject = 0 ; uObject < uObjects ; ++( uObject ) )
		{
			auto & Object = LandmarkObjects[ uObject ] ;

			fprintf( stderr, "[ %4lu ] score = %11.9f @ %4i, %4i - %4i, %4i" "\n", uObject, double( Object.score ), Object.Rect.L, Object.Rect.T, Object.Rect.R, Object.Rect.B ) ;

		} // for ... uObject
		 */

		iResult = int32_t( uObjects ) ;

		if( nullptr != Callback )
		{
			for( auto & LandmarkObject : LandmarkObjects )
			{
				Callback( &( LandmarkObject ), pOpaque ) ;
			}
		}

	}
	while( 0 ) ;

	return iResult ;

} // Detect()


int32_t detect_exec( Detect_t * pDetect, detect_options_t * pOptions )
{
	int32_t iResult = 0 ;

	do
	{
		if( nullptr == pDetect )
		{
			break ;
		}

		pDetect->fScaleStep = pOptions->fScaleStep ;

		pDetect->fSizeMax = pOptions->fSizeMax ;
		pDetect->fSizeMin = pOptions->fSizeMin ;

		ncnn::Mat Bitmap = ncnn::Mat::from_pixels( pOptions->pPixels, ncnn::Mat::PIXEL_RGB, pOptions->iPixelW, pOptions->iPixelH ) ;

		iResult = Detect( pOptions->pMethod, pOptions->pOpaque, Bitmap, pDetect ) ;

	}
	while( 0 ) ;

	return iResult ;

} // detect_exec()


Detect_t * detect_exit( Detect_t * pDetect )
{
	do
	{
		if( nullptr == pDetect )
		{
			break ;
		}

		pDetect->mtcnn_p.clear() ;
		pDetect->mtcnn_r.clear() ;
		pDetect->mtcnn_o.clear() ;

		delete pDetect ;

		pDetect = nullptr ;

	}
	while( 0 ) ;

	return pDetect ;

} // detect_exit()


Detect_t * detect_init()
{
	auto pDetect = new Detect_t() ;

	do
	{
		int iResult = 0 ; // net.opt.use_vulkan_compute = 1;

		iResult += pDetect->mtcnn_p.load_param_mem( mtcnn_p_param ) ;
		iResult += pDetect->mtcnn_p.load_model( mtcnn_p_model ) ;

		iResult += pDetect->mtcnn_r.load_param_mem( mtcnn_r_param ) ;
		iResult += pDetect->mtcnn_r.load_model( mtcnn_r_model ) ;

		iResult += pDetect->mtcnn_o.load_param_mem( mtcnn_o_param ) ;
		iResult += pDetect->mtcnn_o.load_model( mtcnn_o_model ) ;

	}
	while( 0 ) ;

	return pDetect ;

} // detect_init()

#ifndef	RING_BUFFER_H
#	define	RING_BUFFER_H

/** Ring (AKA circular) buffer generic framework.
 *
 * \note	Buffer size \b must be a power of two.
 *
 * Usage
 * -----
 *
 * a) For publicly accessible ring-buffers:
 *
 * Step 1:
 * 	Use `ringbuffer_declare_all( NAME, TYPE, LEN )` in your header file (.h) to
 * 	declare the control structure and all functions at once.
 *
 * Step 2:
 * 	Use `ringbuffer_define_all( NAME )` in your source file (.c) to
 * 	\b define all your functions at once.
 *
 * b) For locally accessible ring-buffers (used only inside a single .c):
 * Step 1:
 * 	Use `ringbuffer_type_def( NAME, TYPE, LEN )` to define your control structure.
 *
 * Step 2:
 * 	Use `ringbuffer_define_all( NAME )` in your source file (.c) to
 * 	\b define all your functions at once exactly as done in a).
 *
 * This macros will create a typedef'd control structure like in the example below:
 *
 * \code
	ringbuffer_type_def( peanuts, int, 8 );

	// This generates:
	struct ring_buffer_peanuts;

	typedef bool	( * peanuts_push_callback_t ) ( struct ring_buffer_peanuts *rb, int data );

	typedef struct ring_buffer_peanuts
	{
		index_t input;
		index_t output;
		int data_buffer[ 8 ];
		peanuts_push_callback_t push_callback;
	} peanuts;
 * \endcode
 *
 * The set of generated functions will be:
 *
 * \code
	void	peanuts_init ( peanuts *rb, peanuts_push_callback_t push_callback );
	bool	peanuts_push_front ( peanuts *rb, int data );
	bool	peanuts_pop_back ( peanuts *rb );
	int *	peanuts_peek ( peanuts *rb, index_t offset );
	size_t	peanuts_count ( peanuts *rb );
	bool	peanuts_empty ( peanuts *rb );
	bool	peanuts_full ( peanuts *rb );
 * \endcode
 *
 * \addtogroup PushCallback	Push Callback Function
 *
 * \todo
 *
 * \code
bool	peanuts_push_front_callback ( peanuts *rb, TYPE data )
{
	rb->data_buffer[ RINGBUF_WRAP ( rb, rb->input++ ) ] = data;

	return true;
}
 * \endcode
 *
 */

// TODO: Consider `static` ring buffer functions.


#ifdef __KERNEL__
#	include <linux/types.h>
#else
#	include <stdint.h>
#	include <stddef.h>
#	include <stdbool.h>
#endif

#include "utility.h"
#include "cprep_tricks.h"


typedef	uint32_t	index_t;


/** Ring buffer helpers. @{ */

// --------------------------------------
#define	DECL_qualif_( t, ... )	t
#define	DECL_qualif( ... )	DECL_qualif_( __VA_ARGS__, )

// --------------------------------------

/** Limits index inside buffer bounds wrapping if needed.
 *
 * \note Buffer lenght \b must be power of two!
 */
#define RINGBUF_WRAP( rb, index )	( ( index ) & ( ARRAY_COUNT( ( rb )->data_buffer ) - 1 ) )

/// Shortcut to current input position.
#define RINGBUF_CURR_i( rb )		( rb )->data_buffer[ RINGBUF_WRAP ( ( rb ), ( rb )->input ) ]

/// Don't use. Use RINGBUF_CURR_o instead.
#define RINGBUF_CURR_o_( rb, offset )	TEST( offset )( \
	( rb )->data_buffer[ RINGBUF_WRAP ( ( rb ), ( rb )->output + offset ) ], \
	( rb )->data_buffer[ RINGBUF_WRAP ( ( rb ), ( rb )->output ) ] )

/// Shortcut to current output position.
#define RINGBUF_CURR_o( rb, ... )	RINGBUF_CURR_o_( ( rb ), EFIRST( __VA_ARGS__ ) )

/// Get buffer intrinsic element type.
#define DATA_TYPE( NAME )	typeof( ( ( NAME * )0 )->data_buffer[ 0 ] )

/// Get buffer element capacity.
#define	BUFFER_LEN( NAME )	ARRAY_COUNT( ( ( NAME * )0 )->data_buffer )


/** @} end Ring buffer helpers. */


// --------------------------------------
/** Define ring buffer control structure.
 *
 * \var input		Input index in data_buffer.
 * \var output		Output index in data_buffer.
 * \var data_buffer	Buffer containing circular buffer data.
 * 			This buffer must be allocated outside
 * 			with the actual data type.
 * \var push_callback	Custom action to perform on element insertion.
 */
#define ringbuffer_type_def( NAME, TYPE, LEN, ... )	\
	typedef struct ring_buffer_ ## NAME NAME;	\
	typedef bool ( * NAME ## _push_callback_t )( DECL_qualif( __VA_ARGS__ ) NAME *rb, TYPE *data );	\
	\
	struct ring_buffer_ ## NAME	\
	{					\
		index_t	input;			\
		index_t	output;			\
		TYPE	data_buffer[ LEN ];	\
		NAME ## _push_callback_t	push_callback;	\
	}


/** Ring buffer function declaration macros. @{ */

#define	ringbuffer_init_decl( NAME, ... )	\
	void	NAME ## _init( DECL_qualif( __VA_ARGS__ ) NAME *rb, NAME ## _push_callback_t push_callback )

#define	ringbuffer_push_front_decl( NAME, ... )	\
	bool	NAME ## _push_front ( DECL_qualif( __VA_ARGS__ ) NAME *rb, DATA_TYPE( NAME ) *data )

#define	ringbuffer_pop_back_decl( NAME, ... )	\
	bool	NAME ## _pop_back ( DECL_qualif( __VA_ARGS__ ) NAME *rb )

#define	ringbuffer_peek_decl( NAME, ... )	\
	DECL_qualif( __VA_ARGS__ ) DATA_TYPE( NAME )	*NAME ## _peek ( DECL_qualif( __VA_ARGS__ ) NAME *rb, index_t offset )

#define	ringbuffer_count_decl( NAME, ... )	\
	size_t	NAME ## _count ( DECL_qualif( __VA_ARGS__ ) NAME *rb )

#define	ringbuffer_empty_decl( NAME, ... )	\
	bool	NAME ## _empty ( DECL_qualif( __VA_ARGS__ ) NAME *rb )

#define	ringbuffer_full_decl( NAME, ... )	\
	bool	NAME ## _full ( DECL_qualif( __VA_ARGS__ ) NAME *rb )

// --------------------------------------
#define ringbuffer_declare_all( NAME, TYPE, LEN, ... )	\
	ringbuffer_type_def( NAME, TYPE, LEN, __VA_ARGS__ );	\
	\
	ringbuffer_init_decl( NAME, __VA_ARGS__ );	\
	\
	ringbuffer_count_decl( NAME, __VA_ARGS__ );	\
	\
	ringbuffer_empty_decl( NAME, __VA_ARGS__ );	\
	\
	ringbuffer_full_decl( NAME, __VA_ARGS__ );	\
	\
	ringbuffer_push_front_decl( NAME, __VA_ARGS__ );	\
	\
	ringbuffer_pop_back_decl( NAME, __VA_ARGS__ );	\
	\
	ringbuffer_peek_decl( NAME, __VA_ARGS__ )

// --------------------------------------

/** @} end Ring buffer function declaration macros. */

// --------------------------------------


/** Ring buffer function definition macros. @{ */

#define	ringbuffer_init_def( NAME, ... )	\
	void	NAME ## _init( DECL_qualif( __VA_ARGS__ ) NAME *rb, NAME ## _push_callback_t push_callback )	{\
		rb->input = rb->output	= 0;	\
		rb->push_callback	= push_callback; }

#define	ringbuffer_push_front_def( NAME, ... )	\
	bool	NAME ## _push_front ( DECL_qualif( __VA_ARGS__ ) NAME *rb, DATA_TYPE( NAME ) *data )	{\
		if ( NAME ## _full( rb ) ) return false;	\
		if ( rb->push_callback )	\
		{ if( !rb->push_callback( rb, data ) ) return false; }	\
		else { RINGBUF_CURR_i( rb ) = *data; }	\
		rb->input++; return true; }

#define	ringbuffer_pop_back_def( NAME, ... )	\
	bool	NAME ## _pop_back ( DECL_qualif( __VA_ARGS__ ) NAME *rb )	{\
		if ( NAME ## _empty( rb ) ) return false;	\
		rb->output++;	/* Just increment since read will be masked. */	\
		return true; }

#define	ringbuffer_peek_def( NAME, ... )	\
	DECL_qualif( __VA_ARGS__ ) DATA_TYPE( NAME )	*NAME ## _peek ( DECL_qualif( __VA_ARGS__ ) NAME *rb, index_t offset )	{\
		if ( NAME ## _count( rb ) > offset )	\
			return &( RINGBUF_CURR_o( rb, offset ) );	\
		return  NULL; }
// 		if ( NAME ## _count( rb ) && NAME ## _count( rb ) > offset )
// 	return  &( rb->data_buffer[ RINGBUF_WRAP( rb, rb->output + offset ) ] ); }

#define	ringbuffer_count_def( NAME, ... )	\
	size_t	NAME ## _count ( DECL_qualif( __VA_ARGS__ ) NAME *rb )	\
	{ return ( rb->input - rb->output ); }

#define	ringbuffer_empty_def( NAME, ... )	\
	bool	NAME ## _empty ( DECL_qualif( __VA_ARGS__ ) NAME *rb )	\
	{ return 0 == NAME ## _count( rb ); }

#define	ringbuffer_full_def( NAME, ... )	\
	bool	NAME ## _full ( DECL_qualif( __VA_ARGS__ ) NAME *rb )	\
	{ return NAME ## _count( rb ) == BUFFER_LEN( NAME ); }

// --------------------------------------
#define ringbuffer_define_all( NAME, ... )	\
	ringbuffer_init_def( NAME, __VA_ARGS__ )	\
	\
	ringbuffer_count_def( NAME, __VA_ARGS__ )	\
	\
	ringbuffer_empty_def( NAME, __VA_ARGS__ )	\
	\
	ringbuffer_full_def( NAME, __VA_ARGS__ )	\
	\
	ringbuffer_push_front_def( NAME, __VA_ARGS__ )	\
	\
	ringbuffer_pop_back_def( NAME, __VA_ARGS__ )	\
	\
	ringbuffer_peek_def( NAME, __VA_ARGS__ )

// --------------------------------------

/** @} end Ring buffer function definition macros. */


#endif	// RING_BUFFER_H

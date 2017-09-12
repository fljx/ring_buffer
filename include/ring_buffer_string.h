#ifndef	RING_BUFFER_STRING_H
#	define	RING_BUFFER_STRING_H


#include "ring_buffer.h"
#include "cprep_tricks.h"


// --------------------
#define PASTE_cond(COND,BEHAV)	TEST(COND)( TEST( BEHAV )(if( COND ) { BEHAV; }, ), )
// --------------------

// --------------------------------------
/** String ring buffer function declaration macros.. @{ */


/** Push all items from a string up to @len elements. */
#define	ringbuffer_push_string_decl( NAME, ... )	\
	size_t	NAME ## _push_string ( DECL_qualif( __VA_ARGS__ ) NAME *rb, DATA_TYPE( NAME ) *data, size_t len )

/** Pop all strings up to @len elements. */
#define	ringbuffer_pop_string_decl( NAME, ... )	\
	size_t	NAME ## _pop_string ( DECL_qualif( __VA_ARGS__ ) NAME *rb, DATA_TYPE( NAME ) *dest, size_t limit )

/** Pop all strings up to @len items and null-terminate the output.
 *
 * \note Null items in the middle of the string will be copied as well.
 * 	To stop a copying at a null, use ringbuffer_pop_cstring_cond_decl instead.
 */
#define	ringbuffer_pop_cstring_decl( NAME, ... )	\
	size_t	NAME ## _pop_cstring ( DECL_qualif( __VA_ARGS__ ) NAME *rb, DATA_TYPE( NAME ) *dest, size_t limit )

/** \brief Pop an entire string from ring-buffer applying condition `COND`.
 *
 * \param	COND	`COND` is anything that evaluates to a logic expression (tested inside an `if`).
 * \param	BEHAV	Behaviour case `COND` is true (block of code do execude on `if`).
 *
 * \note	`COND` is evaluated \b before current item copy.
 *
 *	This can be useful for, e.g. copying c-strings skipping null items (e.g. '\\0') like below:
 * \code
 * 	ringbuffer_pop_cstring_cond_def( my_buffer,	// `char` buffer.
 *		'\\0' ==  RINGBUF_CURR_o( rb )		// COND
 * 		continue				// BEHAV
 * 	)
 *
 * \endcode
 *
 *	Another use would be stop copying on value (e.g. '0x002a') like below:
 * \code
 * 	ringbuffer_pop_cstring_cond_def( my_buffer,
 *		0x002a ==  RINGBUF_CURR_o( rb )		// COND
 * 		break					// BEHAV
 * 	)
 *
 * \endcode
 *
 * 	\note Output wll be null-terminated.
 */
#define	ringbuffer_pop_cstring_cond_decl( NAME, SUFFIX, SKIP_COND, ... )	\
	size_t	NAME ## _pop_string_ ## SUFFIX ( DECL_qualif( __VA_ARGS__ ) NAME *rb, DATA_TYPE( NAME ) *dest, size_t limit )


/** @} end String ring buffer function declaration macros.. */


// --------------------------------------
/** String ring buffer function definition macros.. @{ */

#define	ringbuffer_push_string_def( NAME, ... )	\
	size_t	NAME ## _push_string ( DECL_qualif( __VA_ARGS__ ) NAME *rb, DATA_TYPE( NAME ) *data, size_t len )	\
	{ size_t count = 0;	\
	for ( ; len &&			/* Check end of source... */	\
		NAME ## _full( rb );	/*	and and available space. */	\
		rb->input++, --len )	\
	{ RINGBUF_CURR_i( rb ) = src[ count++ ]; }	\
	return count; }
// 	{ rb->data_buffer[ RINGBUF_WRAP( rb, rb->input ) ] = src[ count++ ]; }

#define	ringbuffer_pop_string_def( NAME, ... )	\
	size_t	NAME ## _pop_string ( DECL_qualif( __VA_ARGS__ ) NAME *rb, DATA_TYPE( NAME ) *dest, size_t limit )	\
	{ size_t count = 0;	\
	if ( NAME ## _empty( rb ) ) return 0;	\
	for ( ; limit-- &&			/* Check end of dest buffer... */	\
		NAME ## _count( rb );		/*	and ring-buffer empty. */	\
		rb->output++ )	{	\
		dest[ count++ ] = RINGBUF_CURR_o( rb ); }	\
	return count; }
// 	for ( ; count < limit &&		/* Check end of dest buffer... */
// 		dest[ count++ ] = rb->data_buffer[ RINGBUF_WRAP( rb, rb->output ) ]; }

#define	ringbuffer_pop_cstring_def( NAME, ... )	\
	size_t	NAME ## _pop_cstring ( DECL_qualif( __VA_ARGS__ ) NAME *rb, DATA_TYPE( NAME ) *dest, size_t limit )	\
	{ size_t count = 0;	\
	if ( NAME ## _empty( rb ) ) return 0;	\
	for ( ; --limit &&			/* Check end of dest buffer (save space for terminator)... */	\
		NAME ## _count( rb );		/*	and ring-buffer empty. */	\
		rb->output++ )	{	\
		dest[ count++ ] = RINGBUF_CURR_o( rb ); }	\
	dest[ count ] = ( ( DATA_TYPE( NAME ) )0 );	\
	return count; }

#define	ringbuffer_pop_cstring_cond_def( NAME, SUFFIX, COND, BEHAV, ... )	\
	size_t	NAME ## _pop_string_ ## SUFFIX ( DECL_qualif( __VA_ARGS__ ) NAME *rb, DATA_TYPE( NAME ) *dest, size_t limit )	\
	{ size_t count = 0;	\
	if ( NAME ## _empty( rb ) ) return 0;	\
	for ( ; --limit &&			/* Check end of dest buffer (save space for terminator)... */	\
		NAME ## _count( rb );		/*	and ring-buffer empty. */	\
		rb->output++ )	{	\
		PASTE_cond( COND, BEHAV )	\
		dest[ count++ ] = RINGBUF_CURR_o( rb ); }	\
	dest[ count ] = ( ( DATA_TYPE( NAME ) )0 );	\
	return count; }
// 	for ( ; count < ( limit - 1 ) &&	/* Check end of dest buffer (save space for terminator)... */

/** @} end String ring buffer function definition macros.. */


#endif	// RING_BUFFER_STRING_H


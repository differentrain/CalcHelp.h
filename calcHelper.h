/****************************************************************************
*			DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE						*
*					Version 2, December 2004								*
*																			*
*	Copyright (C) 2018 differentrain <yy@differentrain.cn>					*
*																			*
*	Everyone is permitted to copy and distribute verbatim or modified		*
*	copies of this license document, and changing it is allowed as long		*
*	as the name is changed.													*
*																			*
*		DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE							*
*	TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION			*
*																			*
*	0. You just DO WHAT THE FUCK YOU WANT TO.								*
*																			*
****************************************************************************/

/*
	calcHelper is a portable library written in ANSI C, provides the expression computing functionality.
	This library support the computing of integers(can be collectively called "programmer mode") and decimals -- "math mode" .
	----------------------------------------------------------
	Programmer mode has these features :
		Operand-pre-checking	√
		Overflow-pre-checking	√  ( EXCEPT multiplication, "*" )
		Ignore-white-space      √
		Thread-safety			×
		Operators：
			!	:	NOT
		-------------------------------------
			*	:	MUL
			/	:	DIV
			%	:	MOD
		-------------------------------------
			+	:	ADD
			-	:	SUB
		-------------------------------------
			<<	:	Arithmetic shift left
			>>	:	Arithmetic shift right
			<<.	:	Rotate left
			.>>	:	Rotate right
			>>>	:	Logical shift right
		-------------------------------------
			&	:	AND
		-------------------------------------
			^	:	XOR
		-------------------------------------
			|	：	OR
	----------------------------------------------------------
	Math mode has these features :
		Operand-pre-checking	×	( EXCEPT division, "/" )
		Overflow-pre-checking	×
		Operand-checking		√	( EXCEPT division, "/" )
		Overflow-checking		×	( implemented by <errno.h>(C89), incomplete.)
		Ignore-white-space      √
		Thread-safety			×
		Operators：
			*	:	MUL
			/	:	DIV
			%	:	MOD		( the result has the same sign as left-operand. )
		-------------------------------------
			+	:	ADD
			-	:	SUB
		-------------------------------------
		Functions, NOT-ignore-case :
			acos(x)		:	Arc cosine
			asin(x)		:	Arc sine
			atan(x)		:	Arc tangent
			atan2(y,x)	:	Arc tangent, using signs to determine quadrants
			cos(x)		:	Cosine
			cosh(x)		:	Hyperbolic cosine
			sin(x)		:	Sine
			sinh(x)		:	Hyperbolic sine
			tan(x)		:	Tangent
			tanh(x)		:	Hyperbolic tangent
			exp(x)		:	e raised to x power
			lg(x)		:	log10(x)
			ln(x)		:	loge(x)
			sqrt(x)		:	Square root
			pow(x,y)	:	A x raised to the y power
	----------------------------------------------------------
*/

#ifndef CALCH_VERSION
/*******************************************************************************************************/
/*******************************************************************************************************/

		/* The version of this library. This macro also be used as include guard. */
#define CALCH_VERSION 1000

/*-----------------------------------------------------------------------------------------------------*/
		/*	The max length of input expression, includes terminating character. */
#define CALCH_MAX_EXP_LENGTH 256
/*-----------------------------------------------------------------------------------------------------*/
		/* 8 bit signed integer */
typedef signed char CALCH_int8;
		/* 16 bit signed  integer */
typedef signed short CALCH_int16;
		/* 32 bit signed  integer */
typedef signed long CALCH_int32;


#ifdef __STDC_VERSION__
#	if (__STDC_VERSION__ >= 199901L) /* C99 */
			/* if defined, this library supports 64bit integer */
#		define CALCH_SUPPORT_INT64 1
			/* 64 bit signed  integer */
typedef signed long long CALCH_int64;
			/* long double */
typedef long double CALCH_decimal;
#		if (__STDC_VERSION__ >= 201112L) /* C11 */
				/* generic version makes the library so cool */
#			define CALCH_computeExpression(pctstrExp,wExpLength,pResult) _Generic((pResult),													\
															CALCH_int8*:CALCH_computeExpInt8(pctstrExp,wExpLength,(CALCH_int8*) pResult),		\
															CALCH_int16*:CALCH_computeExpInt16(pctstrExp,wExpLength,(CALCH_int16*) pResult),	\
															CALCH_int32*:CALCH_computeExpInt32(pctstrExp,wExpLength,(CALCH_int32*) pResult),	\
															CALCH_int64*:CALCH_computeExpInt64(pctstrExp,wExpLength,(CALCH_int64*) pResult),	\
															CALCH_decimal*:CALCH_computeExpMath(pctstrExp,wExpLength,(CALCH_decimal*) pResult),	\
															default		 :CALCH_ERROR_NOT_SUPPORT)

#			define CALCH_SUPPORT_Generic 1
#		endif
#	else
		/* double */
typedef double CALCH_decimal;
#	endif
#elif (defined(_MSC_VER) && (_MSC_VER>=1900)) || ( defined(__cplusplus) && (__cplusplus>= 201103L) ) /*  VS2015 or C++11 or later */
		/* if defined, this library supports 64bit integer */
#	define CALCH_SUPPORT_INT64 1
		/* 64 bit signed integer */
typedef signed long long CALCH_int64;
		/* long double */
typedef long double CALCH_decimal;
#else
		/* double */
typedef double CALCH_decimal;
#endif
/*******************************************************************************************************/
/*******************************************************************************************************/
		/* errors */
typedef enum tagCalcHelperError
{
	CALCH_NO_ERROR, /* all right */
	CALCH_ERROR_ARGUMENT_NULL, /* the pointer of expression string is null. */
	CALCH_ERROR_INVALID_LENGTH, /* the length of expression is less than 1 or greater than CALCH_MAX_EXP_LENGTH. */
	CALCH_ERROR_INVALID_EXPRESSION, /* invalid expression. */
	CALCH_ERROR_OPERAND_OUT_OF_RANGE, /* the operand is out of range, such as division by zero. */
	CALCH_ERROR_RESULT_OVERFLOW /* the result of mathematics is overflow or underflow.  */
} CALCH_error;
/*******************************************************************************************************/
/*******************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
		/* Programmer mode: BYTE.
		   Note that the wExpLength does not include terminating character.
		*/
	CALCH_error CALCH_computeExpInt8(const char *pctstrExp, int wExpLength, CALCH_int8 *pResult);
		/* Programmer mode: WORD.
		   Note that the wExpLength does not include terminating character.
		*/
	CALCH_error CALCH_computeExpInt16(const char *pctstrExp, int wExpLength, CALCH_int16 *pResult);
		/* Programmer mode: DWORD.
		   Note that the wExpLength does not include terminating character.
		*/
	CALCH_error CALCH_computeExpInt32(const char *pctstrExp, int wExpLength, CALCH_int32 *pResult);
#ifdef CALCH_SUPPORT_INT64
		/* Programmer mode: QWORD.
		   Note that the wExpLength does not include terminating character.
		*/
	CALCH_error CALCH_computeExpInt64(const char *pctstrExp, int wExpLength, CALCH_int64 *pResult);
#endif
		/* Math mode.
		   Note that the wExpLength does not include terminating character.
		*/
	CALCH_error CALCH_computeExpMath(const char *pctstrExp, int wExpLength, CALCH_decimal *pResult);

#ifdef __cplusplus
}
#endif

#endif


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

#include "calcHelper.h"
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <errno.h>
/*******************************************************************************************************/
/*******************************************************************************************************/

#ifdef _MSC_VER
#	define s_inline_kw static __inline
#elif defined (__cplusplus) || (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* C99 */)
#	define s_inline_kw static inline
#elif defined (__GNUC__)
#   define s_inline_kw static __inline__
#else
#	define s_inline_kw static
#endif

/*-----------------------------------------------------------------------------------------------------*/

#ifdef CALCH_SUPPORT_INT64
#	define m_getInt_f(str,str_end,base) strtoll(str,str_end,base)
#	define m_getDec_f(str,str_end) strtold(str,str_end)
/*--------------------------------------------------------- */
#define m_acos_f(x) acosl(x)
#define m_asin_f(x) asinl(x)
#define m_atan_f(x) atanl(x)
#define m_cos_f(x) cosl(x)
#define m_cosh_f(x) coshl(x)
#define m_sin_f(x) sinl(x)
#define m_sinh_f(x) sinhl(x)
#define m_tan_f(x) tanl(x)
#define m_tanh_f(x) tanhl(x)
#define m_exp_f(x) expl(x)
#define m_lg_f(x) log10l(x)
#define m_ln_f(x) logl(x)
#define m_sqrt_f(x) sqrtl(x)
#define m_pow_f(x,y) powl(x,y)
#define m_atan2_f(y,x) atan2l(y,x)
#define m_mod_f(x,y) fmodl(x,y)
/*--------------------------------------------------------- */
#define m_hugeVal_c HUGE_VALL
typedef CALCH_int64 m_intmax_t;
typedef unsigned long long m_uintmax_t;
#else
#	define m_getInt_f(str,str_end,base) strtol(str,str_end,base)
#	define m_getDec_f(str,str_end) strtod(str,str_end)
/*--------------------------------------------------------- */
#define m_acos_f(x) acos(x)
#define m_asin_f(x) asin(x)
#define m_atan_f(x) atan(x)
#define m_cos_f(x) cos(x)
#define m_cosh_f(x) cosh(x)
#define m_sin_f(x) sin(x)
#define m_sinh_f(x) sinh(x)
#define m_tan_f(x) tan(x)
#define m_tanh_f(x) tanh(x)
#define m_exp_f(x) exp(x)
#define m_lg_f(x) log10(x)
#define m_ln_f(x) log(x)
#define m_sqrt_f(x) sqrt(x)
#define m_pow_f(x,y) pow(x,y)
#define m_atan2_f(y,x) atan2(y,x)
#define m_mod_f(x,y) fmod(x,y)
/*--------------------------------------------------------- */
#define m_hugeVal_c HUGE_VAL
typedef CALCH_int32 m_intmax_t;
typedef unsigned long m_uintmax_t;
#endif

typedef unsigned char m_uint8;

typedef union tagCalcHelperNumber {
	CALCH_decimal decValue;
	m_intmax_t intValue;
	m_uintmax_t uintValue;
	int fastIntValue;

}m_number;


/*******************************************************************************************************/
/*******************************************************************************************************/

/*
	Shunting Yard Algorithm support.
	https://en.wikipedia.org/wiki/Shunting-yard_algorithm
*/

/* a struct stores the value and type of a token. */
typedef struct tagCalcHelperToken
{
	m_number mToken; /* token value */
	m_uint8 mInfo;	/* token type */
}m_token;


/* a m_token array, the front of array is a queue for RPN (and a stack for operands and output), and the end of array is a stack for operators.
   Buffer takes up only 129*(CALCH_MAX_EXP_LENGTH - 1) bytes by this design. (assume that long double stored as 16 bytes.)
*/
static struct tagCalcHelperStyard
{
	int queueIndex;
	int queueFrontIdx;
	int frontStackIdx;
	int stackIndex;
	m_token tokens[CALCH_MAX_EXP_LENGTH - 1];
} hiddenYard = { -1,-1,-1,CALCH_MAX_EXP_LENGTH - 1,{{{0.0},0}} };

s_inline_kw void initYard()
{
	hiddenYard.queueIndex = -1;
	hiddenYard.queueFrontIdx = -1;
	hiddenYard.stackIndex = CALCH_MAX_EXP_LENGTH - 1;
}


#define FUNC_STYARD_QUEUE_PUSH(token) hiddenYard.tokens[++hiddenYard.queueIndex]=token
#define FUNC_STYARD_QUEUE_POP hiddenYard.tokens[++hiddenYard.queueFrontIdx]
#define FUNC_STYARD_QUEUE_NOT_EMPTY hiddenYard.queueFrontIdx<hiddenYard.queueIndex

#define FUNC_STYARD_FRONT_STACK_PUSH(token) hiddenYard.tokens[++hiddenYard.frontStackIdx]=token
#define FUNC_STYARD_FRONT_STACK_POP hiddenYard.tokens[hiddenYard.frontStackIdx--]
#define FUNC_STYARD_FRONT_STACK_PEEK hiddenYard.tokens[hiddenYard.frontStackIdx]
#define FUNC_STYARD_FRONT_STACK_COUNT hiddenYard.frontStackIdx+1


#define FUNC_STYARD_STACK_PUSH(token) hiddenYard.tokens[--hiddenYard.stackIndex]=token
#define FUNC_STYARD_STACK_POP hiddenYard.tokens[hiddenYard.stackIndex++]
#define FUNC_STYARD_STACK_DISCARD ++hiddenYard.stackIndex
#define FUNC_STYARD_STACK_PEEK hiddenYard.tokens[hiddenYard.stackIndex]
#define FUNC_STYARD_STACK_NOT_EMPTY ((CALCH_MAX_EXP_LENGTH-1)>hiddenYard.stackIndex)
#define FUNC_STYARD_STACK_TO_QUEUE hiddenYard.tokens[++hiddenYard.queueIndex]=hiddenYard.tokens[hiddenYard.stackIndex++]

/*******************************************************************************************************/
/*******************************************************************************************************/

#define MASK_TOKEN_INFO_OPERAND_INT8  0x08U
#define MASK_TOKEN_INFO_OPERAND_INT16 0x10U
#define MASK_TOKEN_INFO_OPERAND_INT32 0x20U
#define MASK_TOKEN_INFO_OPERAND_INT64 0x40U
#define MASK_TOKEN_INFO_OPERAND_DEC 0x7FU

#define MASK_TOKEN_INFO_IS_OPERATOR 0x80U	/* 10000000 */

/*
 This values are not be used,
 the judgment for priority is enough in this case.

#define MASK_TOKEN_INFO_OPERATOR_ASS_LEFT 0xC0U
#define MASK_TOKEN_INFO_OPERATOR_ASS_RIGHT 0xA0U
#define MASK_TOKEN_INFO_OPERATOR_ARGS_NUM 0x9FU
*/


#define MASK_TOKEN_OPERATOR_PRIORITY 0x7F00
/*--------------------------------------------------------- */
#define FLAG_TOKEN_OPERATOR_NOT 0x0121
/*--------------------------------------------------------- */
#define FLAG_TOKEN_OPERATOR_MUL 0x022A
#define FLAG_TOKEN_OPERATOR_DIV 0x022D
#define FLAG_TOKEN_OPERATOR_MOD 0x0225
/*--------------------------------------------------------- */
#define FLAG_TOKEN_OPERATOR_ADD 0x042B
#define FLAG_TOKEN_OPERATOR_SUB 0x042D
/*--------------------------------------------------------- */
#define FLAG_TOKEN_OPERATOR_SAL 0x083C
#define FLAG_TOKEN_OPERATOR_SAR 0x083E
#define FLAG_TOKEN_OPERATOR_ROL 0x082C
#define FLAG_TOKEN_OPERATOR_ROR 0x082E
#define FLAG_TOKEN_OPERATOR_SHR 0x082F
/*--------------------------------------------------------- */
#define FLAG_TOKEN_OPERATOR_AND 0x1026
/*--------------------------------------------------------- */
#define FLAG_TOKEN_OPERATOR_XOR 0x205E
/*--------------------------------------------------------- */
#define FLAG_TOKEN_OPERATOR_OR 0x407C
/*--------------------------------------------------------- */
#define FLAG_TOKEN_OPERATOR_FUNC 0x4101
/*--------------------------------------------------------- */
#define FLAG_TOKEN_OPERATOR_ACOS 0x4101
#define FLAG_TOKEN_OPERATOR_ASIN 0x4102
#define FLAG_TOKEN_OPERATOR_ATAN 0x4103
#define FLAG_TOKEN_OPERATOR_ATAN2 0x4104
#define FLAG_TOKEN_OPERATOR_COS 0x4105
#define FLAG_TOKEN_OPERATOR_COSH 0x4106
#define FLAG_TOKEN_OPERATOR_EXP 0x4107
#define FLAG_TOKEN_OPERATOR_LG 0x4108
#define FLAG_TOKEN_OPERATOR_LN 0x4109
#define FLAG_TOKEN_OPERATOR_POW 0x410A
#define FLAG_TOKEN_OPERATOR_SQRT 0x410B
#define FLAG_TOKEN_OPERATOR_SIN 0x410C
#define FLAG_TOKEN_OPERATOR_SINH 0x410D
#define FLAG_TOKEN_OPERATOR_TAN 0x410E
#define FLAG_TOKEN_OPERATOR_TANH 0x410F
/*--------------------------------------------------------- */
#define FLAG_TOKEN_OPERATOR_LBRACKET 0x4211
/*-----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/

static struct tagCalcHelperGlobalVar {
	m_intmax_t intMax;
	m_intmax_t intMin;
	const char *pCurrent; /*current pos */
	const char *pEnd;	/* final pos */
	char *pNextPos;		/* next pos */
	int foundLeftBracket;	/* if a '(' was found and handled */
	m_token mOperand;
	m_token mOperator;
} globalVar = { 0,0,NULL,NULL,NULL,0,{{0.0},0U},{{0.0},MASK_TOKEN_INFO_IS_OPERATOR} };


s_inline_kw CALCH_error processRPNFinal()
{
	if (FUNC_STYARD_STACK_NOT_EMPTY) /* if operator stack not empty */
	{
		/* if top of the stack is (,  invalid exp. */
		if (FUNC_STYARD_STACK_PEEK.mToken.fastIntValue == FLAG_TOKEN_OPERATOR_LBRACKET)
		{
			return CALCH_ERROR_INVALID_EXPRESSION;
		}
		while (FUNC_STYARD_STACK_NOT_EMPTY > 0)
		{
			/* pop stack, and push the value onto queue */
			FUNC_STYARD_STACK_TO_QUEUE;
		}
	}
	return CALCH_NO_ERROR;
}

/* IN to RPN (Programmer mode) */
static CALCH_error inToRpnInt()
{
	char cTemp;
	/* the var is changed after ')' handling.
		in this case, next token must be an operator.
	*/
	int canNotBeNumber = 0;
	while (globalVar.pCurrent < globalVar.pEnd )
	{
		if (canNotBeNumber == 0)
		{
			errno = 0;
			globalVar.mOperand.mToken.intValue = m_getInt_f(globalVar.pCurrent, &(globalVar.pNextPos), 0);
			if (errno != 0) /* range error */
			{
				return CALCH_ERROR_OPERAND_OUT_OF_RANGE;
			}
			else if (globalVar.pNextPos != globalVar.pCurrent) /* is number */
			{
				if (globalVar.mOperand.mToken.intValue< globalVar.intMin || globalVar.mOperand.mToken.intValue>globalVar.intMax)
				{
					return CALCH_ERROR_OPERAND_OUT_OF_RANGE;
				}
				FUNC_STYARD_QUEUE_PUSH(globalVar.mOperand);
				if (globalVar.pNextPos >= globalVar.pEnd)
				{
					break;
				}
				globalVar.pCurrent = globalVar.pNextPos;
			}
		}
		else
		{
			canNotBeNumber = 0;
		}

#define STMT_IGNORE_SPACE while(*globalVar.pCurrent==' '){++globalVar.pCurrent;}

		STMT_IGNORE_SPACE

			switch (*globalVar.pCurrent)
			{
#define STMT_SWITCH_BASE_OPERATORS  \
		case '*': \
			globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_MUL; \
			break; \
		case '/': \
			globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_DIV; \
			break; \
		case '%': \
			globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_MOD; \
			break; \
		case '+': \
			globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_ADD; \
			break; \
		case '-': \
			globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_SUB; \
			break; \
		case '(': \
			globalVar.mOperator.mToken.fastIntValue=FLAG_TOKEN_OPERATOR_LBRACKET; \
			FUNC_STYARD_STACK_PUSH(globalVar.mOperator);\
			++globalVar.pCurrent;\
			continue; \
		case ')': \
			globalVar.foundLeftBracket=0; \
			while (FUNC_STYARD_STACK_NOT_EMPTY) \
			{ \
				if (FUNC_STYARD_STACK_PEEK.mToken.fastIntValue == FLAG_TOKEN_OPERATOR_LBRACKET) \
				{\
					FUNC_STYARD_STACK_DISCARD;\
					globalVar.foundLeftBracket = 1;\
					++globalVar.pCurrent; \
					break;\
				}\
				FUNC_STYARD_STACK_TO_QUEUE;\
			}
				STMT_SWITCH_BASE_OPERATORS
					if (globalVar.foundLeftBracket == 1)
					{
						canNotBeNumber = 1;
						continue;
					}
				return CALCH_ERROR_INVALID_EXPRESSION;
			case '!':
				globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_NOT;
				break;
			case '&':
				globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_AND;
				break;
			case '^':
				globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_XOR;
				break;
			case '|':
				globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_OR;
				break;
			case '<':
				if ((++globalVar.pCurrent) < globalVar.pEnd && (*globalVar.pCurrent) == '<' && (globalVar.pCurrent + 1) < globalVar.pEnd)
				{
					if (*(globalVar.pCurrent + 1) == '.')  /* <<. */
					{
						++globalVar.pCurrent;
						globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_ROL;
					}
					else /* << */
					{
						globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_SAL;
					}
					break;
				}
				return CALCH_ERROR_INVALID_EXPRESSION;
			case '>':
				if ((++globalVar.pCurrent) < globalVar.pEnd && (*globalVar.pCurrent) == '>' && (globalVar.pCurrent + 1) < globalVar.pEnd)
				{
					cTemp = *(globalVar.pCurrent + 1);
					if (cTemp == '.') /* >>. */
					{
						++globalVar.pCurrent;
						globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_ROR;
					}
					else if (cTemp == '>') /* >>> */
					{
						++globalVar.pCurrent;
						globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_SHR;
					}
					else /* >> */
					{
						globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_SAR;
					}
					break;
				}
				return CALCH_ERROR_INVALID_EXPRESSION;
			default:
				return CALCH_ERROR_INVALID_EXPRESSION;
			}
		/* if it's a normal operator */
#define STMT_PROCESS_PUSH_NORMAL_OPERATER \
		while (FUNC_STYARD_STACK_NOT_EMPTY && (FUNC_STYARD_STACK_PEEK.mToken.fastIntValue & MASK_TOKEN_OPERATOR_PRIORITY) < (globalVar.mOperator.mToken.fastIntValue & MASK_TOKEN_OPERATOR_PRIORITY)) \
		{ \
			FUNC_STYARD_STACK_TO_QUEUE; \
		} \
		FUNC_STYARD_STACK_PUSH(globalVar.mOperator);\
		++globalVar.pCurrent;

		STMT_PROCESS_PUSH_NORMAL_OPERATER

	}

	return processRPNFinal();
}

/* compute the RPN. (Programmer mode) */
static CALCH_error comExpInt(m_number *pResult)
{
	m_token token, tkOP1, tkOP2;
	/* NOT and SAL may need this value to cut off. */
	m_uintmax_t cutMask = (((m_uintmax_t)-1) >> (64 - globalVar.mOperand.mInfo));
	while (FUNC_STYARD_QUEUE_NOT_EMPTY)
	{
		token = FUNC_STYARD_QUEUE_POP;
		if (token.mInfo == MASK_TOKEN_INFO_IS_OPERATOR)
		{
			switch (token.mToken.fastIntValue)
			{
			case FLAG_TOKEN_OPERATOR_NOT: /* NOT */
#define STMT_ENSURE_STACK_POP_ROP(needed,tokenRightOP) if(FUNC_STYARD_FRONT_STACK_COUNT<needed){return CALCH_ERROR_INVALID_EXPRESSION;}tokenRightOP=FUNC_STYARD_FRONT_STACK_POP;
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.intValue = (~tkOP2.mToken.intValue) & cutMask;
				FUNC_STYARD_FRONT_STACK_PUSH(tkOP2);
				continue;
			case FLAG_TOKEN_OPERATOR_AND:	 /* AND */
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					FUNC_STYARD_FRONT_STACK_PEEK.mToken.uintValue &= tkOP2.mToken.intValue;
				continue;
			case FLAG_TOKEN_OPERATOR_XOR:	 /* XOR */
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					FUNC_STYARD_FRONT_STACK_PEEK.mToken.uintValue ^= tkOP2.mToken.intValue;
				continue;
			case FLAG_TOKEN_OPERATOR_OR:	 /* OR */
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					FUNC_STYARD_FRONT_STACK_PEEK.mToken.uintValue |= tkOP2.mToken.intValue;
				continue;
			case FLAG_TOKEN_OPERATOR_SAL:  /* SAL */
#define STMT_GET_AND_ENSURE_SHIFT  	STMT_ENSURE_STACK_POP_ROP(2, tkOP2)if(tkOP2.mToken.uintValue>=tkOP2.mInfo){return CALCH_ERROR_OPERAND_OUT_OF_RANGE;}
				STMT_GET_AND_ENSURE_SHIFT
					tkOP1 = FUNC_STYARD_FRONT_STACK_PEEK;
				FUNC_STYARD_FRONT_STACK_PEEK.mToken.intValue = (tkOP1.mToken.intValue << tkOP2.mToken.intValue) & cutMask;
				continue;
			case FLAG_TOKEN_OPERATOR_SAR:  /* SAR */
				STMT_GET_AND_ENSURE_SHIFT
					FUNC_STYARD_FRONT_STACK_PEEK.mToken.intValue >>= tkOP2.mToken.intValue;
				continue;
			case FLAG_TOKEN_OPERATOR_ROL:  /* ROL */
				STMT_GET_AND_ENSURE_SHIFT
					tkOP1 = FUNC_STYARD_FRONT_STACK_PEEK;
				FUNC_STYARD_FRONT_STACK_PEEK.mToken.intValue = (tkOP1.mToken.uintValue << tkOP2.mToken.intValue) | (tkOP1.mToken.uintValue >> (tkOP2.mInfo - tkOP2.mToken.intValue));
				continue;
			case FLAG_TOKEN_OPERATOR_ROR: /* ROR */
				STMT_GET_AND_ENSURE_SHIFT
					tkOP1 = FUNC_STYARD_FRONT_STACK_PEEK;
				FUNC_STYARD_FRONT_STACK_PEEK.mToken.intValue = (tkOP1.mToken.uintValue >> tkOP2.mToken.intValue) | (tkOP1.mToken.uintValue << (tkOP2.mInfo - tkOP2.mToken.intValue));
				continue;
			case FLAG_TOKEN_OPERATOR_SHR:	 /* SHR */
				STMT_GET_AND_ENSURE_SHIFT
					FUNC_STYARD_FRONT_STACK_PEEK.mToken.uintValue >>= tkOP2.mToken.intValue;
				continue;
			case FLAG_TOKEN_OPERATOR_MUL:	/* MULTI */
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					tkOP1 = FUNC_STYARD_FRONT_STACK_PEEK;
				if ((tkOP1.mToken.intValue == -1 && tkOP2.mToken.intValue == globalVar.intMin) ||
					(tkOP2.mToken.intValue == -1 && tkOP1.mToken.intValue == globalVar.intMin) ||
					(tkOP1.mToken.intValue > (globalVar.intMax / tkOP2.mToken.intValue)) ||
					(tkOP1.mToken.intValue < (globalVar.intMin / tkOP2.mToken.intValue)))
				{
					return CALCH_ERROR_RESULT_OVERFLOW;
				}
				FUNC_STYARD_FRONT_STACK_PEEK.mToken.intValue *= tkOP2.mToken.intValue;
				continue;
			case FLAG_TOKEN_OPERATOR_DIV:	/* DIV */
#define STMT_GET_AND_ENSURE_DIV(needed,tokenRightOP) STMT_ENSURE_STACK_POP_ROP(needed,tokenRightOP)if(tokenRightOP.mToken.intValue==0){return CALCH_ERROR_OPERAND_OUT_OF_RANGE;}
#define STMT_GET_AND_ENSURE_INT_DIV(needed,tokenRightOP) STMT_GET_AND_ENSURE_DIV(needed,tokenRightOP)if(tokenRightOP.mToken.intValue==-1&&FUNC_STYARD_FRONT_STACK_PEEK.mToken.intValue==globalVar.intMin){return CALCH_ERROR_RESULT_OVERFLOW;}
				STMT_GET_AND_ENSURE_INT_DIV(2, tkOP2)
					FUNC_STYARD_FRONT_STACK_PEEK.mToken.intValue /= tkOP2.mToken.intValue;
				continue;
			case FLAG_TOKEN_OPERATOR_MOD:	/* MOD */
				STMT_GET_AND_ENSURE_INT_DIV(2, tkOP2)
					FUNC_STYARD_FRONT_STACK_PEEK.mToken.intValue %= tkOP2.mToken.intValue;
				continue;
			case FLAG_TOKEN_OPERATOR_ADD:	/* ADD */
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					tkOP1 = FUNC_STYARD_FRONT_STACK_PEEK;
				if ((tkOP2.mToken.intValue > 0 && tkOP1.mToken.intValue > (globalVar.intMax - tkOP2.mToken.intValue)) ||
					(tkOP2.mToken.intValue < 0 && tkOP1.mToken.intValue < (globalVar.intMin - tkOP2.mToken.intValue)))
				{
					return CALCH_ERROR_RESULT_OVERFLOW;
				}
				FUNC_STYARD_FRONT_STACK_PEEK.mToken.intValue += tkOP2.mToken.intValue;
				continue;
			case FLAG_TOKEN_OPERATOR_SUB:	/* SUB */
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					tkOP1 = FUNC_STYARD_FRONT_STACK_PEEK;
				if ((tkOP2.mToken.intValue < 0 && tkOP1.mToken.intValue >(globalVar.intMax + tkOP2.mToken.intValue)) ||
					(tkOP2.mToken.intValue > 0 && tkOP1.mToken.intValue < (globalVar.intMin + tkOP2.mToken.intValue)))
				{
					return CALCH_ERROR_RESULT_OVERFLOW;
				}
				FUNC_STYARD_FRONT_STACK_PEEK.mToken.intValue -= tkOP2.mToken.intValue;
				continue;
			default:
				return CALCH_ERROR_INVALID_EXPRESSION;
			}
		}
		else
		{
			FUNC_STYARD_FRONT_STACK_PUSH(token);
		}
	}

	if (FUNC_STYARD_FRONT_STACK_COUNT != 1)
	{
		return CALCH_ERROR_INVALID_EXPRESSION;
	}
	*pResult = FUNC_STYARD_FRONT_STACK_POP.mToken;
	return CALCH_NO_ERROR;
}

static CALCH_error computeExpIntCore(const char *pctstrExp, int wExpLength, m_number *pResult)
{
#define STMT_INIT_VAR \
	CALCH_error mError; \
	if (pctstrExp==NULL) \
	{ \
		return CALCH_ERROR_ARGUMENT_NULL; \
	} \
	if (wExpLength > CALCH_MAX_EXP_LENGTH || wExpLength < 1) \
	{ \
		return CALCH_ERROR_INVALID_LENGTH;\
	} \
	initYard();\
	globalVar.pCurrent = pctstrExp;\
	globalVar.pEnd = pctstrExp + wExpLength;

	STMT_INIT_VAR
		mError = inToRpnInt();
	if (mError != CALCH_NO_ERROR)
	{
		return  mError;
	}
	mError = comExpInt(pResult);
	if (mError != CALCH_NO_ERROR)
	{
		return  mError;
	}
	return CALCH_NO_ERROR;
}

static const union { int no_used; char isLittleEndian; } uniEndian = { 1 };
/* read an little endian 32bit intege to compare if the next token is function. */
#define FUNC_GET_LITTLE_ENDIAN_INT32(pChar) uniEndian.isLittleEndian==1?(*(CALCH_int32*)pChar):(((CALCH_int32)*pChar)|((CALCH_int32)(*(pChar+1)<<8))|((CALCH_int32)(*(pChar+2)<<16))|((CALCH_int32)(*(pChar+3)<<24)))

/* IN to RPN (Math mode) */
static CALCH_error inToRpnDec()
{
	CALCH_int32 iTemp;
	int iOffset;
	int canNotBeNumber = 0;
	while (globalVar.pCurrent < globalVar.pEnd)
	{
		if (canNotBeNumber == 0)
		{
			errno = 0;
			globalVar.mOperand.mToken.decValue = m_getDec_f(globalVar.pCurrent, &(globalVar.pNextPos));
			if (errno != 0) /* range error */
			{
				return CALCH_ERROR_OPERAND_OUT_OF_RANGE;
			}
			else if (globalVar.pNextPos != globalVar.pCurrent) /* is number */
			{
				FUNC_STYARD_QUEUE_PUSH(globalVar.mOperand);
				if (globalVar.pNextPos >= globalVar.pEnd)
				{
					break;
				}
				globalVar.pCurrent = globalVar.pNextPos;
			}
		}
		else
		{
			canNotBeNumber = 0;
		}
		STMT_IGNORE_SPACE
			switch (*globalVar.pCurrent)
			{
				STMT_SWITCH_BASE_OPERATORS
					if (globalVar.foundLeftBracket == 1)
					{
						if (FUNC_STYARD_STACK_NOT_EMPTY &&  FUNC_STYARD_STACK_PEEK.mToken.fastIntValue > FLAG_TOKEN_OPERATOR_FUNC && FUNC_STYARD_STACK_PEEK.mToken.fastIntValue < FLAG_TOKEN_OPERATOR_LBRACKET) /* if is function */
						{
							FUNC_STYARD_STACK_TO_QUEUE;
						}
						canNotBeNumber = 1;
						continue;
					}
				return CALCH_ERROR_INVALID_EXPRESSION;
			case ',':
				globalVar.foundLeftBracket = 0;
				while (FUNC_STYARD_STACK_NOT_EMPTY)
				{
					if (FUNC_STYARD_STACK_PEEK.mToken.fastIntValue == FLAG_TOKEN_OPERATOR_LBRACKET)
					{
						globalVar.foundLeftBracket = 1;
						break;
					}
					FUNC_STYARD_STACK_TO_QUEUE;
				}
				if (globalVar.foundLeftBracket == 1)
				{
					++globalVar.pCurrent;
					continue;
				}
				return CALCH_ERROR_INVALID_EXPRESSION;
			default:
				if ((globalVar.pCurrent + 4) < globalVar.pEnd)
				{
					iTemp = FUNC_GET_LITTLE_ENDIAN_INT32(globalVar.pCurrent);
					iOffset = 4;
					switch (iTemp)
					{
					case 0x736F6361:  /* acos */
						globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_ACOS;
						break;
					case 0x6E697361:  /* asin */
						globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_ASIN;
						break;
					case 0x68736F63:  /* cosh */
						globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_COSH;
						break;
					case 0x74727173:  /* sqrt */
						globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_SQRT;
						break;
					case 0x686E6973:  /* sinh */
						globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_SINH;
						break;
					case 0x686E6174:  /* tanh */
						globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_TANH;
						break;
					case 0x6E617461:  /* atan */ /* atan2 */
						if (*(globalVar.pCurrent + 4) == '2') /* atan2 */
						{
							globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_ATAN2;
							iOffset = 5;
						}
						else  /* atan */
						{
							globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_ATAN;
						}
						break;
					default:
						iOffset = 3;
						switch (iTemp & 0x00FFFFFF)
						{
						case 0x00736F63:  /* cos */
							globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_COS;
							break;
						case 0x00707865:  /* exp */
							globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_EXP;
							break;
						case 0x00776F70:  /* pow */
							globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_POW;
							break;
						case 0x006E6973:  /* sin */
							globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_SIN;
							break;
						case 0x006E6174:  /* tan */
							globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_TAN;
							break;
						default:
							iOffset = 2;
							switch (iTemp & 0x0000FFFF)
							{
							case 0x0000676C: /*lg*/
								globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_LG;
								break;
							case 0x00006E6C: /*ln*/
								globalVar.mOperator.mToken.fastIntValue = FLAG_TOKEN_OPERATOR_LN;
								break;
							default:
								return CALCH_ERROR_INVALID_EXPRESSION;
							}
							break;
						}
						break;
					}

					globalVar.pCurrent += iOffset;
					FUNC_STYARD_STACK_PUSH(globalVar.mOperator);
					continue;
				}
				else
				{
					return CALCH_ERROR_INVALID_EXPRESSION;
				}
				break;
			}
		/* if it's a normal operator */
		STMT_PROCESS_PUSH_NORMAL_OPERATER
	}
	return processRPNFinal();
}

/* compute the RPN. (Math mode) */
static CALCH_error comExpDec(m_number *pResult)
{
	m_token token, tkOP1, tkOP2;
	while (FUNC_STYARD_QUEUE_NOT_EMPTY)
	{
		token = FUNC_STYARD_QUEUE_POP;

		if (token.mInfo == MASK_TOKEN_INFO_IS_OPERATOR)
		{
			errno = 0;
			switch (token.mToken.fastIntValue)
			{
			case FLAG_TOKEN_OPERATOR_ACOS:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_acos_f(tkOP2.mToken.decValue);
#define STMT_ENSURE_DEC_MATH_PUSH_OP(op) if(errno==ERANGE){return CALCH_ERROR_OPERAND_OUT_OF_RANGE;}else if(errno==EDOM||op.mToken.decValue==m_hugeVal_c){return CALCH_ERROR_RESULT_OVERFLOW;}FUNC_STYARD_FRONT_STACK_PUSH(op);continue;
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_ASIN:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_asin_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_ATAN:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_atan_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_COS:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_cos_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_COSH:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_cosh_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_EXP:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_exp_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_LG:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_lg_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_LN:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_ln_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_SIN:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_sin_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_SINH:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_sinh_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_TAN:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_tan_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_TANH:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_tanh_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_SQRT:
				STMT_ENSURE_STACK_POP_ROP(1, tkOP2)
					tkOP2.mToken.decValue = m_sqrt_f(tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP2)
			case FLAG_TOKEN_OPERATOR_POW:
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					tkOP1 = FUNC_STYARD_FRONT_STACK_POP;
				tkOP1.mToken.decValue = m_pow_f(tkOP1.mToken.decValue, tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP1)
			case FLAG_TOKEN_OPERATOR_ATAN2:
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					tkOP1 = FUNC_STYARD_FRONT_STACK_POP;
				tkOP1.mToken.decValue = m_atan2_f(tkOP1.mToken.decValue, tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP1)
			case FLAG_TOKEN_OPERATOR_MUL:
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					tkOP1 = FUNC_STYARD_FRONT_STACK_POP;
				tkOP1.mToken.decValue = tkOP1.mToken.decValue * tkOP2.mToken.decValue;
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP1)
			case FLAG_TOKEN_OPERATOR_DIV:
				STMT_GET_AND_ENSURE_DIV(2, tkOP2)
					tkOP1 = FUNC_STYARD_FRONT_STACK_POP;
				tkOP1.mToken.decValue = tkOP1.mToken.decValue / tkOP2.mToken.decValue;
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP1)
			case FLAG_TOKEN_OPERATOR_MOD:
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					tkOP1 = FUNC_STYARD_FRONT_STACK_POP;
				tkOP1.mToken.decValue = m_mod_f(tkOP1.mToken.decValue, tkOP2.mToken.decValue);
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP1)
			case FLAG_TOKEN_OPERATOR_ADD:
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					tkOP1 = FUNC_STYARD_FRONT_STACK_POP;
				tkOP1.mToken.decValue = tkOP1.mToken.decValue + tkOP2.mToken.decValue;
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP1)
			case FLAG_TOKEN_OPERATOR_SUB:
				STMT_ENSURE_STACK_POP_ROP(2, tkOP2)
					tkOP1 = FUNC_STYARD_FRONT_STACK_POP;
				tkOP1.mToken.decValue = tkOP1.mToken.decValue - tkOP2.mToken.decValue;
				STMT_ENSURE_DEC_MATH_PUSH_OP(tkOP1)
			default:
				return CALCH_ERROR_INVALID_EXPRESSION;
			}
		}
		else
		{
			FUNC_STYARD_FRONT_STACK_PUSH(token);
		}
	}

	if (FUNC_STYARD_FRONT_STACK_COUNT != 1)
	{
		return CALCH_ERROR_INVALID_EXPRESSION;
	}
	*pResult = FUNC_STYARD_FRONT_STACK_POP.mToken;
	return CALCH_NO_ERROR;
}

static CALCH_error computeExpDecCore(const char *pctstrExp, int wExpLength, m_number *pResult)
{
	STMT_INIT_VAR
		mError = inToRpnDec();
	if (mError != CALCH_NO_ERROR)
	{
		return  mError;
	}
	mError = comExpDec(pResult);
	if (mError != CALCH_NO_ERROR)
	{
		return  mError;
	}
	return CALCH_NO_ERROR;
}

/*******************************************************************************************************/
/*******************************************************************************************************/

CALCH_error CALCH_computeExpInt8(const char *pctstrExp, int wExpLength, CALCH_int8 *pResult)
{
	m_number result;
	CALCH_error err;
	globalVar.mOperand.mInfo = MASK_TOKEN_INFO_OPERAND_INT8;
	globalVar.intMax = SCHAR_MAX;
	globalVar.intMin = SCHAR_MIN;
	err = computeExpIntCore(pctstrExp, wExpLength, &result);
	*pResult = (CALCH_int8)result.intValue;
	return err;
}

CALCH_error CALCH_computeExpInt16(const char *pctstrExp, int wExpLength, CALCH_int16 *pResult)
{
	m_number result;
	CALCH_error err;
	globalVar.mOperand.mInfo = MASK_TOKEN_INFO_OPERAND_INT16;
	globalVar.intMax = SHRT_MAX;
	globalVar.intMin = SHRT_MIN;
	err = computeExpIntCore(pctstrExp, wExpLength, &result);
	*pResult = (CALCH_int16)result.intValue;
	return err;
}

CALCH_error CALCH_computeExpInt32(const char *pctstrExp, int wExpLength, CALCH_int32 *pResult)
{
	m_number result;
	CALCH_error err;
	globalVar.mOperand.mInfo = MASK_TOKEN_INFO_OPERAND_INT32;
	globalVar.intMax = LONG_MAX;
	globalVar.intMin = LONG_MIN;
	err = computeExpIntCore(pctstrExp, wExpLength, &result);
	*pResult = (CALCH_int32)result.intValue;
	return err;
}

#ifdef CALCH_SUPPORT_INT64
CALCH_error CALCH_computeExpInt64(const char *pctstrExp, int wExpLength, CALCH_int64 *pResult)
{
	m_number result;
	CALCH_error err;
	globalVar.mOperand.mInfo = MASK_TOKEN_INFO_OPERAND_INT64;
	globalVar.intMax = LLONG_MAX;
	globalVar.intMin = LLONG_MIN;
	err = computeExpIntCore(pctstrExp, wExpLength, &result);
	*pResult = (CALCH_int64)result.intValue;
	return err;
}
#endif
/*-----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/
CALCH_error CALCH_computeExpMath(const char *pctstrExp, int wExpLength, CALCH_decimal *pResult)
{
	m_number result;
	CALCH_error err;
	globalVar.mOperand.mInfo = MASK_TOKEN_INFO_OPERAND_DEC;
	err = computeExpDecCore(pctstrExp, wExpLength, &result);
	*pResult = result.decValue;
	return err;
}

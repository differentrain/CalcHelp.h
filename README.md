# CalcHelp.h
A portable library written in ANSI C, provides the expression computing functionality, support the computing of integers(can be collectively called "programmer mode") and decimals -- "math mode" .



	Programmer mode has these features :
      Operand-pre-checking  √	
      Overflow-pre-checking √  ( EXCEPT multiplication, "*" )
      Ignore-white-space    √
      Thread-safety	        ×
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
		Operand-checking        √	( EXCEPT division, "/" )
		Overflow-checking       ×	( implemented by <errno.h>(C89), incomplete.)
		Ignore-white-space      √
		Thread-safety           ×
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

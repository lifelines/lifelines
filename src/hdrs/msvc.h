/* WIN32 - MSVC STUFF */


#pragma warning(disable:4057)
/*
4057: 'identifier1' indirection to slightly different base types from 'identifier2'
	This can occur with signed and unsigned, or short and long
*/
#pragma warning(disable:4706)
/*
4706: assignment within conditional expression
*/
#pragma warning(disable:4127)
/*
4127: conditional expression is constant
*/
#pragma warning(disable:4244)
/*
4244: conversion from 'type1' to 'type2', possible loss of data
	This can occur from int to short
*/
#pragma warning(disable:4100)
/*
4100: unreferenced formal parameter
*/

//------------------------------------------------------------------------------
// GB_binop:  hard-coded functions for each built-in binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Unless this file is Generator/GB_binop.c, do not edit it (auto-generated)

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_binop__include.h"

// C=binop(A,B) is defined by the following types and operators:

// A*D function (colscale):  GB_AxD__lt_uint8
// D*A function (rowscale):  GB_DxB__lt_uint8

// C type:   bool
// A type:   uint8_t
// B type:   uint8_t
// BinaryOp: cij = (aij < bij)

#define GB_ATYPE \
    uint8_t

#define GB_BTYPE \
    uint8_t

// aij = Ax [pA]
#define GB_GETA(aij,Ax,pA)  \
    uint8_t aij = Ax [pA]

// bij = Bx [pB]
#define GB_GETB(bij,Bx,pB)  \
    uint8_t bij = Bx [pB]

#define GB_CX(p) Cx [p]

// binary operator
#define GB_BINOP(z, x, y)   \
    z = (x < y) ;

//------------------------------------------------------------------------------
// C = A*D, column scale with diagonal D matrix
//------------------------------------------------------------------------------

void GB_AxD__lt_uint8
(
    GrB_Matrix C,
    const GrB_Matrix A, bool A_is_pattern,
    const GrB_Matrix D, bool D_is_pattern,
    int nthreads
)
{ 
    bool *restrict Cx = C->x ;
    #include "GB_AxB_colscale_meta.c"
}

//------------------------------------------------------------------------------
// C = D*B, row scale with diagonal D matrix
//------------------------------------------------------------------------------

void GB_DxB__lt_uint8
(
    GrB_Matrix C,
    const GrB_Matrix D, bool D_is_pattern,
    const GrB_Matrix B, bool B_is_pattern,
    int nthreads
)
{ 
    bool *restrict Cx = C->x ;
    #include "GB_AxB_rowscale_meta.c"
}

#endif

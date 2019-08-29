function s = isvector (G)
%ISVECTOR determine if the GraphBLAS matrix is a row or column vector.
% isvector (G) is true for an m-by-n GraphBLAS matrix if m or n is 1.
%
% See also issparse, ismatrix, isscalar, sparse, full, isa, gb.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = gbsize (G.opaque) ;
s = (m == 1) || (n == 1) ;

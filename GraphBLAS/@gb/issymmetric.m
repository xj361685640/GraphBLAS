function s = issymmetric (G, option)
%ISSYMMETRIC Determine if a GraphBLAS matrix is real or complex symmetric.
% issymmetric (G) is true if G equals G.' and false otherwise.
% issymmetric (G, 'skew') is true if G equals -G.' and false otherwise.
% issymmetric (G, 'nonskew') is the same as issymmetric (G).
%
% See also ishermitian.

% FUTURE: this will be much faster.  See CHOLMOD/MATLAB/spsym.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m n] = size (G) ;
if (m ~= n)
    s = false ;
else
    if (nargin < 2)
        option = 'nonskew' ;
    end
    if (isequal (option, 'skew'))
        s = (norm (G + G.', 1) == 0) ;
    else
        s = (norm (G - G.', 1) == 0) ;
    end
    if (s)
        % also check the pattern; G might have explicit zeros
        S = logical (spones (G)) ;
        s = isequal (S, S') ;
    end
end


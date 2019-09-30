function [C I] = max (varargin)
%MAX Maximum elements of a GraphBLAS or MATLAB matrix.
%
% C = max (G) is the largest entry in the vector G.  If G is a matrix,
% C is a row vector with C(j) = max (G (:,j)).
%
% C = max (A,B) is an array of the element-wise maximum of two matrices
% A and B, which either have the same size, or one can be a scalar.
% Either A and/or B can be GraphBLAS or MATLAB matrices.
%
% C = max (G, [ ], 'all') is a scalar, with the largest entry in G.
% C = max (G, [ ], 1) is a row vector with C(j) = max (G (:,j))
% C = max (G, [ ], 2) is a column vector with C(i) = max (G (i,:))
%
% The indices of the maximum entry, or [C,I] = max (...) in the MATLAB
% built-in max function, are not computed.  The max (..., nanflag) option
% is not available; only the 'includenan' behavior is supported.
%
% See also min.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

G = varargin {1} ;
[m, n] = size (G) ;
if (isequal (GrB.type (G), 'logical'))
    op = '|.logical' ;
else
    op = 'max' ;
end

if (nargin == 1)

    % C = max (G)
    if (isvector (G))
        % C = max (G) for a vector G results in a scalar C
        C = GrB.reduce (op, G) ;
        if (~GrB.isfull (G))
            C = max (C, 0) ;    % recursively, on a scalar
        end
    else
        % C = max (G) reduces each column to a scalar,
        % giving a 1-by-n row vector.
        C = GrB.vreduce (op, G, struct ('in0', 'transpose')) ;
        % if C(j) < 0, but the column is sparse, then assign C(j) = 0.
        coldegree = GrB.entries (G, 'col', 'degree') ;
        C = GrB.subassign (C, (C < 0) & (coldegree < m), 0)' ;
    end

elseif (nargin == 2)

    % C = max (A,B)
    A = varargin {1} ;
    B = varargin {2} ;
    if (isscalar (A))
        if (isscalar (B))
            % both A and B are scalars.  Result is also a scalar.
            C = gb_sparse_comparator (op, A, B) ;
        else
            % A is a scalar, B is a matrix
            if (gb_get_scalar (A) > 0)
                % since A > 0, the result is full
                [m, n] = size (B) ;
                A = GrB.subassign (GrB (m, n, GrB.type (A)), A, { }, { }) ;
            else
                % since A <= 0, the result is sparse.  Expand the scalar A
                % to the pattern of B.
                A = GrB.expand (A, B) ;
            end
            C = GrB.eadd (op, A, B) ;
        end
    else
        if (isscalar (B))
            % A is a matrix, B is a scalar
            if (gb_get_scalar (B) > 0)
                % since B > 0, the result is full
                [m, n] = size (A) ;
                B = GrB.subassign (GrB (m, n, GrB.type (A)), B, { }, { }) ;
            else
                % since B <= 0, the result is sparse.  Expand the scalar B
                % to the pattern of A.
                B = GrB.expand (B, A) ;
            end
            C = GrB.eadd (op, A, B) ;
        else
            % both A and B are matrices.  Result is sparse.
            C = gb_sparse_comparator (op, A, B) ;
        end
    end

elseif (nargin == 3)

    % C = max (G, [ ], option)
    option = varargin {3} ;
    if (isequal (option, 'all'))
        % C = max (G, [ ] 'all'), reducing all entries to a scalar
        C = GrB.reduce (op, G) ;
        if (~GrB.isfull (G))
            C = max (C, 0) ;    % recursively, on a scalar
        end
    elseif (isequal (option, 1))
        % C = max (G, [ ], 1) reduces each column to a scalar,
        % giving a 1-by-n row vector.
        C = GrB.vreduce (op, G, struct ('in0', 'transpose')) ;
        % if C(j) < 0, but the column is sparse, then assign C(j) = 0.
        coldegree = GrB.entries (G, 'col', 'degree') ;
        C = GrB.subassign (C, (C < 0) & (coldegree < m), 0)' ;
    elseif (isequal (option, 2))
        % C = max (G, [ ], 2) reduces each row to a scalar,
        % giving an m-by-1 column vector.
        C = GrB.vreduce (op, G) ;
        % if C(i) < 0, but the row is sparse, then assign C(i) = 0.
        rowdegree = GrB.entries (G, 'row', 'degree') ;
        C = GrB.subassign (C, (C < 0) & (rowdegree < n), 0) ;
    else
        gb_error ('unknown option') ;
    end

else
    gb_error ('invalid usage') ;
end

function C = graph (G,uplo)
% C = graph (G) is the undirected MATLAB graph of the GraphBLAS matrix G.
% G is used as the adjacency matrix of the graph C.  G must be square and
% symmetric.  No weights are added if G is logical.
%
% With a second argument, G may be unsymmetric.  C = graph (G, 'upper') uses
% the upper triangular part of G.  If C = graph (G, 'lower') uses the lower
% triangular part of G.

% TODO test

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m n] = size (G) ;
if (m ~= n)
    error ('G must be square') ;
end

type = gb.type (G) ;

if (nargin > 1)

    % C = graph (G, uplo)
    if (isequal (type, 'logical'))
        C = graph (logical (G), uplo) ;
    elseif (isequal (type, 'double'))
        C = graph (double (G), uplo) ;
    else
        % all other types (int*, uint*, single)
        if (isequal (uplo, 'lower'))
            [i, j, x] = gb.extracttuples (tril (G)) ;
        elseif (isequal (uplo, 'upper'))
            [i, j, x] = gb.extracttuples (triu (G)) ;
        else
            error ('usage: graph(G,''lower'') or graph(G,''upper'')') ;
        end
        C = graph (i, j, x) ;
    end

else

    % C = graph (G)
    if (isequal (type, 'logical'))
        C = graph (logical (G)) ;
    elseif (isequal (type, 'double'))
        C = graph (double (G)) ;
    else
        % all other types (int*, uint*, single)
        if (~issymmetric (G))
            error ('G must be symmetric') ;
        end
        [i, j, x] = gb.extracttuples (tril (G)) ;
        C = graph (i, j, x) ;
    end

end

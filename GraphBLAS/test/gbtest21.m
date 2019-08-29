function gbtest21
%GBTEST21 test isfinite, isinf, isnan

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
for trial = 1:40
    fprintf ('.') ;
    for m = 0:5
        for n = 0:5
            A = 100 * sprand (m, n, 0.5) ;
            A (1,1) = nan ;
            A (2,2) = inf ;
            G = gb (A) ;

            assert (isequal (isfinite (A), logical (isfinite (G)))) ;
            assert (isequal (isinf    (A), logical (isinf    (G)))) ;
            assert (isequal (isnan    (A), logical (isnan    (G)))) ;

            assert (isrow    (A) == isrow    (G)) ;
            assert (iscolumn (A) == iscolumn (G)) ;
        end
    end
end

fprintf ('\ngbtest21: all tests passed\n') ;

function gbtest24
%GBTEST24 test any, all

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
for trial = 1:40
    fprintf ('.') ;
    for m = 1:5
        for n = 1:5

            MA = sprand (m, n, 0.5) ;
            S = -(sprand (m, n, 0.5) > 0.5) ;
            MA = MA .* S ;

            MB = sprand (m, n, 0.5) ;
            S = -(sprand (m, n, 0.5) > 0.5) ;
            MB = MB .* S ;

            GA = gb (MA) ;
            GB = gb (MB) ;

            c1 = all (MA) ;
            c2 = all (GA) ;
            assert (gbtest_eq (c1, c2)) ;

            c1 = any (MA) ;
            c2 = any (GA) ;
            assert (gbtest_eq (c1, c2)) ;

            % c1 = all (MA, 'all') ;
            c1 = all (all (MA)) ;
            c2 = all (GA, 'all') ;
            assert (gbtest_eq (c1, c2)) ;

            % c1 = any (MA, 'all') ;
            c1 = any (any (MA)) ;
            c2 = any (GA, 'all') ;
            assert (gbtest_eq (c1, c2)) ;

            C1 = all (MA, 1) ;
            C2 = all (GA, 1) ;
            assert (gbtest_eq (C1, C2)) ;

            C1 = any (MA, 1) ;
            C2 = any (GA, 1) ;
            assert (gbtest_eq (C1, C2)) ;

            C1 = all (MA, 2) ;
            C2 = all (GA, 2) ;
            assert (gbtest_eq (C1, C2)) ;

            C1 = any (MA, 2) ;
            C2 = any (GA, 2) ;
            assert (gbtest_eq (C1, C2)) ;

        end
    end
end

fprintf ('\ngbtest24: all tests passed\n') ;


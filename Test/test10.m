% function test10
%TEST10 test GrB_apply

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\ntest10: GrB_apply tests\n') ;

[~, unary_ops, ~, types, ~, ~] = GB_spec_opsall ;

rng ('default') ;

m = 8 ;
n = 4 ;
dt = struct ('inp0', 'tran') ;
dr = struct ('outp', 'replace') ;

% test_types = types.real ;
% test_unops = unary_ops.real ;

test_types = types.all ;
test_unops = unary_ops.all ;


for k1 = 1:length(test_types)
    atype = test_types {k1} ;
    fprintf ('\n%s: ', atype) ;

    Mask = GB_random_mask (m, n, 0.5, true, false) ;
    Cin = GB_spec_random (m, n, 0.3, 100, atype) ;
    Cmask = spones (GB_mex_cast (full (Cin.matrix), Cin.class)) ;

    % for most operators 
    A = GB_spec_random (m, n, 0.3, 100, atype) ;
    B = GB_spec_random (n, m, 0.3, 100, atype) ;

    A_matrix = A.matrix ;
    B_matrix = B.matrix ;

    % for pow, sqrt, log, log10, log2, gammaln (domain is [0,inf])
    A_pos_matrix = abs (A.matrix) ;
    B_pos_matrix = abs (B.matrix) ;

    % for asin, acos, atanh (domain is [-1,1])
    A_1_matrix = A_matrix ;
    B_1_matrix = B_matrix ;
    A_1_matrix (abs (A_matrix) > 1) = 1 ;
    B_1_matrix (abs (B_matrix) > 1) = 1 ;

    % for acosh, asech (domain is [1, inf])
    A_1inf_matrix = A_matrix ;
    B_1inf_matrix = B_matrix ;
    A_1inf_matrix (A_matrix < 1 & A_matrix ~= 0) = 1 ;
    B_1inf_matrix (B_matrix < 1 & B_matrix ~= 0) = 1 ;

    % for log1p (domain is [-1, inf])
    A_n1inf_matrix = A_matrix ;
    B_n1inf_matrix = B_matrix ;
    A_n1inf_matrix (A_matrix < -1) = 1 ;
    B_n1inf_matrix (B_matrix < -1) = 1 ;

    % for tanh: domain is [-inf,inf], but rounding to
    % integers fails when x is outside this range
    A_5_matrix = A_matrix ;
    B_5_matrix = B_matrix ;
    A_5_matrix (abs (A_matrix) > 5) = 5 ;
    B_5_matrix (abs (B_matrix) > 5) = 5 ;

    % for gamma: domain is [-inf,inf], but not defined for negative
    % integers, and rounding to integers fails when x is outside this range
    A_pos5_matrix = A_matrix ;
    B_pos5_matrix = B_matrix ;
    A_pos5_matrix (A_matrix <= 0.1 & A_matrix ~= 0) = 0.1 ;
    B_pos5_matrix (B_matrix <= 0.1 & B_matrix ~= 0) = 0.1 ;
    A_pos5_matrix (A_matrix > 5) = 5 ;
    B_pos5_matrix (B_matrix > 5) = 5 ;

    if (isequal (atype, 'double'))
        hrange = [0 1] ;
        crange = [0 1] ;
    else
        hrange = 0 ;
        crange = 1 ;
    end

    ops = test_unops ;
    for k2 = 1:length(ops)
        op.opname = ops {k2} ;
        fprintf (' %s', op.opname) ;

        for k3 = 1:length(test_types)
            op.optype = test_types {k3} ;

            try
                [opname optype ztype xtype ytype] = GB_spec_operator (op) ;
            catch
                continue 
            end
            fprintf ('.') ;

            A.matrix = A_matrix ;
            B.matrix = B_matrix ;

            if (contains (optype, 'complex'))

                switch (opname)
                    % domain is ok, but limit it to avoid integer typecast
                    % failures from O(eps) errors
                    case { 'tanh', 'tgamma', 'exp' }
                        A.matrix = A_5_matrix ;
                        B.matrix = B_5_matrix ;
                    otherwise
                        % no change
                end

            else

                switch (opname)
                    case { 'pow', 'sqrt', 'log', 'log10', 'log2', ...
                        'gammaln', 'lgamma' }
                        A.matrix = A_pos_matrix ;
                        B.matrix = B_pos_matrix ;
                    case { 'asin', 'acos', 'atanh' }
                        A.matrix = A_1_matrix ;
                        B.matrix = B_1_matrix ;
                    case { 'acosh', 'asech' }
                        A.matrix = A_1inf_matrix ;
                        B.matrix = B_1inf_matrix ;
                    case 'log1p'
                        A.matrix = A_n1inf_matrix ;
                        B.matrix = B_n1inf_matrix ;
                    case { 'tanh', 'exp' }
                        % domain is ok, but limit it to avoid integer typecast
                        % failures from O(eps) errors
                        A.matrix = A_5_matrix ;
                        B.matrix = B_5_matrix ;
                    case { 'tgamma' }
                        A.matrix = A_pos5_matrix ;
                        B.matrix = B_pos5_matrix ;
                    otherwise
                        % no change
                end

            end

            % op

            tol = 0 ;
            if (contains (optype, 'single'))
                tol = 1e-5 ;
            elseif (contains (optype, 'double'))
                tol = 1e-12 ;
            end

            for A_is_hyper = hrange
            for A_is_csc   = crange
            for C_is_hyper = hrange
            for C_is_csc   = crange
            for M_is_hyper = hrange
            for M_is_csc   = crange
            A.is_csc    = A_is_csc ; A.is_hyper    = A_is_hyper ;
            Cin.is_csc  = C_is_csc ; Cin.is_hyper  = C_is_hyper ;
            B.is_csc    = A_is_csc ; B.is_hyper    = A_is_hyper ;
            Mask.is_csc = M_is_csc ; Mask.is_hyper = M_is_hyper ;

            % no mask
            C1 = GB_spec_apply (Cin, [], [], op, A, []) ;
            C2 = GB_mex_apply  (Cin, [], [], op, A, []) ;
            GB_spec_compare (C1, C2, 0, tol) ;

            % no mask, with accum
save gunk Cin op A tol
            C1 = GB_spec_apply (Cin, [], 'plus', op, A, []) ;
            C2 = GB_mex_apply  (Cin, [], 'plus', op, A, []) ;
            GB_spec_compare (C1, C2, 0, tol) ;

            % with mask
            C1 = GB_spec_apply (Cin, Mask, [], op, A, []) ;
            C2 = GB_mex_apply  (Cin, Mask, [], op, A, []) ;
            GB_spec_compare (C1, C2, 0, tol) ;

            % with mask and accum
            C1 = GB_spec_apply (Cin, Mask, 'plus', op, A, []) ;
            C2 = GB_mex_apply  (Cin, Mask, 'plus', op, A, []) ;
            GB_spec_compare (C1, C2, 0, tol) ;

            % with C == mask, and outp = replace
            C1 = GB_spec_apply (Cin, Cmask, [], op, A, dr) ;
            C2 = GB_mex_apply2 (Cin,        [], op, A, dr) ;
            GB_spec_compare (C1, C2, 0, tol) ;

            % with C == mask and accum, and outp = replace
            C1 = GB_spec_apply (Cin, Cmask, 'plus', op, A, dr) ;
            C2 = GB_mex_apply2 (Cin,        'plus', op, A, dr) ;
            GB_spec_compare (C1, C2, 0, tol) ;

            % no mask, transpose
            C1 = GB_spec_apply (Cin, [], [], op, B, dt) ;
            C2 = GB_mex_apply  (Cin, [], [], op, B, dt) ;
            GB_spec_compare (C1, C2, 0, tol) ;

            % no mask, with accum, transpose
            C1 = GB_spec_apply (Cin, [], 'plus', op, B, dt) ;
            C2 = GB_mex_apply  (Cin, [], 'plus', op, B, dt) ;
            GB_spec_compare (C1, C2, 0, tol) ;

            % with mask, transpose
            C1 = GB_spec_apply (Cin, Mask, [], op, B, dt) ;
            C2 = GB_mex_apply  (Cin, Mask, [], op, B, dt) ;
            GB_spec_compare (C1, C2, 0, tol) ;

            % with mask and accum, transpose
            C1 = GB_spec_apply (Cin, Mask, 'plus', op, B, dt) ;
            C2 = GB_mex_apply  (Cin, Mask, 'plus', op, B, dt) ;
            GB_spec_compare (C1, C2, 0, tol) ;

        end
    end

    end
    end
    end
    end
    end
    end
    fprintf ('\n') ;

end

fprintf ('\ntest10: all tests passed\n') ;


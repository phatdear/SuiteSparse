//------------------------------------------------------------------------------
// GB_AxB_saxpy3_coarseGus_M_phase5: C<M>=A*B, coarse Gustavson, phase5
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2025, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // phase5: coarse Gustavson task, C<M>=A*B
    //--------------------------------------------------------------------------

    // Initially, Hf [...] < mark for all of Hf.

    // Hf [i] < mark    : M(i,j)=0, C(i,j) is ignored.
    // Hf [i] == mark   : M(i,j)=1, and C(i,j) not yet seen.
    // Hf [i] == mark+1 : M(i,j)=1, and C(i,j) has been seen.

    for (int64_t kk = kfirst ; kk <= klast ; kk++)
    {
        int64_t pC_start = GB_Cp_IGET (kk) ;
        int64_t pC_end = GB_Cp_IGET (kk+1) ;
        int64_t pC = pC_start ;
        int64_t cjnz = pC_end - pC ;
        ASSERT (pC_start >= 0) ;
        ASSERT (pC_start <= pC_end) ;
        ASSERT (pC_end <= cnz) ;

        if (cjnz == 0) continue ;   // nothing to do
        GB_GET_B_j ;                // get B(:,j)

        #ifndef GB_GENERIC
        if (cjnz == cvlen)          // C(:,j) is dense
        { 
            // This is not used for the generic saxpy3.
            GB_COMPUTE_DENSE_C_j ;  // C(:,j) = A*B(:,j)
            continue ;
        }
        #endif

        GB_GET_M_j ;            // get M(:,j)
        GB_GET_M_j_RANGE (64) ; // get first and last in M(:,j)
        mark += 2 ;
        uint64_t mark1 = mark+1 ;

        // scatter M(:,j) into the Gustavson workspace
        GB_SCATTER_M_j (pM_start, pM_end, mark) ;

        if (16 * cjnz > cvlen)
        {

            //------------------------------------------------------------------
            // C(:,j) is not very sparse
            //------------------------------------------------------------------

            for ( ; pB < pB_end ; pB++)     // scan B(:,j)
            { 
                GB_GET_B_kj_INDEX ;         // get k of B(k,j)
                GB_GET_A_k ;                // get A(:,k)
                if (aknz == 0) continue ;
                GB_GET_B_kj ;               // bkj = B(k,j)
                #define GB_UPDATE_IKJ                               \
                {                                                   \
                    uint64_t hf = Hf [i] ;                          \
                    if (hf == mark)                                 \
                    {                                               \
                        /* C(i,j) = A(i,k) * B(k,j) */              \
                        Hf [i] = mark1 ;        /* mark as seen */  \
                        GB_MULT_A_ik_B_kj ;     /* t = aik*bkj */   \
                        GB_HX_WRITE (i, t) ;    /* Hx [i] = t */    \
                    }                                               \
                    else if (hf == mark1)                           \
                    {                                               \
                        /* C(i,j) += A(i,k) * B(k,j) */             \
                        GB_MULT_A_ik_B_kj ;     /* t = aik*bkj */   \
                        GB_HX_UPDATE (i, t) ;   /* Hx [i] += t */   \
                    }                                               \
                }
                GB_SCAN_M_j_OR_A_k (A_ok_for_binary_search) ;
                #undef GB_UPDATE_IKJ
            }
            GB_GATHER_ALL_C_j(mark1) ;  // gather into C(:,j) 

        }
        else
        {

            //------------------------------------------------------------------
            // C(:,j) is very sparse
            //------------------------------------------------------------------

            for ( ; pB < pB_end ; pB++)     // scan B(:,j)
            { 
                GB_GET_B_kj_INDEX ;         // get k of B(k,j)
                GB_GET_A_k ;                // get A(:,k)
                if (aknz == 0) continue ;
                GB_GET_B_kj ;               // bkj = B(k,j)
                #define GB_UPDATE_IKJ                                   \
                {                                                       \
                    uint64_t hf = Hf [i] ;                              \
                    if (hf == mark)                                     \
                    {                                                   \
                        /* C(i,j) = A(i,k) * B(k,j) */                  \
                        Hf [i] = mark1 ;        /* mark as seen */      \
                        GB_MULT_A_ik_B_kj ;     /* t = aik*bkj */       \
                        GB_HX_WRITE (i, t) ;    /* Hx [i] = t */        \
                        GB_ISET (Ci, pC, i) ;   /* Ci [pC] = i */       \
                        pC++ ;                                          \
                    }                                                   \
                    else if (hf == mark1)                               \
                    {                                                   \
                        /* C(i,j) += A(i,k) * B(k,j) */                 \
                        GB_MULT_A_ik_B_kj ;     /* t = aik*bkj */       \
                        GB_HX_UPDATE (i, t) ;   /* Hx [i] += t */       \
                    }                                                   \
                }
                GB_SCAN_M_j_OR_A_k (A_ok_for_binary_search) ;
                #undef GB_UPDATE_IKJ
            }
            GB_SORT_AND_GATHER_C_j ;    // gather into C(:,j)
        }
    }
}


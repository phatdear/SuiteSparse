//------------------------------------------------------------------------------
// GB_search_for_vector_template: find the vector k that contains p
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2025, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Given an index p, find k so that Ap [k] <= p && p < Ap [k+1].  The search is
// limited to k in the range Ap [kleft ... anvec].

// A->p can come from any matrix: hypersparse, sparse, bitmap, or full.
// For the latter two cases, A->p is NULL.

//------------------------------------------------------------------------------
// GB_search_for_vector_32 and GB_search_for_vector_64
//------------------------------------------------------------------------------

static inline int64_t GB_search_for_vector_TYPE // return vector k
(
    const GB_SV_TYPE *restrict Ap,  // vector pointers to search
    const int64_t p,                // search for vector k that contains p
    const int64_t kleft,            // left-most k to search
    const int64_t anvec,            // Ap is of size anvec+1
    const int64_t avlen             // A->vlen
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (Ap == NULL)
    { 
        // A is full or bitmap
        ASSERT (p >= 0 && p < avlen * anvec) ;
        return ((avlen == 0) ? 0 : (p / avlen)) ;
    }

    // A is sparse or hypersparse
    ASSERT (p >= 0 && p < Ap [anvec]) ;

    //--------------------------------------------------------------------------
    // search for k
    //--------------------------------------------------------------------------

    int64_t k = kleft ;
    int64_t kright = anvec ;
    bool found ;
    found = GB_split_binary_search_TYPE (p, Ap, &k, &kright) ;
    if (found)
    {
        // Ap [k] == p has been found, but if k is an empty vector, then the
        // next vector will also contain the entry p.  In that case, k needs to
        // be incremented until finding the first non-empty vector for which
        // Ap [k] == p.
        ASSERT (Ap [k] == p) ;
        while (k < anvec-1 && Ap [k+1] == p)
        { 
            k++ ;
        }
    }
    else
    { 
        // p has not been found in Ap, so it appears in the middle of Ap [k-1]
        // ... Ap [k], as computed by the binary search.  This is the range of
        // entries for the vector k-1, so k must be decremented.
        k-- ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    // The entry p must reside in a non-empty vector.
    ASSERT (k >= 0 && k < anvec) ;
    ASSERT (Ap [k] <= p && p < Ap [k+1]) ;

    return (k) ;
}

#undef GB_SV_TYPE
#undef GB_search_for_vector_TYPE
#undef GB_split_binary_search_TYPE


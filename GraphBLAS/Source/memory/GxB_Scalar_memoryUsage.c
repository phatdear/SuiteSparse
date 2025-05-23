//------------------------------------------------------------------------------
// GxB_Scalar_memoryUsage: # of bytes used for a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2025, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Scalar_memoryUsage  // return # of bytes used for a scalar
(
    size_t *size,           // # of bytes used by the scalar s
    const GrB_Scalar s      // GrB_Scalar to query
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GB_CHECK_INIT ;
    GB_RETURN_IF_NULL (size) ;
    GB_RETURN_IF_NULL_OR_INVALID (s) ;

    //--------------------------------------------------------------------------
    // get the memory size taken by the scalar
    //--------------------------------------------------------------------------

    int64_t nallocs ;
    size_t mem_shallow ;
    GB_memoryUsage (&nallocs, size, &mem_shallow, (GrB_Matrix) s, false) ;
    if (GB_Global_stats_mem_shallow_get ( ))
    { 
        (*size) += mem_shallow ;
    }
    return (GrB_SUCCESS) ;
}


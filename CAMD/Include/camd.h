//------------------------------------------------------------------------------
// CAMD/Include/camd.h:  constrained approximate minimum degree ordering
//------------------------------------------------------------------------------

// CAMD, Copyright (c) 1996-2024, Timothy A. Davis, Yanqing Chen,
// Patrick R. Amestoy, and Iain S. Duff.  All Rights Reserved.
// SPDX-License-Identifier: BSD-3-clause

//------------------------------------------------------------------------------

/* CAMD finds a symmetric ordering P of a matrix A so that the Cholesky
 * factorization of P*A*P' has fewer nonzeros and takes less work than the
 * Cholesky factorization of A.  If A is not symmetric, then it performs its
 * ordering on the matrix A+A'.  Two sets of user-callable routines are
 * provided, one for int32_t integers and the other for int64_t integers.
 *
 * The method is based on the approximate minimum degree algorithm, discussed
 * in Amestoy, Davis, and Duff, "An approximate degree ordering algorithm",
 * SIAM Journal of Matrix Analysis and Applications, vol. 17, no. 4, pp.
 * 886-905, 1996.
 */

#ifndef CAMD_H
#define CAMD_H

#include "SuiteSparse_config.h"

/* make it easy for C++ programs to include CAMD */
#ifdef __cplusplus
extern "C" {
#endif

int camd_order /* returns CAMD_OK, CAMD_OK_BUT_JUMBLED,
                             * CAMD_INVALID, or CAMD_OUT_OF_MEMORY */
(
    int32_t n,                  /* A is n-by-n.  n must be >= 0. */
    const int32_t Ap [ ],       /* column pointers for A, of size n+1 */
    const int32_t Ai [ ],       /* row indices of A, of size nz = Ap [n] */
    int32_t P [ ],              /* output permutation, of size n */
    double Control [ ],     /* input Control settings, of size CAMD_CONTROL */
    double Info [ ],        /* output Info statistics, of size CAMD_INFO */
    const int32_t C [ ]         /* Constraint set of A, of size n; can be NULL */
) ;

int camd_l_order   /* see above for description */
(
    int64_t n,
    const int64_t Ap [ ],
    const int64_t Ai [ ],
    int64_t P [ ],
    double Control [ ],
    double Info [ ],
    const int64_t C [ ]
) ;

/* Input arguments (not modified):
 *
 *      n: the matrix A is n-by-n.
 *      Ap: an int32_t/int64_t array of size n+1, containing column
 *          pointers of A.
 *      Ai: an int32_t/int64_t array of size nz, containing the row
 *          indices of A, where nz = Ap [n].
 *      Control:  a double array of size CAMD_CONTROL, containing control
 *          parameters.  Defaults are used if Control is NULL.
 *
 * Output arguments (not defined on input):
 *
 *      P: an int32_t/int64_t array of size n, containing the output
 *          permutation. If row i is the kth pivot row, then P [k] = i.  In
 *          MATLAB notation, the reordered matrix is A (P,P).
 *      Info: a double array of size CAMD_INFO, containing statistical
 *          information.  Ignored if Info is NULL.
 *
 * On input, the matrix A is stored in column-oriented form.  The row indices
 * of nonzero entries in column j are stored in Ai [Ap [j] ... Ap [j+1]-1].
 *
 * If the row indices appear in ascending order in each column, and there
 * are no duplicate entries, then camd_order is slightly more efficient in
 * terms of time and memory usage.  If this condition does not hold, a copy
 * of the matrix is created (where these conditions do hold), and the copy is
 * ordered.
 *
 * Row indices must be in the range 0 to
 * n-1.  Ap [0] must be zero, and thus nz = Ap [n] is the number of nonzeros
 * in A.  The array Ap is of size n+1, and the array Ai is of size nz = Ap [n].
 * The matrix does not need to be symmetric, and the diagonal does not need to
 * be present (if diagonal entries are present, they are ignored except for
 * the output statistic Info [CAMD_NZDIAG]).  The arrays Ai and Ap are not
 * modified.  This form of the Ap and Ai arrays to represent the nonzero
 * pattern of the matrix A is the same as that used internally by MATLAB.
 * If you wish to use a more flexible input structure, please see the
 * umfpack_*_triplet_to_col routines in the UMFPACK package, at
 * http://www.suitesparse.com.
 *
 * Restrictions:  n >= 0.  Ap [0] = 0.  Ap [j] <= Ap [j+1] for all j in the
 *      range 0 to n-1.  nz = Ap [n] >= 0.  Ai [0..nz-1] must be in the range 0
 *      to n-1.  Finally, Ai, Ap, and P must not be NULL.  If any of these
 *      restrictions are not met, CAMD returns CAMD_INVALID.
 *
 * CAMD returns:
 *
 *      CAMD_OK if the matrix is valid and sufficient memory can be allocated to
 *          perform the ordering.
 *
 *      CAMD_OUT_OF_MEMORY if not enough memory can be allocated.
 *
 *      CAMD_INVALID if the input arguments n, Ap, Ai are invalid, or if P is
 *          NULL.
 *
 *      CAMD_OK_BUT_JUMBLED if the matrix had unsorted columns, and/or duplicate
 *          entries, but was otherwise valid.
 *
 * The CAMD routine first forms the pattern of the matrix A+A', and then
 * computes a fill-reducing ordering, P.  If P [k] = i, then row/column i of
 * the original is the kth pivotal row.  In MATLAB notation, the permuted
 * matrix is A (P,P), except that 0-based indexing is used instead of the
 * 1-based indexing in MATLAB.
 *
 * The Control array is used to set various parameters for CAMD.  If a NULL
 * pointer is passed, default values are used.  The Control array is not
 * modified.
 *
 *      Control [CAMD_DENSE]:  controls the threshold for "dense" rows/columns.
 *          A dense row/column in A+A' can cause CAMD to spend a lot of time in
 *          ordering the matrix.  If Control [CAMD_DENSE] >= 0, rows/columns
 *          with more than Control [CAMD_DENSE] * sqrt (n) entries are ignored
 *          during the ordering, and placed last in the output order.  The
 *          default value of Control [CAMD_DENSE] is 10.  If negative, no
 *          rows/columns are treated as "dense".  Rows/columns with 16 or
 *          fewer off-diagonal entries are never considered "dense".
 *
 *      Control [CAMD_AGGRESSIVE]: controls whether or not to use aggressive
 *          absorption, in which a prior element is absorbed into the current
 *          element if is a subset of the current element, even if it is not
 *          adjacent to the current pivot element (refer to Amestoy, Davis,
 *          & Duff, 1996, for more details).  The default value is nonzero,
 *          which means to perform aggressive absorption.  This nearly always
 *          leads to a better ordering (because the approximate degrees are
 *          more accurate) and a lower execution time.  There are cases where
 *          it can lead to a slightly worse ordering, however.  To turn it off,
 *          set Control [CAMD_AGGRESSIVE] to 0.
 *
 *      Control [2..4] are not used in the current version, but may be used in
 *          future versions.
 *
 * The Info array provides statistics about the ordering on output.  If it is
 * not present, the statistics are not returned.  This is not an error
 * condition.
 * 
 *      Info [CAMD_STATUS]:  the return value of CAMD, either CAMD_OK,
 *          CAMD_OK_BUT_JUMBLED, CAMD_OUT_OF_MEMORY, or CAMD_INVALID.
 *
 *      Info [CAMD_N]: n, the size of the input matrix
 *
 *      Info [CAMD_NZ]: the number of nonzeros in A, nz = Ap [n]
 *
 *      Info [CAMD_SYMMETRY]:  the symmetry of the matrix A.  It is the number
 *          of "matched" off-diagonal entries divided by the total number of
 *          off-diagonal entries.  An entry A(i,j) is matched if A(j,i) is also
 *          an entry, for any pair (i,j) for which i != j.  In MATLAB notation,
 *              S = spones (A) ;
 *              B = tril (S, -1) + triu (S, 1) ;
 *              symmetry = nnz (B & B') / nnz (B) ;
 *
 *      Info [CAMD_NZDIAG]: the number of entries on the diagonal of A.
 *
 *      Info [CAMD_NZ_A_PLUS_AT]:  the number of nonzeros in A+A', excluding the
 *          diagonal.  If A is perfectly symmetric (Info [CAMD_SYMMETRY] = 1)
 *          with a fully nonzero diagonal, then Info [CAMD_NZ_A_PLUS_AT] = nz-n
 *          (the smallest possible value).  If A is perfectly unsymmetric
 *          (Info [CAMD_SYMMETRY] = 0, for an upper triangular matrix, for
 *          example) with no diagonal, then Info [CAMD_NZ_A_PLUS_AT] = 2*nz
 *          (the largest possible value).
 *
 *      Info [CAMD_NDENSE]: the number of "dense" rows/columns of A+A' that were
 *          removed from A prior to ordering.  These are placed last in the
 *          output order P.
 *
 *      Info [CAMD_MEMORY]: the amount of memory used by CAMD, in bytes.  In the
 *          current version, this is 1.2 * Info  [CAMD_NZ_A_PLUS_AT] + 9*n
 *          times the size of an integer.  This is at most 2.4nz + 9n.  This
 *          excludes the size of the input arguments Ai, Ap, and P, which have
 *          a total size of nz + 2*n + 1 integers.
 *
 *      Info [CAMD_NCMPA]: the number of garbage collections performed.
 *
 *      Info [CAMD_LNZ]: the number of nonzeros in L (excluding the diagonal).
 *          This is a slight upper bound because mass elimination is combined
 *          with the approximate degree update.  It is a rough upper bound if
 *          there are many "dense" rows/columns.  The rest of the statistics,
 *          below, are also slight or rough upper bounds, for the same reasons.
 *          The post-ordering of the assembly tree might also not exactly
 *          correspond to a true elimination tree postordering.
 *
 *      Info [CAMD_NDIV]: the number of divide operations for a subsequent LDL'
 *          or LU factorization of the permuted matrix A (P,P).
 *
 *      Info [CAMD_NMULTSUBS_LDL]:  the number of multiply-subtract pairs for a
 *          subsequent LDL' factorization of A (P,P).
 *
 *      Info [CAMD_NMULTSUBS_LU]:  the number of multiply-subtract pairs for a
 *          subsequent LU factorization of A (P,P), assuming that no numerical
 *          pivoting is required.
 *
 *      Info [CAMD_DMAX]:  the maximum number of nonzeros in any column of L,
 *          including the diagonal.
 *
 *      Info [14..19] are not used in the current version, but may be used in
 *          future versions.
 */    

/* ------------------------------------------------------------------------- */
/* direct interface to CAMD */
/* ------------------------------------------------------------------------- */

/* camd_2 is the primary CAMD ordering routine.  It is not meant to be
 * user-callable because of its restrictive inputs and because it destroys
 * the user's input matrix.  It does not check its inputs for errors, either.
 * However, if you can work with these restrictions it can be faster than
 * camd_order and use less memory (assuming that you can create your own copy
 * of the matrix for CAMD to destroy).  Refer to CAMD/Source/camd_2.c for a
 * description of each parameter. */

void camd_2
(
    int32_t n,
    int32_t Pe [ ],
    int32_t Iw [ ],
    int32_t Len [ ],
    int32_t iwlen,
    int32_t pfree,
    int32_t Nv [ ],
    int32_t Next [ ], 
    int32_t Last [ ],
    int32_t Head [ ],
    int32_t Elen [ ],
    int32_t Degree [ ],
    int32_t W [ ],
    double Control [ ],
    double Info [ ],
    const int32_t C [ ],
    int32_t BucketSet [ ] 
) ;

void camd_l2
(
    int64_t n,
    int64_t Pe [ ],
    int64_t Iw [ ],
    int64_t Len [ ],
    int64_t iwlen,
    int64_t pfree,
    int64_t Nv [ ],
    int64_t Next [ ], 
    int64_t Last [ ],
    int64_t Head [ ],
    int64_t Elen [ ],
    int64_t Degree [ ],
    int64_t W [ ],
    double Control [ ],
    double Info [ ],
    const int64_t C [ ],
    int64_t BucketSet [ ]
    
) ;

/* ------------------------------------------------------------------------- */
/* camd_valid */
/* ------------------------------------------------------------------------- */

/* Returns CAMD_OK or CAMD_OK_BUT_JUMBLED if the matrix is valid as input to
 * camd_order; the latter is returned if the matrix has unsorted and/or
 * duplicate row indices in one or more columns.  Returns CAMD_INVALID if the
 * matrix cannot be passed to camd_order.  For camd_order, the matrix must also
 * be square.  The first two arguments are the number of rows and the number
 * of columns of the matrix.  For its use in CAMD, these must both equal n.
 */

int camd_valid
(
    int32_t n_row,              /* # of rows */
    int32_t n_col,              /* # of columns */
    const int32_t Ap [ ],       /* column pointers, of size n_col+1 */
    const int32_t Ai [ ]        /* row indices, of size Ap [n_col] */
) ;

int camd_l_valid
(
    int64_t n_row,
    int64_t n_col,
    const int64_t Ap [ ],
    const int64_t Ai [ ]
) ;

/* ------------------------------------------------------------------------- */
/* camd_cvalid */
/* ------------------------------------------------------------------------- */

/* Returns TRUE if the constraint set is valid as input to camd_order,
 * FALSE otherwise. */

int camd_cvalid
(
   int32_t n,
   const int32_t C [ ]
) ;

int camd_l_cvalid
(
   int64_t n,
   const int64_t C [ ]
) ;

/* ------------------------------------------------------------------------- */
/* CAMD Control and Info arrays */
/* ------------------------------------------------------------------------- */

/* camd_defaults:  sets the default control settings */
void camd_defaults   (double Control [ ]) ;
void camd_l_defaults (double Control [ ]) ;

/* camd_control: prints the control settings */
void camd_control    (double Control [ ]) ;
void camd_l_control  (double Control [ ]) ;

/* camd_info: prints the statistics */
void camd_info       (double Info [ ]) ;
void camd_l_info     (double Info [ ]) ;

// camd_version: return CAMD version.  The version array is returned with
// version [0..2] = {CAMD_MAIN_VERSION, CAMD_SUB_VERSION, CAMD_SUBSUB_VERSION}
void camd_version (int version [3]) ;

#ifdef __cplusplus
}
#endif

#define CAMD_CONTROL 5      /* size of Control array */
#define CAMD_INFO 20        /* size of Info array */

/* contents of Control */
#define CAMD_DENSE 0        /* "dense" if degree > Control [0] * sqrt (n) */
#define CAMD_AGGRESSIVE 1    /* do aggressive absorption if Control [1] != 0 */

/* default Control settings */
#define CAMD_DEFAULT_DENSE 10.0     /* default "dense" degree 10*sqrt(n) */
#define CAMD_DEFAULT_AGGRESSIVE 1    /* do aggressive absorption by default */

/* contents of Info */
#define CAMD_STATUS 0       /* return value of camd_order and camd_l_order */
#define CAMD_N 1                    /* A is n-by-n */
#define CAMD_NZ 2           /* number of nonzeros in A */ 
#define CAMD_SYMMETRY 3     /* symmetry of pattern (1 is sym., 0 is unsym.) */
#define CAMD_NZDIAG 4       /* # of entries on diagonal */
#define CAMD_NZ_A_PLUS_AT 5  /* nz in A+A' */
#define CAMD_NDENSE 6       /* number of "dense" rows/columns in A */
#define CAMD_MEMORY 7       /* amount of memory used by CAMD */
#define CAMD_NCMPA 8        /* number of garbage collections in CAMD */
#define CAMD_LNZ 9          /* approx. nz in L, excluding the diagonal */
#define CAMD_NDIV 10        /* number of fl. point divides for LU and LDL' */
#define CAMD_NMULTSUBS_LDL 11 /* number of fl. point (*,-) pairs for LDL' */
#define CAMD_NMULTSUBS_LU 12  /* number of fl. point (*,-) pairs for LU */
#define CAMD_DMAX 13         /* max nz. in any column of L, incl. diagonal */

/* ------------------------------------------------------------------------- */
/* return values of CAMD */
/* ------------------------------------------------------------------------- */

#define CAMD_OK 0               /* success */
#define CAMD_OUT_OF_MEMORY -1   /* malloc failed, or problem too large */
#define CAMD_INVALID -2         /* input arguments are not valid */
#define CAMD_OK_BUT_JUMBLED 1   /* input matrix is OK for camd_order, but
    * columns were not sorted, and/or duplicate entries were present.  CAMD had
    * to do extra work before ordering the matrix.  This is a warning, not an
    * error.  */

/* ========================================================================== */
/* === CAMD version ========================================================= */
/* ========================================================================== */

/*
 * As an example, to test if the version you are using is 1.2 or later:
 *
 *      if (CAMD_VERSION >= CAMD_VERSION_CODE (1,2)) ...
 *
 * This also works during compile-time:
 *
 *      #if (CAMD_VERSION >= CAMD_VERSION_CODE (1,2))
 *          printf ("This is version 1.2 or later\n") ;
 *      #else
 *          printf ("This is an early version\n") ;
 *      #endif
 */

#define CAMD_DATE "May 5, 2025"
#define CAMD_MAIN_VERSION   3
#define CAMD_SUB_VERSION    3
#define CAMD_SUBSUB_VERSION 4

#define CAMD_VERSION_CODE(main,sub) SUITESPARSE_VER_CODE(main,sub)
#define CAMD_VERSION CAMD_VERSION_CODE(3,3)

#define CAMD__VERSION SUITESPARSE__VERCODE(3,3,4)
#if !defined (SUITESPARSE__VERSION) || \
    (SUITESPARSE__VERSION < SUITESPARSE__VERCODE(7,10,3))
#error "CAMD 3.3.4 requires SuiteSparse_config 7.10.3 or later"
#endif

#endif

/*****************************************************************************
  Copyright (c) 2010, Intel Corp.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/
/*  Contents: test routine for C interface to LAPACK
*   Author: Intel Corporation
*   Created in March, 2010
*
* Purpose
*
* cpprfs_1 is the test program for the C interface to LAPACK
* routine cpprfs
* The program doesn't require an input, the input data is hardcoded in the
* test program.
* The program tests the C interface in the four combinations:
*   1) column-major layout, middle-level interface
*   2) column-major layout, high-level interface
*   3) row-major layout, middle-level interface
*   4) row-major layout, high-level interface
* The output of the C interface function is compared to those obtained from
* the corresponiding LAPACK routine with the same input data, and the
* comparison diagnostics is then printed on the standard output having PASSED
* keyword if the test is passed, and FAILED keyword if the test isn't passed.
*****************************************************************************/
#include <stdio.h>
#include "lapacke.h"
#include "lapacke_utils.h"
#include "test_utils.h"

static void init_scalars_cpprfs( char *uplo, lapack_int *n, lapack_int *nrhs,
                                 lapack_int *ldb, lapack_int *ldx );
static void init_ap( lapack_int size, lapack_complex_float *ap );
static void init_afp( lapack_int size, lapack_complex_float *afp );
static void init_b( lapack_int size, lapack_complex_float *b );
static void init_x( lapack_int size, lapack_complex_float *x );
static void init_ferr( lapack_int size, float *ferr );
static void init_berr( lapack_int size, float *berr );
static void init_work( lapack_int size, lapack_complex_float *work );
static void init_rwork( lapack_int size, float *rwork );
static int compare_cpprfs( lapack_complex_float *x, lapack_complex_float *x_i,
                           float *ferr, float *ferr_i, float *berr,
                           float *berr_i, lapack_int info, lapack_int info_i,
                           lapack_int ldx, lapack_int nrhs );

int main(void)
{
    /* Local scalars */
    char uplo, uplo_i;
    lapack_int n, n_i;
    lapack_int nrhs, nrhs_i;
    lapack_int ldb, ldb_i;
    lapack_int ldb_r;
    lapack_int ldx, ldx_i;
    lapack_int ldx_r;
    lapack_int info, info_i;
    lapack_int i;
    int failed;

    /* Local arrays */
    lapack_complex_float *ap = NULL, *ap_i = NULL;
    lapack_complex_float *afp = NULL, *afp_i = NULL;
    lapack_complex_float *b = NULL, *b_i = NULL;
    lapack_complex_float *x = NULL, *x_i = NULL;
    float *ferr = NULL, *ferr_i = NULL;
    float *berr = NULL, *berr_i = NULL;
    lapack_complex_float *work = NULL, *work_i = NULL;
    float *rwork = NULL, *rwork_i = NULL;
    lapack_complex_float *x_save = NULL;
    float *ferr_save = NULL;
    float *berr_save = NULL;
    lapack_complex_float *ap_r = NULL;
    lapack_complex_float *afp_r = NULL;
    lapack_complex_float *b_r = NULL;
    lapack_complex_float *x_r = NULL;

    /* Iniitialize the scalar parameters */
    init_scalars_cpprfs( &uplo, &n, &nrhs, &ldb, &ldx );
    ldb_r = nrhs+2;
    ldx_r = nrhs+2;
    uplo_i = uplo;
    n_i = n;
    nrhs_i = nrhs;
    ldb_i = ldb;
    ldx_i = ldx;

    /* Allocate memory for the LAPACK routine arrays */
    ap = (lapack_complex_float *)
        LAPACKE_malloc( ((n*(n+1)/2)) * sizeof(lapack_complex_float) );
    afp = (lapack_complex_float *)
        LAPACKE_malloc( ((n*(n+1)/2)) * sizeof(lapack_complex_float) );
    b = (lapack_complex_float *)
        LAPACKE_malloc( ldb*nrhs * sizeof(lapack_complex_float) );
    x = (lapack_complex_float *)
        LAPACKE_malloc( ldx*nrhs * sizeof(lapack_complex_float) );
    ferr = (float *)LAPACKE_malloc( nrhs * sizeof(float) );
    berr = (float *)LAPACKE_malloc( nrhs * sizeof(float) );
    work = (lapack_complex_float *)
        LAPACKE_malloc( 2*n * sizeof(lapack_complex_float) );
    rwork = (float *)LAPACKE_malloc( n * sizeof(float) );

    /* Allocate memory for the C interface function arrays */
    ap_i = (lapack_complex_float *)
        LAPACKE_malloc( ((n*(n+1)/2)) * sizeof(lapack_complex_float) );
    afp_i = (lapack_complex_float *)
        LAPACKE_malloc( ((n*(n+1)/2)) * sizeof(lapack_complex_float) );
    b_i = (lapack_complex_float *)
        LAPACKE_malloc( ldb*nrhs * sizeof(lapack_complex_float) );
    x_i = (lapack_complex_float *)
        LAPACKE_malloc( ldx*nrhs * sizeof(lapack_complex_float) );
    ferr_i = (float *)LAPACKE_malloc( nrhs * sizeof(float) );
    berr_i = (float *)LAPACKE_malloc( nrhs * sizeof(float) );
    work_i = (lapack_complex_float *)
        LAPACKE_malloc( 2*n * sizeof(lapack_complex_float) );
    rwork_i = (float *)LAPACKE_malloc( n * sizeof(float) );

    /* Allocate memory for the backup arrays */
    x_save = (lapack_complex_float *)
        LAPACKE_malloc( ldx*nrhs * sizeof(lapack_complex_float) );
    ferr_save = (float *)LAPACKE_malloc( nrhs * sizeof(float) );
    berr_save = (float *)LAPACKE_malloc( nrhs * sizeof(float) );

    /* Allocate memory for the row-major arrays */
    ap_r = (lapack_complex_float *)
        LAPACKE_malloc( n*(n+1)/2 * sizeof(lapack_complex_float) );
    afp_r = (lapack_complex_float *)
        LAPACKE_malloc( n*(n+1)/2 * sizeof(lapack_complex_float) );
    b_r = (lapack_complex_float *)
        LAPACKE_malloc( n*(nrhs+2) * sizeof(lapack_complex_float) );
    x_r = (lapack_complex_float *)
        LAPACKE_malloc( n*(nrhs+2) * sizeof(lapack_complex_float) );

    /* Initialize input arrays */
    init_ap( (n*(n+1)/2), ap );
    init_afp( (n*(n+1)/2), afp );
    init_b( ldb*nrhs, b );
    init_x( ldx*nrhs, x );
    init_ferr( nrhs, ferr );
    init_berr( nrhs, berr );
    init_work( 2*n, work );
    init_rwork( n, rwork );

    /* Backup the ouptut arrays */
    for( i = 0; i < ldx*nrhs; i++ ) {
        x_save[i] = x[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        ferr_save[i] = ferr[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        berr_save[i] = berr[i];
    }

    /* Call the LAPACK routine */
    cpprfs_( &uplo, &n, &nrhs, ap, afp, b, &ldb, x, &ldx, ferr, berr, work,
             rwork, &info );

    /* Initialize input data, call the column-major middle-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < (n*(n+1)/2); i++ ) {
        ap_i[i] = ap[i];
    }
    for( i = 0; i < (n*(n+1)/2); i++ ) {
        afp_i[i] = afp[i];
    }
    for( i = 0; i < ldb*nrhs; i++ ) {
        b_i[i] = b[i];
    }
    for( i = 0; i < ldx*nrhs; i++ ) {
        x_i[i] = x_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        ferr_i[i] = ferr_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        berr_i[i] = berr_save[i];
    }
    for( i = 0; i < 2*n; i++ ) {
        work_i[i] = work[i];
    }
    for( i = 0; i < n; i++ ) {
        rwork_i[i] = rwork[i];
    }
    info_i = LAPACKE_cpprfs_work( LAPACK_COL_MAJOR, uplo_i, n_i, nrhs_i, ap_i,
                                  afp_i, b_i, ldb_i, x_i, ldx_i, ferr_i, berr_i,
                                  work_i, rwork_i );

    failed = compare_cpprfs( x, x_i, ferr, ferr_i, berr, berr_i, info, info_i,
                             ldx, nrhs );
    if( failed == 0 ) {
        printf( "PASSED: column-major middle-level interface to cpprfs\n" );
    } else {
        printf( "FAILED: column-major middle-level interface to cpprfs\n" );
    }

    /* Initialize input data, call the column-major high-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < (n*(n+1)/2); i++ ) {
        ap_i[i] = ap[i];
    }
    for( i = 0; i < (n*(n+1)/2); i++ ) {
        afp_i[i] = afp[i];
    }
    for( i = 0; i < ldb*nrhs; i++ ) {
        b_i[i] = b[i];
    }
    for( i = 0; i < ldx*nrhs; i++ ) {
        x_i[i] = x_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        ferr_i[i] = ferr_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        berr_i[i] = berr_save[i];
    }
    for( i = 0; i < 2*n; i++ ) {
        work_i[i] = work[i];
    }
    for( i = 0; i < n; i++ ) {
        rwork_i[i] = rwork[i];
    }
    info_i = LAPACKE_cpprfs( LAPACK_COL_MAJOR, uplo_i, n_i, nrhs_i, ap_i, afp_i,
                             b_i, ldb_i, x_i, ldx_i, ferr_i, berr_i );

    failed = compare_cpprfs( x, x_i, ferr, ferr_i, berr, berr_i, info, info_i,
                             ldx, nrhs );
    if( failed == 0 ) {
        printf( "PASSED: column-major high-level interface to cpprfs\n" );
    } else {
        printf( "FAILED: column-major high-level interface to cpprfs\n" );
    }

    /* Initialize input data, call the row-major middle-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < (n*(n+1)/2); i++ ) {
        ap_i[i] = ap[i];
    }
    for( i = 0; i < (n*(n+1)/2); i++ ) {
        afp_i[i] = afp[i];
    }
    for( i = 0; i < ldb*nrhs; i++ ) {
        b_i[i] = b[i];
    }
    for( i = 0; i < ldx*nrhs; i++ ) {
        x_i[i] = x_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        ferr_i[i] = ferr_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        berr_i[i] = berr_save[i];
    }
    for( i = 0; i < 2*n; i++ ) {
        work_i[i] = work[i];
    }
    for( i = 0; i < n; i++ ) {
        rwork_i[i] = rwork[i];
    }

    LAPACKE_cpp_trans( LAPACK_COL_MAJOR, uplo, n, ap_i, ap_r );
    LAPACKE_cpp_trans( LAPACK_COL_MAJOR, uplo, n, afp_i, afp_r );
    LAPACKE_cge_trans( LAPACK_COL_MAJOR, n, nrhs, b_i, ldb, b_r, nrhs+2 );
    LAPACKE_cge_trans( LAPACK_COL_MAJOR, n, nrhs, x_i, ldx, x_r, nrhs+2 );
    info_i = LAPACKE_cpprfs_work( LAPACK_ROW_MAJOR, uplo_i, n_i, nrhs_i, ap_r,
                                  afp_r, b_r, ldb_r, x_r, ldx_r, ferr_i, berr_i,
                                  work_i, rwork_i );

    LAPACKE_cge_trans( LAPACK_ROW_MAJOR, n, nrhs, x_r, nrhs+2, x_i, ldx );

    failed = compare_cpprfs( x, x_i, ferr, ferr_i, berr, berr_i, info, info_i,
                             ldx, nrhs );
    if( failed == 0 ) {
        printf( "PASSED: row-major middle-level interface to cpprfs\n" );
    } else {
        printf( "FAILED: row-major middle-level interface to cpprfs\n" );
    }

    /* Initialize input data, call the row-major high-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < (n*(n+1)/2); i++ ) {
        ap_i[i] = ap[i];
    }
    for( i = 0; i < (n*(n+1)/2); i++ ) {
        afp_i[i] = afp[i];
    }
    for( i = 0; i < ldb*nrhs; i++ ) {
        b_i[i] = b[i];
    }
    for( i = 0; i < ldx*nrhs; i++ ) {
        x_i[i] = x_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        ferr_i[i] = ferr_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        berr_i[i] = berr_save[i];
    }
    for( i = 0; i < 2*n; i++ ) {
        work_i[i] = work[i];
    }
    for( i = 0; i < n; i++ ) {
        rwork_i[i] = rwork[i];
    }

    /* Init row_major arrays */
    LAPACKE_cpp_trans( LAPACK_COL_MAJOR, uplo, n, ap_i, ap_r );
    LAPACKE_cpp_trans( LAPACK_COL_MAJOR, uplo, n, afp_i, afp_r );
    LAPACKE_cge_trans( LAPACK_COL_MAJOR, n, nrhs, b_i, ldb, b_r, nrhs+2 );
    LAPACKE_cge_trans( LAPACK_COL_MAJOR, n, nrhs, x_i, ldx, x_r, nrhs+2 );
    info_i = LAPACKE_cpprfs( LAPACK_ROW_MAJOR, uplo_i, n_i, nrhs_i, ap_r, afp_r,
                             b_r, ldb_r, x_r, ldx_r, ferr_i, berr_i );

    LAPACKE_cge_trans( LAPACK_ROW_MAJOR, n, nrhs, x_r, nrhs+2, x_i, ldx );

    failed = compare_cpprfs( x, x_i, ferr, ferr_i, berr, berr_i, info, info_i,
                             ldx, nrhs );
    if( failed == 0 ) {
        printf( "PASSED: row-major high-level interface to cpprfs\n" );
    } else {
        printf( "FAILED: row-major high-level interface to cpprfs\n" );
    }

    /* Release memory */
    if( ap != NULL ) {
        LAPACKE_free( ap );
    }
    if( ap_i != NULL ) {
        LAPACKE_free( ap_i );
    }
    if( ap_r != NULL ) {
        LAPACKE_free( ap_r );
    }
    if( afp != NULL ) {
        LAPACKE_free( afp );
    }
    if( afp_i != NULL ) {
        LAPACKE_free( afp_i );
    }
    if( afp_r != NULL ) {
        LAPACKE_free( afp_r );
    }
    if( b != NULL ) {
        LAPACKE_free( b );
    }
    if( b_i != NULL ) {
        LAPACKE_free( b_i );
    }
    if( b_r != NULL ) {
        LAPACKE_free( b_r );
    }
    if( x != NULL ) {
        LAPACKE_free( x );
    }
    if( x_i != NULL ) {
        LAPACKE_free( x_i );
    }
    if( x_r != NULL ) {
        LAPACKE_free( x_r );
    }
    if( x_save != NULL ) {
        LAPACKE_free( x_save );
    }
    if( ferr != NULL ) {
        LAPACKE_free( ferr );
    }
    if( ferr_i != NULL ) {
        LAPACKE_free( ferr_i );
    }
    if( ferr_save != NULL ) {
        LAPACKE_free( ferr_save );
    }
    if( berr != NULL ) {
        LAPACKE_free( berr );
    }
    if( berr_i != NULL ) {
        LAPACKE_free( berr_i );
    }
    if( berr_save != NULL ) {
        LAPACKE_free( berr_save );
    }
    if( work != NULL ) {
        LAPACKE_free( work );
    }
    if( work_i != NULL ) {
        LAPACKE_free( work_i );
    }
    if( rwork != NULL ) {
        LAPACKE_free( rwork );
    }
    if( rwork_i != NULL ) {
        LAPACKE_free( rwork_i );
    }

    return 0;
}

/* Auxiliary function: cpprfs scalar parameters initialization */
static void init_scalars_cpprfs( char *uplo, lapack_int *n, lapack_int *nrhs,
                                 lapack_int *ldb, lapack_int *ldx )
{
    *uplo = 'L';
    *n = 4;
    *nrhs = 2;
    *ldb = 8;
    *ldx = 8;

    return;
}

/* Auxiliary functions: cpprfs array parameters initialization */
static void init_ap( lapack_int size, lapack_complex_float *ap ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        ap[i] = lapack_make_complex_float( 0.0f, 0.0f );
    }
    ap[0] = lapack_make_complex_float( 3.230000019e+000, 0.000000000e+000 );
    ap[1] = lapack_make_complex_float( 1.509999990e+000, 1.919999957e+000 );
    ap[2] = lapack_make_complex_float( 1.899999976e+000, -8.399999738e-001 );
    ap[3] = lapack_make_complex_float( 4.199999869e-001, -2.500000000e+000 );
    ap[4] = lapack_make_complex_float( 3.579999924e+000, 0.000000000e+000 );
    ap[5] = lapack_make_complex_float( -2.300000042e-001, -1.110000014e+000 );
    ap[6] = lapack_make_complex_float( -1.179999948e+000, -1.370000005e+000 );
    ap[7] = lapack_make_complex_float( 4.090000153e+000, 0.000000000e+000 );
    ap[8] = lapack_make_complex_float( 2.329999924e+000, 1.400000006e-001 );
    ap[9] = lapack_make_complex_float( 4.289999962e+000, 0.000000000e+000 );
}
static void init_afp( lapack_int size, lapack_complex_float *afp ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        afp[i] = lapack_make_complex_float( 0.0f, 0.0f );
    }
    afp[0] = lapack_make_complex_float( 1.797220111e+000, 0.000000000e+000 );
    afp[1] = lapack_make_complex_float( 8.401864767e-001, 1.068316579e+000 );
    afp[2] = lapack_make_complex_float( 1.057188272e+000, -4.673885107e-001 );
    afp[3] = lapack_make_complex_float( 2.336942554e-001, -1.391037226e+000 );
    afp[4] = lapack_make_complex_float( 1.316353440e+000, 0.000000000e+000 );
    afp[5] = lapack_make_complex_float( -4.701749682e-001, 3.130658865e-001 );
    afp[6] = lapack_make_complex_float( 8.335255831e-002, 3.676066920e-002 );
    afp[7] = lapack_make_complex_float( 1.560392976e+000, 0.000000000e+000 );
    afp[8] = lapack_make_complex_float( 9.359616637e-001, 9.899691939e-001 );
    afp[9] = lapack_make_complex_float( 6.603332758e-001, 0.000000000e+000 );
}
static void init_b( lapack_int size, lapack_complex_float *b ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        b[i] = lapack_make_complex_float( 0.0f, 0.0f );
    }
    b[0] = lapack_make_complex_float( 3.930000067e+000, -6.139999866e+000 );
    b[8] = lapack_make_complex_float( 1.480000019e+000, 6.579999924e+000 );
    b[1] = lapack_make_complex_float( 6.170000076e+000, 9.420000076e+000 );
    b[9] = lapack_make_complex_float( 4.650000095e+000, -4.750000000e+000 );
    b[2] = lapack_make_complex_float( -7.170000076e+000, -2.182999992e+001 );
    b[10] = lapack_make_complex_float( -4.909999847e+000, 2.289999962e+000 );
    b[3] = lapack_make_complex_float( 1.990000010e+000, -1.438000011e+001 );
    b[11] = lapack_make_complex_float( 7.639999866e+000, -1.078999996e+001 );
}
static void init_x( lapack_int size, lapack_complex_float *x ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        x[i] = lapack_make_complex_float( 0.0f, 0.0f );
    }
    x[0] = lapack_make_complex_float( 9.999954104e-001, -9.999989867e-001 );
    x[8] = lapack_make_complex_float( -9.999988675e-001, 2.000002623e+000 );
    x[1] = lapack_make_complex_float( 1.403683768e-006, 3.000000954e+000 );
    x[9] = lapack_make_complex_float( 3.000000238e+000, -4.000000954e+000 );
    x[2] = lapack_make_complex_float( -3.999997139e+000, -4.999999523e+000 );
    x[10] = lapack_make_complex_float( -2.000000000e+000, 2.999997854e+000 );
    x[3] = lapack_make_complex_float( 1.999998331e+000, 9.999975562e-001 );
    x[11] = lapack_make_complex_float( 3.999998569e+000, -4.999998569e+000 );
}
static void init_ferr( lapack_int size, float *ferr ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        ferr[i] = 0;
    }
}
static void init_berr( lapack_int size, float *berr ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        berr[i] = 0;
    }
}
static void init_work( lapack_int size, lapack_complex_float *work ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        work[i] = lapack_make_complex_float( 0.0f, 0.0f );
    }
}
static void init_rwork( lapack_int size, float *rwork ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        rwork[i] = 0;
    }
}

/* Auxiliary function: C interface to cpprfs results check */
/* Return value: 0 - test is passed, non-zero - test is failed */
static int compare_cpprfs( lapack_complex_float *x, lapack_complex_float *x_i,
                           float *ferr, float *ferr_i, float *berr,
                           float *berr_i, lapack_int info, lapack_int info_i,
                           lapack_int ldx, lapack_int nrhs )
{
    lapack_int i;
    int failed = 0;
    for( i = 0; i < ldx*nrhs; i++ ) {
        failed += compare_complex_floats(x[i],x_i[i]);
    }
    for( i = 0; i < nrhs; i++ ) {
        failed += compare_floats(ferr[i],ferr_i[i]);
    }
    for( i = 0; i < nrhs; i++ ) {
        failed += compare_floats(berr[i],berr_i[i]);
    }
    failed += (info == info_i) ? 0 : 1;
    if( info != 0 || info_i != 0 ) {
        printf( "info=%d, info_i=%d\n",(int)info,(int)info_i );
    }

    return failed;
}

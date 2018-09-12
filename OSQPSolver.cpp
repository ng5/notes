#include "osqp.h"
#include "float.h"
#include <armadillo>
csc *transform(const arma::SpMat<c_float> &input) {
    return csc_matrix(input.n_rows,
                      input.n_cols,
                      input.n_nonzero,
                      const_cast<c_float *>(input.values),
                      reinterpret_cast<c_int *>(const_cast<arma::uword *>(input.row_indices)),
                      reinterpret_cast<c_int *>(const_cast<arma::uword *>(input.col_ptrs)));
}
int solve(const arma::SpMat<c_float> &P,
          const std::vector<c_float> q,
          const arma::SpMat<c_float> &A,
          const std::vector<c_float> &l,
          const std::vector<c_float> &u) {

    if (P.n_rows != P.n_cols) {
        std::cout << "Inconsistent input: P must be symmetrical matrix" << std::endl;
        return -1;
    } else if (P.n_rows != A.n_cols) {
        std::cout << "Inconsistent input: columns of P must match rows of A" << std::endl;
        return -1;
    }

    printf("Solver%14s\n", LINSYS_SOLVER_NAME[0]);
    c_int n = A.n_cols;
    c_int m = A.n_rows;

    // Problem settings
    OSQPSettings *settings = (OSQPSettings *)c_malloc(sizeof(OSQPSettings));

    // Structures
    OSQPWorkspace *work; // Workspace
    OSQPData *data;      // OSQPData

    // Populate data
    data = (OSQPData *)c_malloc(sizeof(OSQPData));
    data->n = n;
    data->m = m;
    data->P = transform(P);
    data->q = const_cast<c_float *>(q.data());
    data->A = transform(A);
    data->l = const_cast<c_float *>(l.data());
    data->u = const_cast<c_float *>(u.data());

    // Define Solver settings as default
    osqp_set_default_settings(settings);
    settings->verbose = false;
    settings->alpha = 1.0; // Change alpha parameter

    // Setup workspace
    work = osqp_setup(data, settings);

    // Solve Problem
    osqp_solve(work);
    if (work->info->status_val == OSQP_SOLVED) {
        printf("objective%14f\n", work->info->obj_val);
        for (size_t i = 0; i < n; ++i)
            printf("Solution%4zu%10f\n", i, *(work->solution->x + i));
    } else {
        printf("Unable to find Solution\n");
    }

    // Cleanup
    osqp_cleanup(work);
    c_free(data->A);
    c_free(data->P);
    c_free(data);
    c_free(settings);

    return 0;
}

int main(int argc, char **argv) {
    arma::Mat<c_float> P{{2, 0}, {0, 8}};
    arma::SpMat<c_float> sm_P{P};

    std::vector<c_float> q{0, -32};

    arma::Mat<c_float> A{{0, 1}, {-1, 2}, {1, 0}, {0, 1}};
    arma::SpMat<c_float> sm_A{A};

    std::vector<c_float> l{FLT_MIN, FLT_MIN, 0, 0};
    std::vector<c_float> u{7, 4, FLT_MAX, 4};

    solve(sm_P, q, sm_A, l, u);
    return 0;
}

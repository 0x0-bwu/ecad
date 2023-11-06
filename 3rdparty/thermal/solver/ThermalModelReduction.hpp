#pragma once

#include <vector>
#include <Eigen/Dense>
#include <Eigen/SparseQR>
#include <Eigen/SparseLU>


template<typename Float>
Eigen::Matrix<Float, Eigen::Dynamic, Eigen::Dynamic>
Prima(Eigen::SparseMatrix<Float> const& C,   // derivative conductance terms
      Eigen::SparseMatrix<Float> const& G,   // conductance
      Eigen::SparseMatrix<Float> const& B,   // input
      std::size_t q) {                       // desired state variables
  // assert preconditions
  assert(C.rows() == C.cols());     // input matrices are square
  assert(G.rows() == G.cols());
  assert(C.rows() == G.rows());     // input matrices are of the same size
  assert(B.rows() == G.rows());
  std::size_t N = B.cols();
  std::size_t state_count = static_cast<std::size_t>(C.rows());
  assert(N < state_count);          // must have more state variables than ports
  assert(q < state_count);          // desired state count must be less than current number

  // unchecked precondition: the state variables associated with the ports must be the last N

  using namespace Eigen;
  using namespace std;

  // Step 1 of PRIMA creates the B and L matrices, and is performed by the caller.

  // Step 2: Solve GR = B for R
  SparseLU<SparseMatrix<Float>, COLAMDOrdering<int> > G_LU(G);
  assert(G_LU.info() == Success);
  SparseMatrix<Float> R = G_LU.solve(B);

  // Step 3: Set X[0] to the orthonormal basis of R as determined by QR factorization
  typedef Matrix<Float, Dynamic, Dynamic> MatrixXX;
  // The various X matrices are stored in a std::vector.  Eigen requires us to use a special
  // allocator to retain alignment:
  typedef aligned_allocator<MatrixXX> AllocatorXX;
  typedef vector<MatrixXX, AllocatorXX> MatrixXXList;
  
  SparseQR<SparseMatrix<Float>, COLAMDOrdering<int> > R_QR(R);
  assert(R_QR.info() == Success);
  // QR stores the Q "matrix" as a series of Householder reflection operations
  // that it will perform for you with the * operator.  If you store it in a matrix
  // it obligingly produces an NxN matrix but if you want the "thin" result only,
  // creating a thin identity matrix and then applying the reflections saves both
  // memory and time:
  MatrixXXList X(1, R_QR.matrixQ() * MatrixXX::Identity(B.rows(), R_QR.rank()));

  // Step 4: Set n = floor(q/N)+1 if q/N is not an integer, and q/N otherwise
  size_t n = (q % N) ? (q/N + 1) : (q/N);

  // Step 5: Block Arnoldi (see Boley for detailed explanation)
  // In some texts this is called "band Arnoldi".
  // Boley and PRIMA paper use X with both subscripts and superscripts
  // to indicate the outer (subscript) and inner (superscript) loops
  // I have used X[] for the outer, Xk for the inner
  // X[k][j] value is just the value for the current inner loop, updated from the previous
  // so a single Xkj will suffice

  for (size_t k = 1; k < n; ++k)
  {
    // because X[] will vary in number of columns, so will Xk[]
    MatrixXX Xkj;             // X[k][j] - vector in PRIMA paper but values not reused

    // Prima paper says:
    // set V = C * X[k-1]
    // solve G*X[k][0] = V for X[k][0]

    Xkj = G_LU.solve(C*X[k-1]);            // Boley: "expand Krylov space"

    for (size_t j = 1; j <= k; ++j)        // "Modified Gram-Schmidt"
    {
      auto H = X[k-j].transpose() * Xkj;   // H[k-j][k-1] per Boley

      // X[k][j] = X[k][j-1] - X[k-j]*H
      Xkj = Xkj - X[k-j] * H;              // update X[k][j] from X[k][j-1]
    }

    // set X[k] to the orthonormal basis of X[k][k] via QR factorization
    // per Boley the "R" produced is H[k][k-1]
    if (Xkj.cols() == 1)
    {
      // a single column is automatically orthogonalized; just normalize
      X.push_back(Xkj.normalized());
    } else {
      auto xkkQR = Xkj.fullPivHouseholderQr();
      X.push_back(xkkQR.matrixQ() * MatrixXX::Identity(Xkj.rows(), xkkQR.rank()));
    }
  }

  // Step 6: Set Xfinal to the concatenation of X[0] to X[n-1],
  //         truncated to q columns
  size_t cols = accumulate(X.begin(), X.end(), 0,
                           [](size_t sum, MatrixXX const& m) { return sum + m.cols(); });
  cols = std::min(q, cols);  // truncate to q

  MatrixXX Xfinal(state_count, cols);
  size_t col = 0;
  for (size_t k = 0; (k <= n) && (col < cols); ++k)
  {
    // copy columns from X[k] to Xfinal
    for (int j = 0; (j < X[k].cols()) && (col < cols); ++j)
    {
      Xfinal.col(col++) = X[k].col(j);
    }
  }

  return Xfinal;
}
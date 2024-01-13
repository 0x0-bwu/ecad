template<typename Float>
inline DenseMatrix<Float> // fork MOR implementation from jefftrull
Prima(const SparseMatrix<Float> & C,   // derivative conductance terms
      const SparseMatrix<Float> & G,   // conductance
      const SparseMatrix<Float> & B, 
      [[maybe_unused]] const SparseMatrix<Float> & L,   // output
      size_t q)// desired state variables
{   
	/* !Caution: G has to be positive definite in order to use SimplicialLLT solver.
	 * If it isn't, one option is to use FullPivLU */

	Eigen::SimplicialLLT<SparseMatrix<Float>> solver;
	solver.compute(G);

	int N = B.cols();
	int n = G.cols();
	int r = q * N; // the order of the reduced system

	/* G*R = B */
	DenseMatrix<Float> R(n,N);
	
	/* a routine that returns the i-th col of sparse matrix sm in dense format */
	auto spMatCol = [](const SparseMatrix<Float> & sm, int col)
	{
		DenseVector<Float> x = DenseVector<Float>::Zero(sm.rows(), 1);
		/* loop over the rows of the i-th column of sm */
		for (typename SparseMatrix<Float>::InnerIterator it(sm, col); it; ++it)
			x(it.row()) = it.value();
		return x;	
	};

	#pragma omp parallel for schedule(dynamic)
	for( int j = 0; j < N; ++j)
		R.col(j) = solver.solve(spMatCol(B,j));

	std::cout << "The matrix R is of size " << R.rows() << "x" << R.cols() << std::endl;

	/* the projection matrix */
	DenseMatrix<Float> X(n,r);
	DenseMatrix<Float> Q;

	/* numerically unstable but fast.
	 * When stability is an issue run FullPivHouseholderQR */
	Eigen::HouseholderQR<DenseMatrix<Float>> qr(R); // try maybe inplace decomposition (?)
	Q = qr.householderQ();

	std::cout << "The matrix Q is of size " << Q.rows() << "x" << Q.cols() << std::endl;

	X.topLeftCorner(n,N) = Q.topLeftCorner(n,N);

	/* loop until you reach the number of moments */
	for( int q_i=1; q_i<q; ++q_i ) // k = 1, 2, ..., q
	{
		DenseMatrix<Float> V;
		V = C * X.block( 0, (q_i-1)*N, n, N ); // V = C*X(k-1)

		/* Xk = G^(-1)*V */
		#pragma omp parallel for schedule(dynamic)
		for( int j=0; j<N; j++ )
			X.col( q_i*N+j ) = solver.solve( V.col(j) );

		for( int q_j=0; q_j<q_i; q_j++ ) // j = 1, 2, ..., k
		{
			DenseMatrix<Float> H = X.block(0, (q_i-q_j-1)*N, n, N).transpose();
			H = H * X.block(0, q_i*N, n, N); // H = X(k-j)'*Xk
			X.block( 0, q_i*N, n, N) = X.block( 0, q_i*N, n, N) - X.block(0, (q_i-q_j-1)*N, n, N) * H; // Xk = Xk-X(k-j)*H
		}

		Eigen::HouseholderQR<DenseMatrix<Float>> qr( X.block( 0, q_i*N, n, N) );
		Q = qr.householderQ();

		X.block( 0, q_i*N, n, N) = Q.topLeftCorner(n,N);
	}

	return X;
}
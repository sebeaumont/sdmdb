#pragma once

//#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/SparseCore>

///////////////////////////////////////////////////////////////
// types for sparse co-occurrence matrix and similarity scores
//  etc.

//typedef double scalar_t; xxx eigen3 bug prevents use of double!
typedef double                                     scalar_t;
typedef Eigen::SparseMatrix<scalar_t>              sparse_matrix;
typedef Eigen::SparseVector<scalar_t>              sparse_vector;
typedef Eigen::Triplet<scalar_t>                   triplet;
typedef std::vector<triplet>                       triplet_vec;
typedef Eigen::Matrix<scalar_t, Eigen::Dynamic, 1> vector;
//typedef Eigen::VectorXf                vector;

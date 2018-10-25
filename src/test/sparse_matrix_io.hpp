// eigen3 sparse matrix stream serialisation templates

#pragma once
#include <iostream>
#include <vector>
#include <Eigen/Core>
#include <Eigen/Sparse>

// TODO template a struct to represent the header?

/// template function to serialize Eigen::SparseMatrix<...>
/// takes an optional std::ostream to write matrix representation

template <typename ScalarT, int options, typename IndexT>
bool serialize_matrix(Eigen::SparseMatrix<ScalarT, options, IndexT>& m,
                      std::ostream& outs=std::cout) {
  
  if (outs.good()) {
    
    IndexT rows, cols, nnzs, outS, innS;
    m.makeCompressed();    

    // copy input matrix (rvalues) onto stack 
    rows = m.rows();
    cols = m.cols();
    nnzs = m.nonZeros();
    outS = m.outerSize();
    innS = m.innerSize();

    // write header
    outs.write((const char *)&(rows), sizeof(IndexT));
    outs.write((const char *)&(cols), sizeof(IndexT));
    outs.write((const char *)&(nnzs), sizeof(IndexT));
    outs.write((const char *)&(outS), sizeof(IndexT));
    outs.write((const char *)&(innS), sizeof(IndexT));

    // write data and indexes
    outs.write((const char *)(m.valuePtr()), sizeof(ScalarT) * m.nonZeros());
    outs.write((const char *)(m.outerIndexPtr()), sizeof(IndexT) * m.outerSize());
    outs.write((const char *)(m.innerIndexPtr()), sizeof(IndexT) * m.nonZeros());

    return true;
  } else return false; // check your ostream!
}


// template function to deserialize Eigen::SparseMatrix<...>
template <typename ScalarT, int options, typename IndexT>
bool deserialize_matrix(Eigen::SparseMatrix<ScalarT, options, IndexT>& m,
                        std::istream& ins=std::cin) {
  
  if (ins.good()) {

    // header vars
    
    IndexT rows, cols, nnz, inSz, outSz;

    // read header

    ins.read((char*)&rows, sizeof(IndexT));  // rows
    ins.read((char*)&cols, sizeof(IndexT));  // cols
    ins.read((char*)&nnz, sizeof(IndexT));   // number of non zero entries
    ins.read((char*)&inSz, sizeof(IndexT));  // inner index size
    ins.read((char*)&outSz, sizeof(IndexT)); // outer index size

    // size up m

    m.resize(rows, cols);
    m.makeCompressed();
    m.resizeNonZeros(nnz);

    // read variable data
    
    ins.read((char*)(m.valuePtr()), sizeof(ScalarT) * nnz);       // element data
    ins.read((char*)(m.outerIndexPtr()), sizeof(IndexT) * outSz); // outer indexes
    ins.read((char*)(m.innerIndexPtr()), sizeof(IndexT) * nnz);   // innner indexes

    // finalise matrix
    
    m.finalize();
    return true;
    
  } else return false; // check your istream!
}


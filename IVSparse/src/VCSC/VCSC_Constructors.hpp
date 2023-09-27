/**
 * @file VCSC_Constructors.hpp
 * @author Skyler Ruiter and Seth Wolfgang
 * @brief Constructors for VCSC Sparse Matrices
 * @version 0.1
 * @date 2023-07-03
 */

#pragma once

namespace IVSparse {

// Destructor
template <typename T, typename indexT, bool columnMajor>
SparseMatrix<T, indexT, 2, columnMajor>::~SparseMatrix() {
  // delete the meta data
  if (metadata != nullptr) {
    delete[] metadata;
  }
}

    // Eigen Constructor
    template <typename T, typename indexT, bool columnMajor>
    SparseMatrix<T, indexT, 2, columnMajor>::SparseMatrix(Eigen::SparseMatrix<T>& mat) {
        // make sure the matrix is compressed
        mat.makeCompressed();

        // get the number of rows and columns
        numRows = mat.rows();
        numCols = mat.cols();

        outerDim = columnMajor ? numCols : numRows;
        innerDim = columnMajor ? numRows : numCols;

        // get the number of non-zero elements
        nnz = mat.nonZeros();

        // call the compression function
        compressCSC(mat.valuePtr(), mat.innerIndexPtr(), mat.outerIndexPtr());
    }

    // Eigen Row Major Constructor
    template <typename T, typename indexT, bool columnMajor>
    SparseMatrix<T, indexT, 2, columnMajor>::SparseMatrix(Eigen::SparseMatrix<T, Eigen::RowMajor>& mat) {
        // make sure the matrix is compressed
        mat.makeCompressed();

        // get the number of rows and columns
        numRows = mat.rows();
        numCols = mat.cols();

        outerDim = numRows;
        innerDim = numCols;

        // get the number of non-zero elements
        nnz = mat.nonZeros();

        // call the compression function
        compressCSC(mat.valuePtr(), mat.innerIndexPtr(), mat.outerIndexPtr());
    }

    // Deep Copy Constructor
    template <typename T, typename indexT, bool columnMajor>
    SparseMatrix<T, indexT, 2, columnMajor>::SparseMatrix(const IVSparse::SparseMatrix<T, indexT, 2, columnMajor>& other) {
        *this = other;
    }

    // Conversion Constructor
    template <typename T, typename indexT, bool columnMajor>
    template <uint8_t otherCompressionLevel>
    SparseMatrix<T, indexT, 2, columnMajor>::SparseMatrix(IVSparse::SparseMatrix<T, indexT, otherCompressionLevel, columnMajor>& other) {
        // if already the right compression level
        if constexpr (otherCompressionLevel == 2) {
            *this = other;
            return;
        }

        // make a temporary matrix of the correct compression level
        IVSparse::SparseMatrix<T, indexT, 2, columnMajor> temp;

        // convert other to the right compression level
        if constexpr (otherCompressionLevel == 1) {
            temp = other.toVCSC();
        }
        else if constexpr (otherCompressionLevel == 3) {
            temp = other.toVCSC();
        }

        // other should be the same compression level as this now
        *this = temp;

        // run the user checks and calculate the compression size
        calculateCompSize();

        #ifdef IVSPARSE_DEBUG
        userChecks();
        #endif
    }

    // Raw CSC Constructor
    template <typename T, typename indexT, bool columnMajor>
    template <typename T2, typename indexT2>
    SparseMatrix<T, indexT, 2, columnMajor>::SparseMatrix(
        T2* vals, indexT2* innerIndices, indexT2* outerPtr, uint32_t num_rows, uint32_t num_cols, uint32_t nnz) {

        #ifdef IVSPARSE_DEBUG
        assert(num_rows > 0 && num_cols > 0 && nnz > 0 &&
               "Error: Matrix dimensions must be greater than 0");
        assert(innerIndices != nullptr && outerPtr != nullptr && vals != nullptr &&
               "Error: Pointers cannot be null");
        #endif


        // set the dimensions
        if (columnMajor) {
            innerDim = num_rows;
            outerDim = num_cols;
        }
        else {
            innerDim = num_cols;
            outerDim = num_rows;
        }
        numRows = num_rows;
        numCols = num_cols;
        this->nnz = nnz;

        // call the compression function
        compressCSC(vals, innerIndices, outerPtr);
    }

    // COO Constructor
    template <typename T, typename indexT, bool columnMajor>
    template <typename T2, typename indexT2>
    SparseMatrix<T, indexT, 2, columnMajor>::SparseMatrix(
        std::vector<std::tuple<indexT2, indexT2, T2>>& entries, uint64_t num_rows, uint32_t num_cols, uint32_t nnz) {

        #ifdef IVSPARSE_DEBUG
        assert(num_rows > 0 && num_cols > 0 && nnz > 0 &&
               "Error: Matrix dimensions must be greater than 0");
        #endif

        // see if the matrix is empty
        if (nnz == 0) {
            *this = SparseMatrix<T, indexT, 2, columnMajor>();
        }

        // set the dimensions
        if (columnMajor) {
            innerDim = num_rows;
            outerDim = num_cols;
        }
        else {
            innerDim = num_cols;
            outerDim = num_rows;
        }

        numRows = num_rows;
        numCols = num_cols;
        this->nnz = nnz;
        encodeValueType();
        index_t = sizeof(indexT);

        metadata = new uint32_t[NUM_META_DATA];
        metadata[0] = 2;
        metadata[1] = innerDim;
        metadata[2] = outerDim;
        metadata[3] = nnz;
        metadata[4] = val_t;
        metadata[5] = index_t;


        // sort the tuples by first by column then by row
        std::sort(entries.begin(), entries.end(),
                  [](const std::tuple<indexT2, indexT2, T2>& a,
                     const std::tuple<indexT2, indexT2, T2>& b) {
                         if (std::get<1>(a) == std::get<1>(b)) {
                             return std::get<0>(a) < std::get<0>(b);
                         }
                         else {
                             return std::get<1>(a) < std::get<1>(b);
                         }
                  });


  data.reserve(outerDim);

        // loop through the tuples
        for (size_t i = 0; i < nnz; i++) {
            // get the column
            indexT2 row = std::get<0>(entries[i]);
            indexT2 col = std::get<1>(entries[i]);
            T2 val = std::get<2>(entries[i]);

    // check if the value is already in the map
    if (data[col].find(val) != data[col].end()) {
      // value found positive delta encode it
      data[col][val].push_back(row);
    } else {
      // value not found
      data[col][val] = std::vector<indexT2>{row};
    }

  }  // end of loop through tuples

        // run the user checks and calculate the compression size
        calculateCompSize();

        #ifdef IVSPARSE_DEBUG
        userChecks();
        #endif
    }

    // IVSparse Vector Constructor
    template <typename T, typename indexT, bool columnMajor>
    SparseMatrix<T, indexT, 2, columnMajor>::SparseMatrix(
        typename IVSparse::SparseMatrix<T, indexT, 2, columnMajor>::Vector& vec) {

        // Get the dimensions and metadata
        if (columnMajor) {
            numRows = vec.getLength();
            numCols = 1;
            innerDim = numRows;
            outerDim = numCols;
        }
        else {
            numRows = 1;
            numCols = vec.getLength();
            innerDim = numCols;
            outerDim = numRows;
        }
        nnz = vec.nonZeros();
        encodeValueType();
        index_t = sizeof(indexT);

  metadata = new uint32_t[NUM_META_DATA];
  metadata[0] = 2;
  metadata[1] = innerDim;
  metadata[2] = outerDim;
  metadata[3] = nnz;
  metadata[4] = val_t;
  metadata[5] = index_t;

  // check if the vector is empty
  if (vec.byteSize() == 0) [[unlikely]] {
    return;
  }

  data.reserve(1);

  // copy the vector map to the matrix
  data[0] = vec.data;

  // run the user checks and calculate the compression size
  calculateCompSize();

  #ifdef IVSPARSE_DEBUG
  userChecks();
  #endif
}  // end of IVSparse Vector Constructor

    // Array of Vectors Constructor
    template <typename T, typename indexT, bool columnMajor>
    SparseMatrix<T, indexT, 2, columnMajor>::SparseMatrix(
        std::vector<typename IVSparse::SparseMatrix<T, indexT, 2, columnMajor>::Vector>& vecs) {

        // Construct a one vector matrix to append to
        IVSparse::SparseMatrix<T, indexT, 2, columnMajor> temp(vecs[0]);

        // append the rest of the vectors
        for (size_t i = 1; i < vecs.size(); i++) {
            temp.append(vecs[i]);
        }

        // copy the temp matrix to this
        *this = temp;

        // run the user checks and calculate the compression size
        calculateCompSize();

        #ifdef IVSPARSE_DEBUG
        userChecks();
        #endif
    }

    // File Constructor
    template <typename T, typename indexT, bool columnMajor>
    SparseMatrix<T, indexT, 2, columnMajor>::SparseMatrix(const char* filename) {
        // open the file
        FILE* fp = fopen(filename, "rb");

        #ifdef IVSPARSE_DEBUG
        // check if the file was opened
        if (fp == nullptr) {
            throw std::runtime_error("Error: Could not open file");
        }
        #endif

        // read the metadata
        metadata = new uint32_t[NUM_META_DATA];
        fread(metadata, sizeof(uint32_t), NUM_META_DATA, fp);

        // set the matrix info
        innerDim = metadata[1];
        outerDim = metadata[2];
        nnz = metadata[3];
        val_t = metadata[4];
        index_t = metadata[5];
        numRows = columnMajor ? innerDim : outerDim;
        numCols = columnMajor ? outerDim : innerDim;

        #ifdef IVSPARSE_DEBUG
        // if the compression level of the file is different than the compression
        // level of the class
        if (metadata[0] != 2) {
            // throw an error
            throw std::runtime_error(
                "Error: Compression level of file does not match compression level of "
                "class");
        }
        #endif

  data.reserve(outerDim);

  // loop through the columns
  for (uint32_t i = 0; i < outerDim; i++) {

    // read in the number of unique values for the column
    indexT numUnique;

    fread(&numUnique, sizeof(indexT), 1, fp);

    // read in the unique values into the map
    for (indexT j = 0; j < numUnique; j++) {

      // read the value
      T val; 
      fread(&val, sizeof(T), 1, fp);

      // add the value to the map
      data[i][val] = std::vector<indexT>();
    }

    // read in the counts for the column
    indexT counts[numUnique];

    fread(&counts, sizeof(indexT), numUnique, fp);

    // read in the indices for the column
    for (indexT j = 0; j < numUnique; j++) {

      // get the number of indices
      indexT numIndices = counts[j];

      // read in the indices
      indexT indices[numIndices];
      fread(indices, sizeof(indexT), numIndices, fp);

      // add the indices to the map using reserve and memcpy
      data[i][j].reserve(numIndices);
      memcpy(data[i][j].data(), indices, sizeof(indexT) * numIndices);

    }

  }

        // close the file
        fclose(fp);

        // calculate the compresssion size
        calculateCompSize();

        // run the user checks
        #ifdef IVSPARSE_DEBUG
        userChecks();
        #endif
    }  // end of File Constructor

    //* Private Constructors *//

// Private Tranpose Constructor
template <typename T, typename indexT, bool columnMajor>
SparseMatrix<T, indexT, 2, columnMajor>::SparseMatrix(
    std::map<T, std::vector<indexT>> maps[], uint32_t num_rows, uint32_t num_cols) {
  
  // set class variables
  if constexpr (columnMajor) {
    innerDim = num_cols;
    outerDim = num_rows;
  } else {
    innerDim = num_rows;
    outerDim = num_cols;
  }

        numRows = num_cols;
        numCols = num_rows;
        encodeValueType();
        index_t = sizeof(indexT);

  data = maps;

        // set the metadata
        metadata = new uint32_t[NUM_META_DATA];
        metadata[0] = 2;
        metadata[1] = innerDim;
        metadata[2] = outerDim;
        metadata[3] = nnz;
        metadata[4] = val_t;
        metadata[5] = index_t;

        // run the user checks and calculate the compression size
        calculateCompSize();

        #ifdef IVSPARSE_DEBUG
        userChecks();
        #endif
    }  // end of Private Tranpose Constructor

}  // namespace IVSparse
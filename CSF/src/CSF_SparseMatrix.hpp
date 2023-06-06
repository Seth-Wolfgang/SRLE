/*  @file CSF_SparseMatrix.hpp
    @brief The header file for all CSF Sparse Matrix Definitions

    [put long description here]

    @author Skyler Ruiter & Seth Wolfgang
*/

#pragma once

namespace CSF {

    /**
     * @brief A class to represent a sparse matrix compressed in the CSF format
     *
     * @details CSF Matrix is a read-only sparse matrix class optimized for sparse-dense computation in cases where non-zero values are highly redundant. For such cases, sparse fiber storage can reduce memory footprint by up to 50% compared to standard sparse compression. CSF also increases the ability to further compress index arrays within each fiber. This default templated version is for compression levels 2 and 3 specifically. For compression level 1 there is a template specialization.
     *
     * @tparam T The data type of the values in the matrix
     * @tparam indexT The data type of the indices in the matrix
     * @tparam compressionLevel The compression level used
     * @tparam columnMajor Whether the matrix is stored in column major format
     */
    template <typename T, typename indexT = uint64_t, uint8_t compressionLevel = 3, bool columnMajor = true>
    class SparseMatrix {
    private:
        //* Private Class Variables *//

        const uint8_t delim = DELIM;

        uint32_t innerDim = 0;
        uint32_t outerDim = 0;
        uint32_t numRows = 0;
        uint32_t numCols = 0;
        uint32_t nnz = 0;

        uint32_t val_t;
        uint32_t index_t;

        size_t compSize = 0;

        // Class Data //

        void** data;
        void** endPointers;
        uint32_t* metadata;

        // Benchmarking Data //

        //! j Vector HERE

        //* Private Class Methods *//

        template <typename T2, typename indexT2>
        void compress(T2* vals, indexT2* innerIndices, indexT2* outerPtr);

        uint8_t byteWidth(size_t size);

        uint32_t encodeVal();

        void checkVal();

        void userChecks();

    public:
        //* Nested Subclasses *//

        class Vector;

        class InnerIterator;

        //* Constructors & Destructor *//

        /** @name Constructors
        */
        ///@{

        /**
         * @brief Eigen Sparse Matrix Constructor
         * 
         * @param mat The Eigen Sparse Matrix to be compressed
         */
        SparseMatrix(Eigen::SparseMatrix<T>& mat);

        /**
         * @brief Eigen Sparse Matrix Constructor (Row Major)
         *
         * @param mat The Eigen Sparse Matrix to be compressed
         */
        SparseMatrix(Eigen::SparseMatrix<T, Eigen::RowMajor>& mat);

        /**
         * @brief Deep Copy Constructor
         * 
         * @param mat The CSF Matrix to be copied
         */
        SparseMatrix(const CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>& mat);

        /**
         * @brief Raw CSC Constructor
         * 
         * @tparam T2 The value type of the CSC matrix
         * @tparam indexT2 The index type of the CSC matrix
         * @param vals The values of the CSC matrix
         * @param innerIndices The inner indices of the CSC matrix
         * @param outerPtr The outer pointers of the CSC matrix
         * @param num_rows The number of rows
         * @param num_cols The number of columns
         * @param nnz The number of non-zero values
         */
        template <typename T2, typename indexT2>
        SparseMatrix(T2* vals, indexT2* innerIndices, indexT2* outerPtr, uint32_t num_rows, uint32_t num_cols, uint32_t nnz);


        /**
         * @brief Array of CSF Vectors Constructor
         * 
         * @param vec The vector array to construct the matrix from
         * @param size The numer of vectors in the array
         */
        SparseMatrix(typename CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector vec[], size_t size);

        /**
         * @brief CSF Vector Constructor
         * 
         * @param vec The vector to construct the matrix from
         */
        SparseMatrix(typename CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector& vec);

        /**
         * @brief File Constructor
         * 
         * 
         * @param filename The filepath of the matrix to be read in
         */
        SparseMatrix(const char *filename);

        ///@}

        SparseMatrix();

        // Map constructor for use in transposing
        SparseMatrix(std::map<indexT, std::unordered_map<T, std::vector<indexT>>> &map, uint32_t num_rows, uint32_t num_cols);

        // destructor
        ~SparseMatrix();

        //* Utility Methods *//

        /**
         * @name Utility Methods
        */
        ///@{

        /**
         * @brief Write the CSF matrix to a file
         * 
         * @param filename The filename of the matrix to write to
         */
        void write(const char* filename);

        /**
         * @brief Print the CSF matrix to the console
         * 
         */
        void print();

        ///@}

        //* Getters *//

        /** @name Getters
         */
        ///@{

        /**
         * @brief Get the value at the specified row and column
         * 
         * @param row The row of the value to get
         * @param col The column of the value to get
         * @return T The value at the specified row and column
         */
        T coeff(uint32_t row, uint32_t col) const;

        /**
         * @brief Get a pointer to a specific vector in the CSF matrix
         * 
         * @param vec The vector to get the pointer to
         * @return void* The pointer to the vector
         */
        void* getVecPointer(uint32_t vec) const;

        /**
         * @brief Get a vector copy from the CSF matrix
         * 
         * @param vec The vector to get a copy of
         * @return typename CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector The vector copy returned
         */
        typename CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector getVector(uint32_t vec);

        /**
         * @brief Get the size of a vector in the CSF matrix
         * 
         * @param vec The vector to get the size of
         * @return size_t The size of the vector
         */
        size_t getVecSize(uint32_t vec) const;

        /**
         * @brief Get the size of the inner dimension of the CSF matrix
         * 
         * @return uint32_t The size of the inner dimension
         */
        uint32_t innerSize() const;

        /**
         * @brief Get the size of the outer dimension of the CSF matrix
         * 
         * @return uint32_t The size of the outer dimension
         */
        uint32_t outerSize() const;

        /**
         * @brief Get the number of rows in the CSF matrix
         * 
         * @return uint32_t The number of rows
         */
        uint32_t rows() const;

        /**
         * @brief Get the number of columns in the CSF matrix
         * 
         * @return uint32_t The number of columns
         */
        uint32_t cols() const;

        /**
         * @brief Get the number of non-zero values in the CSF matrix
         * 
         * @return uint32_t The number of non-zero values
         */
        uint32_t nonZeros() const;

        /**
         * @brief Get the size of the CSF matrix in bytes
         * 
         * @return size_t The size of the CSF matrix in bytes
         */
        size_t compressionSize() const;

        ///@}

        /** @name Matrix Manipulation Methods
        */
        ///@{

        //* Conversion Methods *//

        /**
         * @brief Convert the CSF matrix to a CSC format
         * 
         * @return CSF::SparseMatrix<T, indexT, 1, columnMajor> The CSC matrix returned
         */
        CSF::SparseMatrix<T, indexT, 1, columnMajor> toCSF1();

        /**
         * @brief Convert the CSF matrix to an Eigen Sparse Matrix
         * 
         * @return Eigen::SparseMatrix<T, columnMajor ? Eigen::ColMajor : Eigen::RowMajor> The Eigen Sparse Matrix returned
         */
        Eigen::SparseMatrix<T, columnMajor ? Eigen::ColMajor : Eigen::RowMajor> toEigen();

        /**
         * @brief Transposes the CSF matrix
         * 
         * @return CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor> The transposed CSF matrix
         */
        CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor> transpose();

        /**
         * @brief Append a CSF vector to the outer dimension of the CSF matrix
         * 
         * @param vec The vector to be appended
         */
        void append(typename SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector& vec);

        /**
         * @brief Get a slice of a CSF matrix as an array of vectors
         * 
         * @param start The starting index of the slice
         * @param end The ending index of the slice
         * @return CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector* The array of vectors returned
         */
        typename CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector* slice(uint32_t start, uint32_t end);

        ///@}

        //* Operator Overloads *//

        bool operator==(const SparseMatrix<T, indexT, compressionLevel, columnMajor>& other);

        bool operator!=(const SparseMatrix<T, indexT, compressionLevel, columnMajor>& other);

        CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor> operator*(T scalar) const;

        void operator*=(T scalar);

        //This method does not seem to work unless it is in this file
        friend std::ostream& operator<<(std::ostream& os, CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>& mat) {
            // #ifndef CSF_DEBUG
            // if (mat.cols() > 110) {
            //     std::cout << "CSF matrix is too large to print" << std::endl;
            //     return os;
            // }
            // #endif

            // create a matrix to store the full matrix representation of the CSF matrix
            T** matrix = new T * [mat.rows()];
            for (size_t i = 0; i < mat.rows(); i++) {
                matrix[i] = new T[mat.cols()];
                memset(matrix[i], 0, sizeof(matrix[i]) * mat.cols());
            }

            // Build the full matrix representation of the the CSF matrix
            for (size_t i = 0; i < mat.cols(); i++) {
                for (typename CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>::InnerIterator it(mat, i); it; ++it) {
                    // std::cout << "it.row(): " << it.row() << " col: " << it.col() << " value: " << it.value() << std::endl;
                    matrix[it.row()][it.col()] = it.value();
                }
            }


            // std::cout << "rows: " << mat.rows() << std::endl;
            // std::cout << "cols: " << mat.cols() << std::endl;

            // store all of matrix into the output stream
            for (size_t i = 0; i < mat.rows(); i++) {
                for (size_t j = 0; j < mat.cols(); j++) {
                    os << matrix[i][j] << " ";
                }
                os << std::endl;
            }

            return os;
        }

        CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>& operator=(const CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>& other);

        Eigen::VectorXd operator*(typename SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector& vec);

        Eigen::VectorXd operator*(Eigen::VectorXd& vec);

        Eigen::Matrix<T, -1, -1> operator*(Eigen::Matrix<T, -1, -1> mat);

        T operator()(uint32_t row, uint32_t col);

        typename CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector operator[](uint32_t vec);

        inline Eigen::Matrix<T, -1, -1> vectorMultiply(Eigen::VectorXd& vec);

        inline Eigen::Matrix<T, -1, 1> vectorMultiply(typename SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector& vec);

        inline Eigen::Matrix<T, -1, -1> matrixMultiply(Eigen::Matrix<T, -1, -1>& mat);

        //WIP
        inline Eigen::Matrix<T, -1, -1> matrixMultiply2(Eigen::Matrix<T, -1, -1>& mat);
    };


    /**
     * @brief Compression Level 1 CSF Matrix (CSC Format)
     * 
     * @tparam T The data type of the values
     * @tparam indexT The data type of the indices
     * @tparam columnMajor If the matrix is in column major format
     */
    template <typename T, typename indexT, bool columnMajor>
    class SparseMatrix<T, indexT, 1, columnMajor> {
    private:
        //* Private Class Variables *//

        const uint8_t delim = DELIM;

        uint32_t innerDim = 0;
        uint32_t outerDim = 0;
        uint32_t numRows = 0;
        uint32_t numCols = 0;
        uint32_t nnz = 0;

        uint32_t val_t;
        uint32_t index_t;

        size_t compSize = 0;

        T* vals = nullptr;
        indexT* innerIdx = nullptr;
        indexT* outerPtr = nullptr;

        uint32_t* metadata = nullptr;

        //* Private Class Methods *//

        uint8_t byteWidth(size_t size);

        uint32_t encodeVal();

        void checkVal();

        void userChecks();

    public:
        //* Nested Subclasses *//

        class Vector;

        class InnerIterator;

        //* Constructors & Destructor *//

        // default empty constructor
        SparseMatrix();

        // eigen sparse matrix constructor
        SparseMatrix(Eigen::SparseMatrix<T>& mat);

        // eigen sparse matrix constructor (row major)
        SparseMatrix(Eigen::SparseMatrix<T, Eigen::RowMajor>& mat);

        // deep copy constructor
        SparseMatrix(CSF::SparseMatrix<T, indexT, 1, columnMajor>& mat);

        // file constructor
        SparseMatrix(const char* filename);

        // COO constructor
        //! Decleration Here

        // destructor
        ~SparseMatrix();

        //* Utility Methods *//

        void write(const char* filename);

        void print();

        //* Getters *//

        T coeff(uint32_t row, uint32_t col);

        uint32_t innerSize();

        uint32_t outerSize();

        uint32_t rows();

        uint32_t cols();

        uint32_t nonZeros();

        size_t compressionSize();

        T* values();

        indexT* innerIdxPtr();

        indexT* outerPtrs();

        //* Conversion Methods *//

        Eigen::SparseMatrix<T> toEigen();

        CSF::SparseMatrix<T, indexT, 2, columnMajor> toCSF2();

        CSF::SparseMatrix<T, indexT, 3, columnMajor> toCSF3();

        CSF::SparseMatrix<T, indexT, 1, columnMajor> transpose();

        void append(typename SparseMatrix<T, indexT, 1, columnMajor>::Vector& vec);

        //* Operator Overloads *//

        bool operator==(const SparseMatrix<T, indexT, 1, columnMajor>& other);

        bool operator!=(const SparseMatrix<T, indexT, 1, columnMajor>& other);

        T operator()(uint32_t row, uint32_t col);
    };

    /**
     * @brief An iterator over the inner dimension of a CSF matrix
     * 
     * @tparam T 
     * @tparam indexT 
     * @tparam compressionLevel 
     * @tparam columnMajor 
     */
    template <typename T, typename indexT, uint8_t compressionLevel, bool columnMajor>
    class SparseMatrix<T, indexT, compressionLevel, columnMajor>::InnerIterator {
    private:
        //* Private Class Variables *//

        indexT outer;
        indexT index;
        T* val;

        indexT newIndex;

        uint8_t indexWidth = 1;

        void* data;
        void* endPtr;

        bool firstIndex = true;

        //* Private Class Methods *//

        void __attribute__((hot)) decodeIndex();

        void userChecks();


    public:
        //* Constructors & Destructor *//

        InnerIterator();

        /**
         * @name Constructors
        */
        ///@{

        /**
         * @brief Construct a new Inner Iterator object for a CSF matrix column
         * 
         * @param mat The CSF matrix to iterate over
         * @param col The column to iterate over
         */
        InnerIterator(SparseMatrix<T, indexT, compressionLevel, columnMajor>& mat, uint32_t col);

        /**
         * @brief Construct a new Inner Iterator object for a CSF matrix vector
         * 
         * @param vec The CSF vector to iterate over
         */
        InnerIterator(SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector& vec);

        ///@}

        //* Operator Overloads *//

        void __attribute__((hot))  operator++();

        bool operator==(const InnerIterator& other);

        bool operator!=(const InnerIterator& other);

        bool operator<(const InnerIterator& other);

        bool operator>(const InnerIterator& other);

        inline __attribute__((hot)) operator bool() { return (char*)endPtr - indexWidth > data; };

        T& operator*();

        //* Getters *//

        /**
         * @brief Get the index of the current value
         * 
         * @return indexT The index of the current value
         */
        indexT getIndex();

        /**
         * @brief Get the outer dimension of the CSF matrix
         * 
         * @return indexT The outer dimension of the CSF matrix
         */
        indexT outerDim();

        /**
         * @brief Get the row of the current value
         * 
         * @return indexT The row of the current value
         */
        indexT row();

        /**
         * @brief Get the column of the current value
         * 
         * @return indexT The column of the current value
         */
        indexT col();

        /**
         * @brief Get the current value
         * 
         * @return T The current value
         */
        T value();

        /**
         * @brief Sets the current value to newValue
         * 
         * @param newValue The new value to set the current value to
         */
        void coeff(T newValue);

        //* Utility Methods *//

        /**
         * @brief Checks if the iterator is at the start of a new CSF run
         * 
         * @return bool Whether the iterator is at the start of a new CSF run
         */
        bool isNewRun();

    };


    /**
     * @brief An iterator over the inner dimension of a CSF matrix (Compression Level 1)
     * 
     * @tparam T The data type of the values
     * @tparam indexT The data type of the indices
     * @tparam columnMajor If the matrix is in column major format
     */
    template <typename T, typename indexT, bool columnMajor>
    class SparseMatrix<T, indexT, 1, columnMajor>::InnerIterator {
        private:

        //* Private Class Variables *//

        T* val;
        indexT index;
        indexT outer;

        //* Private Class Methods *//

        void userChecks();


        public:

        //* Constructors & Destructor *//

        InnerIterator();

        InnerIterator(SparseMatrix<T, indexT, 1, columnMajor>& mat, uint32_t col);

        InnerIterator(SparseMatrix<T, indexT, 1, columnMajor>::Vector& vec);

        //* Operator Overloads *//

        void operator++(int);

        void operator++();

        bool operator==(const InnerIterator& other);

        bool operator!=(const InnerIterator& other);

        bool operator<(const InnerIterator& other);

        bool operator>(const InnerIterator& other);

        T& operator*();

        //* Getters *//

        indexT getIndex();

        indexT outerDim();

        indexT row();

        indexT col();

        T value();

        //* Utility Methods *//
    };

    /**
     * @brief A class to represent a vector in a CSF matrix
     * 
     * @tparam T The data type of the values
     * @tparam indexT The data type of the indices
     * @tparam compressionLevel The compression level used
     * @tparam columnMajor If the matrix is in column major format
     */
    template <typename T, typename indexT, uint8_t compressionLevel, bool columnMajor>
    class SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector {
    private:
        //* Private Class Variables *//

        size_t size;

        void* data;
        void* endPtr;

        uint32_t vecLength;

        uint8_t indexWidth = 1;

        uint32_t nnz = 0;

        //* Private Class Methods *//

        void userChecks();

    public:
        //* Constructors & Destructor *//

        // empty constructor
        Vector();

        /**
         * @name Constructors
        */
        ///@{

        /**
         * @brief Construct a new Vector object from a slice of a CSF matrix
         * 
         * @param mat The CSF matrix to slice
         * @param vec The slice to construct the vector from
         */
        Vector(CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>& mat, uint32_t vec);

        /**
         * @brief Construct a new Vector object from a CSF vector
         * 
         * @param vec The CSF vector to be copied from
         */
        Vector(CSF::SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector& vec);

        ///@}

        // map encoding constructor (for testing and transpose)
        Vector(std::unordered_map<T, std::vector<indexT>>& map, uint32_t vecLength);

        // destructor
        ~Vector();

        //* Operator Overloads *//

        T operator[](uint32_t index);

        operator bool() { return (char*)endPtr - indexWidth > data; };

        typename SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector operator=(typename SparseMatrix<T, indexT, compressionLevel, columnMajor>::Vector& vec);

        //* Getters *//
        //! not written yet
        T coeff(uint32_t index);

        /**
         * @brief Get a pointer to the beginning of the vector
         * 
         * @return void* The pointer to the beginning of the vector
         */
        void* begin();

        /**
         * @brief Get a pointer to the end of the vector
         * 
         * @return void* The pointer to the end of the vector
         */
        void* end();

        /**
         * @brief Get the size of the vector in bytes
         * 
         * @return size_t The size of the vector in bytes
         */
        size_t byteSize();

        /**
         * @brief Get the length of the vector
         * 
         * @return uint32_t The length of the vector
         */
        uint32_t length();

        /**
         * @brief Get the outer dimension of the CSF matrix
         * 
         * @return uint32_t The outer dimension of the CSF matrix
         */
        uint32_t outerSize();

        /**
         * @brief Get the number of non-zero values in the vector
         * 
         * @return uint32_t The number of non-zero values in the vector
         */
        uint32_t nonZeros();

        //* Utility Methods *//

        /**
         * @brief Write the vector to a file
         * 
         * @param filename The filename of the file to write to
         */
        void write(const char* filename);
    };

    /**
     * @brief A class to represent a vector in a CSF matrix (Compression Level 1)
     * 
     * @tparam T The data type of the values
     * @tparam indexT The data type of the indices
     * @tparam columnMajor The column major format of the matrix
     */
    template <typename T, typename indexT, bool columnMajor>
    class SparseMatrix<T, indexT, 1, columnMajor>::Vector {
    private:

        //* Private Class Variables *//

        size_t size = 0;

        T* vals = nullptr;
        indexT* indices = nullptr;

        uint32_t nnz = 0;
        uint32_t vecLength = 0;

        //* Private Class Methods *//

        void userChecks();

    public:

        //* Constructors & Destructor *//

        Vector();

        Vector(CSF::SparseMatrix<T, indexT, 1, columnMajor>& mat, uint32_t vec);

        Vector(CSF::SparseMatrix<T, indexT, 1, columnMajor>::Vector& vec);

        ~Vector();

        //* Operator Overloads *//

        T operator[](uint32_t index);

        //* Getters *//

        uint32_t length();

        uint32_t nonZeros();

        T* values();

        indexT* indexPtr();

        size_t byteSize();

        //* Utility Methods *//

        void write(const char* filename);
    };

}
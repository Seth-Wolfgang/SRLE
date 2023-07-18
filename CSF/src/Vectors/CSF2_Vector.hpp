/**
 * @file CSF2_Vector.hpp
 * @author Skyler Ruiter and Seth Wolfgang
 * @brief CSF2 Vector Class Declerations
 * @version 0.1
 * @date 2023-07-03
 */

#pragma once

namespace CSF {

    /**
     * CSF2 Vector Class \n \n
     * The CSF2 Vector class is a vector class that is used to work with
     * CSF2 matrices. It works with the same logic as the corresponding
     * matrix compression level and is useful when working with these matrices.
    */    
    template <typename T, typename indexT, bool columnMajor>
    class SparseMatrix<T, indexT, 2, columnMajor>::Vector {
        private:

        //* Private Class Variables *//

        size_t size = 0; // size of the vector in bytes

        T *values = nullptr; // values of the vector
        indexT *counts = nullptr; // counts of the vectors values
        indexT *indices = nullptr; // indices of the vectors values
        indexT valuesSize = 0; // size of the values array
        indexT indexSize = 0; // size of the indices array

        uint32_t length = 0; // length of the vector

        uint8_t indexWidth = 1; // width of the indices

        uint32_t nnz = 0; // number of non-zero elements in the vector

        //* Private Class Methods *//

        // User checks to confirm a valid vector
        void userChecks();

        // Calculates the size of the vector in bytes
        void calculateCompSize();


        public:

        //* Constructors & Destructor *//
        /** @name Constructors
         */
        ///@{

        /**
         * Default Vector Constructor \n \n
         * Creates an empty vector with everything set to null/zero.
        */
        Vector() {};

        /**
         * CSF Matrix to Vector Constructor \n \n
         * Creates a vector from a CSF2 Matrix at the given vector index.
         *
         * @note Can only get a vector from a matrix in the storage order of the matrix.
         */
        Vector(CSF::SparseMatrix<T, indexT, 2, columnMajor> &mat, uint32_t vec);

        /**
         * Deep Copy Vector Constructor \n \n
         * Creates a deep copy of the given vector.
         */
        Vector(CSF::SparseMatrix<T, indexT, 2, columnMajor>::Vector &vec);

        /**
         * Destroys the vector.
         */
        ~Vector();

        ///@}

        //* Getters *//
        /** @name Getters
         */
        ///@{

        /**
         * @returns The coefficient at the given index.
         */
        T coeff(uint32_t index);

        /**
         * @returns The size of the vector in bytes.
         */
        size_t byteSize();

        /**
         * @returns The inner size of the vector.
         */
        uint32_t innerSize();

        /**
         * @returns The outer size of the vector.
         */
        uint32_t outerSize();

        /**
         * @returns The number of non-zero elements in the vector.
         */
        uint32_t nonZeros();

        /**
         * @returns The length of the vector.
         */
        uint32_t getLength();

        /**
         * @returns A pointer to the values of the vector.
         */
        T *getValues();

        /**
         * @returns A pointer to the counts of the vector.
         */
        indexT *getCounts();

        /**
         * @returns A pointer to the indices of the vector.
         */
        indexT *getIndices();

        /**
         * @returns The number of unique values in the vector.
         */
        indexT uniqueVals();

        ///@}

        //* Utility Methods *//
        /** @name Utility Methods
         */
        ///@{

        /**
         * Prints the vector dense to the console.
         */
        void print();

        ///@}

        //* Calculations *//
        /** @name Calculation Methods
         */
        ///@{

        /**
         * @returns The norm of the vector.
         */
        double norm();

        /**
         * @returns The sum of the vector.
         */
        T sum();

        /**
         * @returns The dot product of the vector and an Eigen Dense Vector.
         */
        double dot(Eigen::Vector<T, -1> &other);

        /**
         * @returns The dot product of the vector and an Eigen Sparse Vector.
         */
        double dot(Eigen::SparseVector<T, -1> &other);

        ///@}

        //* Operator Overloads *//

        // Coefficient Access Operator
        T operator[](uint32_t index);

        // Assignment Operator
        typename SparseMatrix<T, indexT, 2, columnMajor>::Vector operator=(typename SparseMatrix<T, indexT, 2, columnMajor>::Vector &vec);

        // Equality Operators
        bool operator==(typename SparseMatrix<T, indexT, 2, columnMajor>::Vector &vec);

        // Inequality Operators
        bool operator!=(typename SparseMatrix<T, indexT, 2, columnMajor>::Vector &vec);

        // Scalar Multiplication Operator (In Place)
        void operator*=(T scalar);

        // Scalar Multiplication Operator (Copy)
        typename CSF::SparseMatrix<T, indexT, 2, columnMajor>::Vector operator*(T scalar);
    
    }; // class Vector
    
} // namespace CSF
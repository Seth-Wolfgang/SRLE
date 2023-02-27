namespace CSF {
class Iterator {
    Iterator(CSF::SparseMatrix& matrix) {
        data = matrix.getData();
        endOfData = matrix.getEnd();
        currentIndex = data;

        // read first 20 bytes of fileData put it into params -> metadata
        uint32_t params[5];

        memcpy(&params, currentIndex, 20);
        numRows = params[3];
        numColumns = params[4];

        //Skips metadata and goes to first column
        currentIndex = static_cast<char*>(currentIndex) + 20;
        goToColumn(0);

        //Insures the matrix is not empty
        assert(currentIndex < endOfData);

        // valueWidth is set and the first value is read in
        valueWidth = params[2];
        value = interpretPointer(valueWidth);

        // Read in the width of this run's indices and go to first index
        newIndexWidth = *static_cast<uint8_t*>(currentIndex);
        currentIndex = static_cast<char*>(currentIndex) + 1;

        // std::cout << "value: " << value << std::endl;
        // std::cout << "newIndexWidth: " << (int)newIndexWidth << std::endl;
    }

    Iterator(const char* filePath) {
        readFile(filePath);

        // read first 28 bytes of fileData put it into params -> metadata
        uint32_t params[8];

        memcpy(&params, currentIndex, 20);
        currentIndex = static_cast<char*>(currentIndex) + 20;

        // valueWidth is set and the first value is read in
        valueWidth = params[4];
        value = interpretPointer(valueWidth);
        goToColumn(0);

        // Read in the width of this run's indices and go to first index
        newIndexWidth = *static_cast<uint8_t*>(currentIndex);
        currentIndex = static_cast<char*>(currentIndex) + 1;

        // std::cout << "value: " << value << std::endl;
        // std::cout << "newIndexWidth: " << (int)newIndexWidth << std::endl;
    }

    /**
     * @brief Returns the value of the run.
     *
     * @return T&
     */

    T& operator * () { return value; };

    /**
     * @brief Equality operator of this iterator and another iterator
     * 
     * @param other 
     * @return true 
     * @return false 
     */

    bool operator == (const Iterator& other) {
        return currentIndex == other.getIndex();
    }

    /**
     * @brief Inequality operator of this iterator and another iterator
     * 
     * @param other 
     * @return true 
     * @return false 
     */

    bool operator != (const Iterator& other) {
        return currentIndex != other.getIndex();
    }

    /**
     * @brief Less than operator of this iterator and another iterator
     * 
     * @param other 
     * @return true 
     * @return false 
     */

    bool operator < (const Iterator& other) {
        return currentIndex < other.getIndex();
    }

    /**
     * @brief Greater than operator of this iterator and another iterator
     * 
     * @param other 
     * @return true 
     * @return false 
     */

    bool operator > (const Iterator& other) {
        return currentIndex > other.getIndex();
    }

    /**
     * @brief Getter for the index of the iterator
     *
     */
    uint64_t getIndex() { return index; }

    /**
     * @brief Increment the iterator
     *
     * @return uint64_t
     */

    uint64_t operator++(int) {
        uint64_t newIndex = interpretPointer(newIndexWidth);

        // If newIndex is 0 and not the first index, then the index is a delimitor
        if (newIndex == 0 && !firstIndex) {
            // Value is the first index of the run
            value = interpretPointer(valueWidth);

            // newIndexWidth is the second value in the run
            newIndexWidth = *static_cast<uint8_t*>(currentIndex);
            currentIndex = static_cast<char*>(currentIndex) + 1;

            //Restart the index
            memset(&index, 0, 8);

            // Returns the first index of the run
            index = interpretPointer(newIndexWidth);
            return index;
        }

        // Returns the next index of the run for positive delta encoded runs
        firstIndex = false;
        return index += newIndex;
    }

    /**
     * @brief Check if the iterator is at the end of the the data
     *
     * @return true
     * @return false
     */

    operator bool() { return endOfData != currentIndex; }


    /**
     * @brief Gets the data of a specified column
     *
     * @param column
     * @return char*
     */

    // char* getColumn(uint64_t column) {
    //     //TODO: optimize this function

    //     //Reseets iterator to the beginning and sends it to the corresponding column
    //     Iterator it = *this;
    //     it.index = 0;
    //     it.goToColumn(column);
    //     it.currentIndex = static_cast<char*>(it.currentIndex) + 1;
    //     it.value = it.interpretPointer(it.valueWidth);
    //     char* columnData = (char*)calloc(it.numRows, sizeof(T));

    //     if (numColumns != 1) it.endOfData = static_cast<char*>(data) + *(static_cast<uint64_t*>(data) + 20 + ((column + 1) * 8));

    //     //copy data into new array
    //     while (it) {
    //         it++;
    //         columnData[it.index] = value;
    //     }

    //     return columnData;
    // }

    /**
     * @brief Returns an iterator to the specified column
     * 
     * @param column 
     * @return CSF::Iterator<T> 
     */

    CSF::Iterator<T> getColumn(uint64_t column) {
        Iterator* it = new Iterator(*this);
        it.goToColumn(column);
        it.setEnd(goToColumn(column + 1));

        return *it;
    }

    /**
     * @brief Iterates until iterator hit the next value
     * 
     */

    void nextValue() {
        T currentValue = value;

        while(*this == value) {
            this++;
        }
    }

private:
    /**
     * @brief Read a file into memory
     *
     * @param filePath
     */

    inline void readFile(const char* filePath) {
        FILE* file = fopen(filePath, "rb");

        // Find end of file and allocate size
        fseek(file, 0, SEEK_END);
        int sizeOfFile = ftell(file);
        data = (char*)malloc(sizeof(char*) * sizeOfFile);

        // Read file into memory
        fseek(file, 0, SEEK_SET);
        fread(data, sizeOfFile, 1, file);
        fclose(file);

        currentIndex = data;
        endOfData = data + sizeOfFile;
    }

    /**
     * @brief Read in the next index from the file based on a variable width
     *
     * @param width
     * @return uint64_t
     */

    inline uint64_t interpretPointer(int width) {
        uint64_t newIndex = 0;

        // Case statement takes in 1,2,4, or 8 otherwise the width is invalid
        switch (width) {
        case 1:
            newIndex = static_cast<uint64_t>(*static_cast<uint8_t*>(currentIndex));
            break;
        case 2:
            newIndex = static_cast<uint64_t>(*static_cast<uint16_t*>(currentIndex));
            break;
        case 4:
            newIndex = static_cast<uint64_t>(*static_cast<uint32_t*>(currentIndex));
            break;
        case 8:
            newIndex = static_cast<uint64_t>(*static_cast<uint64_t*>(currentIndex));
            break;
        default:
            if (endOfData == currentIndex) {
                // std::cout << "Value: " << value << std::endl;
                // std::cout << static_cast<int>(*static_cast<uint8_t*>(currentIndex)) << std::endl;
                std::cerr << "Invalid width: " << width << std::endl;
                exit(-1);
            }

            break;
        }
        // std::cout << "newIndex: " << newIndex << std::endl;
        currentIndex = static_cast<char*>(currentIndex) + width;
        return newIndex;
    }

    /**
     * @brief Set the ending address of the iterator
     * 
     * @param end 
     */

    void setEnd(void *end) {
        endOfData = end;
    }

    /**
     * @brief Get the address of a specified column
     * 
     * @param column 
     * @return void*
     */

    //???????????
    // inline void* getColumnAddress(uint64_t column) {
    //     uint64_t temp = ((uint64_t*)data + 20 + (column * 8));
        
        
    //     return (void*)((uint64_t)(data) + temp); //+((uint64_t*)data + 20 + (column * 8));
    // }
    };
} // end of iterator
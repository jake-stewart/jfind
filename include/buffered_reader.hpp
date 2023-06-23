#ifndef BUFFERED_READER_HPP
#define BUFFERED_READER_HPP

#include <vector>

/* getline() is slow because it rellacs for the buffer size
   fgets() is slow because it has to perform a memcpy from stdio buffer

   bufferedReader stores lines sequentially and allocates new buffers
   when needed. it is very fast because it simply reads into a buffer.
   buffers must be manually freed. you can get them with getBuffers()
   buffers are not automatically freed with reset() since it useful
   for the buffers to stick around after resetting.
*/

class BufferedReader {
    int m_capacity = 0;
    int m_offset = 0;
    char *m_buffer = nullptr;
    int m_size = 0;
    std::vector<char*> m_buffers;

public:
    BufferedReader();
    const char *getline();
    bool reset();
    std::vector<char*> getBuffers();
};

#endif

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
    size_t m_capacity = 0;
    size_t m_filled = 0;
    int m_lineStart = 0;
    int m_index = 0;
    char *m_buffer = nullptr;
    std::vector<char*> m_buffers;
    int m_fd;

public:
    BufferedReader();
    const char *getlines(int numLines);
    bool reset();
    void setFd(int fd);
    std::vector<char*> getBuffers();
};

#endif

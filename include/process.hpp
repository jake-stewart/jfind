#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <cstdio>
#include "logger.hpp"

extern "C" {
#include <termios.h>
#include <unistd.h>
#include <sys/wait.h>
}

enum class ProcessState {
    None,
    Suspended,
    Active,
    Finished,
};

class Process {
    Logger m_logger = Logger("Process");
    ProcessState m_state = ProcessState::None;
    int m_pipefd[2];
    pid_t m_child_pid;
    FILE *m_file;

public:
    FILE *getStdout() const;
    ProcessState getState() const;

    bool start(char *const *argv);
    bool suspend();
    bool resume();
    bool end();
};

#endif

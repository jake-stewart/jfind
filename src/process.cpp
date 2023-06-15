#include "../include/process.hpp"
#include <cstdlib>
#include <csignal>

#define READ 0
#define WRITE 1

FILE *Process::getStdout() const {
    return m_file;
}

ProcessState Process::getState() const {
    return m_state;
}

bool Process::start(char *const *argv) {
    switch (m_state) {
        case ProcessState::Suspended:
            m_logger.log("Attempted to start suspended process");
            return false;
        case ProcessState::Active:
            m_logger.log("Attempted to start active process");
            return false;
        case ProcessState::None:
            break;
    }

    if (pipe(m_pipefd) == -1) {
        m_logger.log("pipe failed");
        exit(EXIT_FAILURE);
    }

    m_child_pid = fork();

    if (m_child_pid < 0) {
        m_logger.log("forking failed pid=%d errno=%d", m_child_pid, errno);
        return false;
    }

    if (m_child_pid == 0) {
        close(m_pipefd[READ]);
        if (dup2(m_pipefd[WRITE], STDOUT_FILENO) == -1) {
            _exit(EXIT_FAILURE);
        }
        close(STDERR_FILENO);
        execvp(argv[0], argv);
        _exit(EXIT_FAILURE);
    }

    close(m_pipefd[WRITE]);
    m_file = fdopen(m_pipefd[READ], "r");
    if (m_file == NULL) {
        m_logger.log("fdopen failed errno=%d", errno);
        exit(EXIT_FAILURE);
    }

    m_state = ProcessState::Active;
    return true;
}

bool Process::suspend() {
    switch (m_state) {
        case ProcessState::None:
            m_logger.log("Attempted to suspend null process");
            return false;
        case ProcessState::Suspended:
            m_logger.log("Attempted to suspend suspended process");
            return false;
        case ProcessState::Active:
            break;
    }

    if (kill(m_child_pid, SIGSTOP) < 0) {
        m_logger.log("Could not suspend process errno=%d", errno);
        return false;
    }
    m_state = ProcessState::Suspended;
    return true;
}

bool Process::resume() {
    switch (m_state) {
        case ProcessState::Active:
            m_logger.log("Attempted to resume active process");
            return false;
        case ProcessState::None:
            m_logger.log("Attempted to resume null process");
            return false;
        case ProcessState::Suspended:
            break;
    }

    if (kill(m_child_pid, SIGCONT) < 0) {
        m_logger.log("Could not resume process errno=%d", errno);
        return false;
    }
    m_state = ProcessState::Active;
    return true;
}

bool Process::end() {
    switch (m_state) {
        case ProcessState::None:
            m_logger.log("Attempted to end null process");
            return false;
        default:
            break;
    }

    if (kill(m_child_pid, SIGTERM) < 0) {
        m_logger.log("Could not end process errno=%d", errno);
        return false;
    }
    m_state = ProcessState::None;
    fclose(m_file);
    return true;
}

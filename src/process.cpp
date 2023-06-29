#include "../include/process.hpp"
#include "../include/logger.hpp"
#include <cstdlib>
#include <csignal>

#define READ 0
#define WRITE 1

int Process::getFd() const {
    return m_pipefd[READ];
}

ProcessState Process::getState() const {
    return m_state;
}

bool Process::start(char *const *argv) {
    switch (m_state) {
        case ProcessState::Suspended:
            LOG("Attempted to start suspended process");
            return false;
        case ProcessState::Active:
            LOG("Attempted to start active process");
            return false;
        case ProcessState::None:
            break;
    }

    if (pipe(m_pipefd) == -1) {
        LOG("pipe failed errno=%d", errno);
        exit(EXIT_FAILURE);
    }

    m_child_pid = fork();

    if (m_child_pid < 0) {
        LOG("forking failed pid=%d errno=%d", m_child_pid, errno);
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
    m_state = ProcessState::Active;
    return true;
}

bool Process::suspend() {
    switch (m_state) {
        case ProcessState::None:
            LOG("Attempted to suspend null process");
            return false;
        case ProcessState::Suspended:
            LOG("Attempted to suspend suspended process");
            return false;
        case ProcessState::Active:
            break;
    }

    if (kill(m_child_pid, SIGSTOP) < 0) {
        LOG("Could not suspend process errno=%d", errno);
        return false;
    }
    m_state = ProcessState::Suspended;
    return true;
}

bool Process::resume() {
    switch (m_state) {
        case ProcessState::Active:
            LOG("Attempted to resume active process");
            return false;
        case ProcessState::None:
            LOG("Attempted to resume null process");
            return false;
        case ProcessState::Suspended:
            break;
    }

    if (kill(m_child_pid, SIGCONT) < 0) {
        LOG("Could not resume process errno=%d", errno);
        return false;
    }
    m_state = ProcessState::Active;
    return true;
}

bool Process::end() {
    switch (m_state) {
        case ProcessState::None:
            LOG("Attempted to end null process");
            return false;
        default:
            break;
    }

    if (kill(m_child_pid, SIGKILL) < 0) {
        LOG("Could not end process errno=%d", errno);
        return false;
    }
    close(m_pipefd[READ]);
    m_state = ProcessState::None;
    return true;
}

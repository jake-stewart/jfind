#ifndef INPUT_READER_HPP
#define INPUT_READER_HPP

#include "key.hpp"
#include "event_dispatch.hpp"
#include "mouse_event.hpp"
#include "logger.hpp"
#include "cancellable_reader.hpp"

#include <string>
#include <vector>
#include <thread>

extern "C" {
#include <sys/select.h>
}

bool isContinuationByte(unsigned char ch);
int utf8CharLen(unsigned char ch);

class InputReader : public EventListener {
public:
    InputReader();

    bool getKey(Key *key);
    void setFileDescriptor(int fileDescriptor);

    void onLoop() override;
    void onStart() override;
    void preOnEvent(EventType type) override;
    void onEvent(std::shared_ptr<Event> event) override;

    void onSigInt();

private:
    EventDispatch& m_dispatch = EventDispatch::instance();
    Logger m_logger = Logger("InputReader");

    CancellableReader m_reader;

    // the file descriptor read for input
    int m_fileDescriptor;

    // payload for events which require more than just
    // the Key enum value
    char m_widechar[4];
    std::vector<MouseEvent> m_mouseEvents;

    std::chrono::time_point<std::chrono::system_clock> m_lastClickTime;
    int m_clickCount = 0;
    MouseButton m_lastClickButton = MB_NONE;

    // when reading for user input, the thread is blocked
    // if a quit event occurs, we have to unblock the thread
    // to do this, the thread reads using select on both
    // the input file descriptor and an internal pipe.
    // when quitting, this internal pipe is closed, waking
    // up select and allowing the thread to see the quit event
    fd_set m_set;
    timeval m_timeout;
    int m_pipe[2];

    char getch();
    bool hasKey();
    int parseEsc(Key *key);
    int parseAltKey(char ch, Key *key);
    int parseMouse(std::string& seq, Key *key);
    int parseEsqSeq(std::string& seq, Key *key);
    int parseUtf8(char ch, Key *key);
};

#endif

#ifndef INPUT_READER_HPP
#define INPUT_READER_HPP

#include "key.hpp"
#include "event_dispatch.hpp"
#include "mouse_event.hpp"
#include "logger.hpp"
#include <string>
#include <vector>
#include <sys/select.h>
#include <thread>

bool isContinuationByte(unsigned char ch);
int utf8CharLen(unsigned char ch);

class InputReader : public EventListener {
public:
    InputReader();

    bool getKey(Key *key);
    bool hasKey();
    void setFileDescriptor(int fileDescriptor);

    void onLoop();
    void preOnEvent(EventType type);
    void onEvent(std::shared_ptr<Event> event);

    void onSigInt();

private:
    EventDispatch& m_dispatch = EventDispatch::instance();
    Logger m_logger = Logger("InputReader");

    // the file descriptor read for input
    int m_fileDescriptor;

    // payload for events which require more than just
    // the Key enum value
    char m_widechar[4];
    std::vector<MouseEvent> m_mouseEvents;

    std::chrono::time_point<std::chrono::system_clock> m_lastClickTime;
    int m_clickCount = 0;
    MouseButton m_lastClickButton = MB_NONE;

    fd_set m_set;
    timeval m_timeout;

    char getch();
    int parseEsc(Key *key);
    int parseAltKey(char ch, Key *key);
    int parseMouse(std::string& seq, Key *key);
    int parseEsqSeq(std::string& seq, Key *key);
    int parseUtf8(char ch, Key *key);
};

#endif

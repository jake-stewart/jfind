#include "../include/item_preview_reader.hpp"
#include "../include/config.hpp"
#include "../include/util.hpp"
#include "../include/logger.hpp"

using namespace std::chrono_literals;

ItemPreviewReader::ItemPreviewReader() {
    m_dispatch.subscribe(this, SELECTED_ITEM_CHANGE_EVENT);
    m_interval.setInterval(50ms);
    m_interval.start();
}

void ItemPreviewReader::onStart() {
    m_ansiStringParser.setTabstop(Config::instance().tabstop);

    if (Config::instance().previewLine.size()) {
        try {
            m_lineRegex = std::regex(Config::instance().previewLine);
        }
        catch (const std::regex_error &error) {
            m_lineRegex.reset();
        }
    }
}

void ItemPreviewReader::setItem(Item *item) {
}

void ItemPreviewReader::onEvent(std::shared_ptr<Event> event) {
    LOG("received %s", getEventNames()[event->getType()]);

    switch (event->getType()) {
        case SELECTED_ITEM_CHANGE_EVENT: {
            SelectedItemChangeEvent* selectedItemChangeEvent
                = (SelectedItemChangeEvent*)event.get();
            m_newItem = selectedItemChangeEvent->getItem();
            break;
        }

        default:
            break;
    }
}

bool ItemPreviewReader::readLine() {
    // LOG("getting line");
    const char *buffer = m_reader.getlines(1);
    if (!buffer) {
        return false;
    }
    m_content.lines.push_back(m_ansiStringParser.parse(buffer));
    return true;
}

void ItemPreviewReader::onLoop() {
    if (m_newItem && (m_interval.ticked() || !m_firstPreviewRead)) {
        m_firstPreviewRead = true;
        m_interval.restart();
        m_process.end();

        m_content.lines.clear();
        for (char *buffer : m_reader.getBuffers()) {
            free(buffer);
        }
        m_reader.reset();

        m_content.lineNumber = -1;
        if (m_lineRegex) {
            std::cmatch match;
            if (std::regex_search(m_newItem->c_str(), match, m_lineRegex.value())) {
                std::string lineNumberString = match[0].str();
                try {
                    int lineNumber = std::stoi(lineNumberString);
                    if (lineNumber > 0) {
                        m_content.lineNumber = lineNumber;
                    }
                }
                catch (const std::out_of_range &ex) {
                }
                catch (const std::invalid_argument &ex) {
                }
            }
        }

        std::string command = applyQuery(
            Config::instance().preview, m_newItem.value()
        );
        m_newItem.reset();

        const char *argv[] = {"/bin/sh", "-c", command.c_str(), NULL};
        m_process.start((char **)argv);
        m_reader.setFd(m_process.getFd());
    }

    if (m_process.getState() != ProcessState::Active) {
        if (m_newItem) {
            return awaitEvent(m_interval.getRemaining());
        }
        return awaitEvent();
    }

    if (!readLine()) {
        m_process.end();
        m_dispatch.dispatch(std::make_shared<PreviewReadEvent>(m_content));
    }
}

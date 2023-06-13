#ifndef OPTION_HPP
#define OPTION_HPP

#include "util.hpp"
#include <map>
#include <optional>
#include <string>

class Option {
    public:
        const std::string& getKey() const;
        virtual bool parse(const char *value) = 0;
        virtual ~Option() {}

    protected:
        std::string m_key;
        bool error(const std::string& message) const;
};

class BooleanOption : public Option {
    public:
        BooleanOption(std::string key, bool *value);
        BooleanOption* defaultTrue();
        bool parse(const char *value) override;

    private:
        bool *m_value;
        bool m_defaultValue;
};

class StringOption : public Option {
    public:
        StringOption(std::string key, std::string *value);
        StringOption* nonEmpty();
        bool parse(const char *value) override;

    private:
        std::string *m_value;
        bool m_allowEmpty;
};

class IntegerOption : public Option {
    public:
        IntegerOption(std::string key, int *value);
        IntegerOption* min(int min);
        IntegerOption* max(int max);
        bool parse(const char *value) override;

    private:
        std::optional<int> m_min;
        std::optional<int> m_max;
        int *m_value;
};


class IntVectorOption : public Option {
public:
    IntVectorOption(std::string key, std::vector<int> *value);
    IntVectorOption* min(int min);
    IntVectorOption* max(int max);
    bool parse(const char *value) override;

private:
    std::optional<int> m_min;
    std::optional<int> m_max;
    std::vector<int> *m_value;
};

template <typename T>
class EnumOption : public Option {
    public:
        EnumOption(std::string key, T *value, std::map<std::string, T> lookup) {
            m_lookup = lookup;
            m_key = key;
            m_value = value;
        }

        bool parse(const char *value) {
            if (!value) {
                return error("expects a value");
            }
            auto it = m_lookup.find(value);
            if (it == m_lookup.end()) {
                std::string message = "expects either ";
                int i = 0;
                for (auto it : m_lookup) {
                    if (i == m_lookup.size() - 1) {
                        message += " or ";
                    }
                    else if (i > 0) {
                        message += ", ";
                    }
                    message += "'" + it.first + "'";
                    i++;
                }
                return error(message);
            }
            *m_value = it->second;
            return true;
        }

    private:
        T *m_value;
        std::map<std::string, T> m_lookup;
};

#endif

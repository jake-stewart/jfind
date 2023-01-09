#ifndef OPTION_HPP
#define OPTION_HPP

#include <string>
#include <optional>

class Option {
    public:
        const std::string& getKey();
        virtual bool parse(const char *value) = 0;

    protected:
        std::string m_key;
        bool error(std::string message);
};

class BooleanOption : public Option {
    public:
        BooleanOption(std::string key, bool *value);
        BooleanOption* defaultTrue();
        bool parse(const char *value);

    private:
        bool *m_value;
        bool m_defaultValue;
};

class StringOption : public Option {
    public:
        StringOption(std::string key, std::string *value);
        StringOption* nonEmpty();
        bool parse(const char *value);

    private:
        std::string *m_value;
        bool m_allowEmpty;
};

class IntegerOption : public Option {
    public:
        IntegerOption(std::string key, int *value);
        IntegerOption* min(int min);
        IntegerOption* max(int max);
        bool parse(const char *value);

    private:
        std::optional<int> m_min;
        std::optional<int> m_max;
        int *m_value;
};

#endif

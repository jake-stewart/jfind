#ifndef OPTION_HPP
#define OPTION_HPP

#include "util.hpp"
#include "choice.hpp"
#include <map>
#include <optional>
#include <string>

class Option
{
public:
    const std::string &getKey() const;
    virtual bool parse(const char *value) = 0;
    virtual ~Option() {}

protected:
    std::string m_key;
    bool error(const std::string &message) const;
};

class BooleanOption : public Option
{
public:
    BooleanOption(std::string key, bool *value);
    BooleanOption *defaultTrue();
    bool parse(const char *value) override;

private:
    bool *m_value;
    bool m_defaultValue;
};

class StringOption : public Option
{
public:
    StringOption(std::string key, std::string *value);
    StringOption *nonEmpty();
    bool parse(const char *value) override;

private:
    std::string *m_value;
    bool m_allowEmpty;
};

class FloatOption : public Option
{
public:
    FloatOption(std::string key, float *value);
    FloatOption *min(float min);
    FloatOption *max(float max);
    bool parse(const char *value) override;

private:
    std::optional<float> m_min;
    std::optional<float> m_max;
    float *m_value;
};

class IntegerOption : public Option
{
public:
    IntegerOption(std::string key, int *value);
    IntegerOption *min(int min);
    IntegerOption *max(int max);
    bool parse(const char *value) override;

private:
    std::optional<int> m_min;
    std::optional<int> m_max;
    int *m_value;
};

class IntVectorOption : public Option
{
public:
    IntVectorOption(std::string key, std::vector<int> *value);
    IntVectorOption *min(int min);
    IntVectorOption *max(int max);
    bool parse(const char *value) override;

private:
    std::optional<int> m_min;
    std::optional<int> m_max;
    std::vector<int> *m_value;
};

template <typename T>
class EnumOption : public Option
{
public:
    EnumOption(std::string key, T *value, std::vector<Choice<T>> choices) {
        m_choices = choices;
        m_key = key;
        m_value = value;
    }

    bool parse(const char *value) {
        if (!value) {
            return error("expects a value");
        }

        int idx = -1;
        for (int i = 0; i < m_choices.size(); i++) {
            if (m_choices[i].name == value) {
                idx = i;
                break;
            }
        }

        if (idx == -1) {
            std::string message = "expects either ";
            int i = 0;
            for (const Choice<T>& choice : m_choices) {
                if (i == m_choices.size() - 1) {
                    message += " or ";
                }
                else if (i > 0) {
                    message += ", ";
                }
                message += "'" + choice.name + "'";
                i++;
            }
            return error(message);
        }
        *m_value = m_choices[idx].value;
        return true;
    }

private:
    T *m_value;
    std::vector<Choice<T>> m_choices;
};

#endif

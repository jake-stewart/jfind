#include "../include/option.hpp"
#include "../include/util.hpp"
#include <cstring>
#include <sstream>

bool Option::error(const std::string &message) const {
    printf("The '--%s' option %s\n", m_key.c_str(), message.c_str());
    return false;
}

const std::string &Option::getKey() const {
    return m_key;
}

BooleanOption::BooleanOption(std::string key, bool *value) {
    m_key = key;
    m_value = value;
    m_defaultValue = false;
}

BooleanOption *BooleanOption::defaultTrue() {
    m_defaultValue = true;
    *m_value = true;
    return this;
}

bool BooleanOption::parse(const char *value) {
    if (value) {
        return error("does not expect a value");
    }
    *m_value = !m_defaultValue;
    return true;
}

StringOption::StringOption(std::string key, std::string *value) {
    m_key = key;
    m_value = value;
    m_allowEmpty = true;
}

StringOption *StringOption::nonEmpty() {
    m_allowEmpty = false;
    return this;
}

bool StringOption::parse(const char *value) {
    if (!value) {
        return error("expects a value");
    }
    if (!m_allowEmpty && strlen(value) == 0) {
        return error("cannot be an empty string");
    }
    *m_value = value;
    return true;
}

FloatOption::FloatOption(std::string key, float *value) {
    m_key = key;
    m_value = value;
}

FloatOption *FloatOption::min(float min) {
    m_min = min;
    return this;
}

FloatOption *FloatOption::max(float max) {
    m_max = max;
    return this;
}

bool FloatOption::parse(const char *value) {
    float num;
    try {
        num = std::stof(value);
    }
    catch (const std::out_of_range &) {
        return error("received an out of range integer");
    }
    catch (const std::invalid_argument &) {
        return error("expects an integer");
    }

    if (m_min.has_value() && num < m_min.value()) {
        return error(
            "value cannot be less than " + std::to_string(m_min.value())
        );
    }
    if (m_max.has_value() && num > m_max.value()) {
        return error(
            "value cannot be greater than " + std::to_string(m_max.value())
        );
    }

    *m_value = num;
    return true;
}

IntegerOption::IntegerOption(std::string key, int *value) {
    m_key = key;
    m_value = value;
}

IntegerOption *IntegerOption::min(int min) {
    m_min = min;
    return this;
}

IntegerOption *IntegerOption::max(int max) {
    m_max = max;
    return this;
}

bool IntegerOption::parse(const char *value) {
    if (!isInteger(value)) {
        return error("expects an integer");
    }

    int num;
    try {
        num = std::stoi(value);
    }
    catch (const std::out_of_range &) {
        return error("received an out of range integer");
    }

    if (m_min.has_value() && num < m_min.value()) {
        return error(
            "value cannot be less than " + std::to_string(m_min.value())
        );
    }
    if (m_max.has_value() && num > m_max.value()) {
        return error(
            "value cannot be greater than " + std::to_string(m_max.value())
        );
    }

    *m_value = num;
    return true;
}

IntVectorOption::IntVectorOption(std::string key, std::vector<int> *value) {
    m_key = key;
    m_value = value;
}

IntVectorOption *IntVectorOption::min(int min) {
    m_min = min;
    return this;
}

IntVectorOption *IntVectorOption::max(int max) {
    m_max = max;
    return this;
}

bool IntVectorOption::parse(const char *value) {
    m_value->clear();

    std::istringstream iss(value);
    std::string token;

    while (std::getline(iss, token, ',')) {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);

        if (!isInteger(token.c_str())) {
            return error("expects a list of integers");
        }
        int num;
        try {
            num = std::stoi(token);
        }
        catch (const std::out_of_range &exception) {
            return error("received an out of range integer");
        }

        if (m_min.has_value() && num < m_min.value()) {
            return error(
                "values cannot be less than " + std::to_string(m_min.value())
            );
        }
        if (m_max.has_value() && num > m_max.value()) {
            return error(
                "values cannot be greater than " + std::to_string(m_max.value())
            );
        }

        m_value->push_back(num);
    }

    return true;
}

#include "../include/util.hpp"

std::vector<std::string> split(std::string str, char delim) {
    std::vector<std::string> words;
    std::string word="";

    for(char c : str) {
        if(c == delim){
           words.push_back(word);
           word="";
        }
        else {
           word += c;
        }
    }
    if (word.size() > 0) {
        words.push_back(word);
    }
    return words;
}

std::string toLower(std::string s) {
    for(char &c : s)
        c = tolower(c);
    return s;
}

std::string& strip(std::string& str) {
    int l_idx = 0;
    int r_idx = str.size() - 1;

    while (r_idx >= 0 && (str[r_idx] == ' ' || str[r_idx] == '\t')) {
        r_idx--;
    }
    while (l_idx < r_idx && (str[l_idx] == ' ' || str[l_idx] == '\t')) {
        l_idx++;
    }

    str.erase(r_idx + 1, str.size());
    str.erase(0, l_idx);
    return str;
}

std::vector<std::string> camelSplit(std::string& text) {
    std::vector<std::string> results;

    std::string v(text);

    int idx = 0;
    int size = 0;
    bool upper = false;
    bool number = false;
     
    for (unsigned char c : text) {
        switch (c) {
            case 'a' ... 'z':
            case 128 ... 255:
                if ((upper || number) && size > 1) {
                    results.push_back(v.substr(idx, size - 1));
                    idx += (size - 1);
                    size = 1;
                }
                size++;
                upper = false;
                number = false;
                break;
            case 'A' ... 'Z':
                if ((!upper || number) && size > 1) {
                    results.push_back(v.substr(idx, size));
                    idx += size;
                    size = 0;
                }
                size++;
                number = false;
                upper = true;
                break;
            case '0' ... '9':
                if (!number && size > 1) {
                    results.push_back(v.substr(idx, size));
                    idx += size;
                    size = 0;
                }
                size++;
                number = true;
                break;
            default:
                if (size) {
                    results.push_back(v.substr(idx, size));
                    idx += size;
                    size = 0;
                }
                idx++;
                break;
        }
    }
    if (size) {
        results.push_back(v.substr(idx, size));
    }

    return results;
}

std::vector<std::string_view> camelSplitLower(std::string_view text) {
    std::vector<std::string_view> results;

    std::string_view v(text);

    int idx = 0;
    int size = 0;
    bool upper = false;
    bool number = false;
     
    for (unsigned char c : text) {
        switch (c) {
            case 'a' ... 'z':
            case 128 ... 255:
                if ((upper || number) && size > 1) {
                    results.push_back(v.substr(idx, size - 1));
                    idx += (size - 1);
                    size = 1;
                }
                size++;
                upper = false;
                number = false;
                break;
            case 'A' ... 'Z':
                if ((!upper || number) && size > 1) {
                    results.push_back(v.substr(idx, size));
                    idx += size;
                    size = 0;
                }
                size++;
                upper = true;
                number = false;
                break;
            case '0' ... '9':
                if (!number && size > 1) {
                    results.push_back(v.substr(idx, size));
                    idx += size;
                    size = 0;
                }
                size++;
                number = true;
                break;
            default:
                if (size) {
                    results.push_back(v.substr(idx, size));
                    idx += size;
                    size = 0;
                }
                idx++;
                break;
        }
    }
    if (size) {
        results.push_back(v.substr(idx, size));
    }

    return results;
}

bool getNonEmptyLine(std::istream& is, std::string& line) {
    while (true) {
        if (!std::getline(is, line)) {
            return false;
        }
        line = strip(line);
        if (line.size()) {
            return true;
        }
    }
}

fs::path expandUserPath(std::string user_path) {
    if (user_path.starts_with("~/")) {
        return getenv("HOME") / fs::path(user_path.substr(2));
    }
    return user_path;
}

int mod(int n, int m) {
    return ((n % m) + m) % m;
}

bool isVowel(char c) {
    switch (c) {
        case 'a': case 'A':
        case 'e': case 'E':
        case 'i': case 'I':
        case 'o': case 'O':
        case 'u': case 'U':
            return true;
        default:
            return false;
    }
}

bool is_lower(char c) {
    switch (c) {
        case 'a' ... 'z':
            return true;
        default:
            return false;
    }
}

bool is_number(char c) {
    switch (c) {
        case '0' ... '9':
            return true;
        default:
            return false;
    }
}

bool is_upper(char c) {
    switch (c) {
        case 'A' ... 'Z':
            return true;
        default:
            return false;
    }
}

bool is_alnum(char c) {
    switch (c) {
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '0' ... '9':
            return true;
        default:
            return false;
    }
}

bool is_lower_alnum(char c) {
    switch (c) {
        case 'a' ... 'z':
        case '0' ... '9':
            return true;
        default:
            return false;
    }
}

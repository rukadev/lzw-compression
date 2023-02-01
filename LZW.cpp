#include <unordered_map>
#include <regex>
#include <iostream>
#include "LZW.h"

void init_dictionary(std::unordered_map<int, std::string>& int_dict, std::unordered_map<std::string, int>& string_dict) {
    int length = 0;
    for (int i = 32; i <= 127; i++) {
        if (i != 34 && i != 92) {
            char c = (char) i;
            std::string s(1, c);
            string_dict[s] = length;
            int_dict[length] = s;
            length++;
        }
    }
}

void init_escapemap(std::unordered_map<char, char>& escapemap) {
    int values[3] = {34, 92, 127};
    for (int i = 1; i <= 34; i++) {
        int j = i;
        if (i > 31) {
            j = values[i - 32];
        }
        char c = char(j);
        char e = char(j + 31);
        escapemap[c] = e;
        escapemap[e] = c;
    }
}

int tobase10(std::string value, std::unordered_map<std::string, int>& string_dict) {
    int n = 0;
    for (int i = 0; i < value.size(); i++) {
        n = n + pow(93, i) * string_dict[value.substr(value.size() - i - 1, 1)];
    }
    return n;
}

std::string unescape(std::string s, std::unordered_map<char, char>& escapemap) {
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == 127) {
            char c = s[i + 1];
            s.erase(i, 1);
            s[i] = escapemap[c];
        }
    }
    return s;
}

std::string tobase93(int n, std::unordered_map<int, std::string> int_dict) {
    std::string value;
    while (n != 0) {
        int remainder = n % 93;
        value = int_dict[remainder] + value;
        n = (n - remainder) / 93;
    }
    return value;
}

std::string escape(std::string s, std::unordered_map<char, char>& escapemap) {
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == 34 || s[i] == 92 || s[i] < 32) {
            s[i] = escapemap[s[i]];
            s.insert(i, 1, (char) 127);
            i++;
        }
    }
    return s;
}

std::string compress(std::string& in) {
    std::unordered_map<int, std::string> int_dict;
    std::unordered_map<std::string, int> string_dict;
    std::unordered_map<char, char> escapemap;

    init_dictionary(int_dict, string_dict);
    init_escapemap(escapemap);

    std::string key;
    std::vector<std::string> sequence;
    std::vector<int> spans;

    int size = string_dict.size();
    int width = 1;
    int span = 0;

    std::function<void(std::string)> listkey = [&](std::string key) {
        std::string value = tobase93(string_dict[key], int_dict);
        if (value.size() > width) {
            width = value.size();
            spans.push_back(span);
            span = 0;
        }
        std::string spaces(width - value.size(), ' ');
        sequence.push_back(spaces + value);
        span++;
    };

    std::string text = escape(in, escapemap);

    for (int i = 0; i < text.size(); i++) {
        char c = text[i];
        std::string new_key = key + c;
        if (string_dict.count(new_key)) {
            key = new_key;
        } else {
            listkey(key);
            key = std::string(1, c);
            string_dict[new_key] = size;
            int_dict[size] = new_key;
            size++;
        }
    }

    listkey(key);
    spans.push_back(span);
    std::string result;

    for (int i = 0; i < spans.size(); i++) {
        if (i == 1) {
            result += ",";
        }
        result += std::to_string(spans[i]);
    }

    result += "|";
    for (int i = 0; i < sequence.size(); i++) {
        result += sequence[i];
    }

    return result;
}

std::string decompress(std::string& in) {
    std::unordered_map<int, std::string> int_dict;
    std::unordered_map<std::string, int> string_dict;
    std::unordered_map<char, char> escapemap;

    init_dictionary(int_dict, string_dict);
    init_escapemap(escapemap);

    std::vector<std::string> groups;
    std::vector<std::string> sequence(1, "");

    std::string::size_type pos = in.find('|');
    std::string spans = in.substr(0, pos);
    std::string content = in.substr(pos+1, in.size());

    std::regex pattern("\\d+");
    std::sregex_iterator iter(spans.begin(), spans.end(), pattern);
    std::sregex_iterator end;

    for (int start = 1; iter != end; ++iter) {
        std::smatch match = *iter;
        int span = stoi(match.str());
        int width = groups.size() + 1;
        std::string sub = content.substr(start-1, start + span*width - 1);
        groups.push_back(sub);
        start += span*width;
    }

    std::string previous;
    for (int width = 0; width < groups.size(); width++) {
        std::string group = groups[width];
        std::string buff(width+1, '.');

        std::regex words_regex(buff);
        auto words_begin = std::sregex_iterator(group.begin(), group.end(), words_regex);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; i++) {
            std::smatch match = *i;
            std::string value = match.str();
            std::string entry = int_dict[tobase10(value, string_dict)];

            if (previous.size() > 0) {
                int d_size = int_dict.size();
                if (entry.size() > 0) {
                    sequence.push_back(entry);
                    std::string t = previous + entry[0];
                    int_dict[d_size] = t;
                    string_dict[t] = d_size;
                  
                } else {
                    entry = previous + previous[0];
                    sequence.push_back(entry);
                    int_dict[d_size] = entry;
                    string_dict[entry] = d_size;
                }
            } else {
   
                sequence[0] = entry;
            }
            previous = entry;
        }
    }

    std::string result;
    for (std::string seq : sequence) {
        result += seq;
    }
    return unescape(result, escapemap);
}
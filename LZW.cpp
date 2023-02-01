#include <string>
#include <regex>
#include <iostream>
#include <map>
#include <cmath>
#include <unordered_map>



std::unordered_map<std::string, int> string_dict;
std::unordered_map<int, std::string> int_dict;
std::unordered_map<char, char> escapemap;

void init_dictionary() {
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

void init_escapemap() {
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

int tobase10(std::string value) {
    int n = 0;
    for (int i = 0; i < value.size(); i++) {
        n = n + pow(93, i) * string_dict[value.substr(value.size() - i - 1, 1)];
    }
    return n;
}

string unescape(std::string s) {
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == 127) {
            char c = s[i + 1];
            s.erase(i, 1);
            s[i] = escapemap[c];
        }
    }
    return s;
}

std::string tobase93(int n) {
    string value = "";
    do {
        int remainder = n % 93;
        value = int_dict[remainder] + value;
        n = (n - remainder) / 93;
    } while (n != 0);
    return value;
}

std::string escape(std::string s) {
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == 34 || s[i] == 92 || s[i] < 32) {
            s[i] = escapemap[s[i]];
            s.insert(i, 1, (char) 127);
            i++;
        }
    }
    return s;
}

std::string compress(std::string& t) {
    std::unordered_map<int, std::string> int_dict2 = int_dict;
    std::unordered_map<std::string, int> string_dict2 = string_dict;
    unordered_map<char, char> escapemap2 = escapemap;

    std::string key;
    std::vector<std::string> sequence;
    int size = string_dict2.size();
    int width = 1;
    std::vector<int> spans;
    int span = 0;

    function<void(string)> listkey = [&](string key) {
        string value = tobase93(string_dict2[key]);
        if (value.size() > width) {
            width = value.size();
            spans.push_back(span);
            span = 0;
        }
        string spaces(width - value.size(), ' ');
        sequence.push_back(spaces + value);
        span++;
    };

    std::string text = escape(t);
    std::cout << "Text: " << text << std::endl;

    for (int i = 0; i < text.size(); i++) {
        char c = text[i];
        std::string new_key = key + c;
        std::cout << "New: " << new_key << std::endl;
        if (string_dict2.count(new_key)) {
            key = new_key;
        } else {
            listkey(key);
            key = string(1, c);
            string_dict2[new_key] = size;
            int_dict2[size] = new_key;
            std::cout << "K: " << key << std::endl;
            std::cout << "S: " << size << std::endl;
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
        std::cout << "Here" << spans[i] << std::endl;
        result += std::to_string(spans[i]);
    }

    result += "|";
    for (int i = 0; i < sequence.size(); i++) {
        result += sequence[i];
    }

    return result;
}

std::string decompress(std::string& in) {
    std::unordered_map<int, std::string> int_dict2 = int_dict;
    std::unordered_map<std::string, int> string_dict2 = string_dict;
    std::unordered_map<char, char> escapemap2 = escapemap;

    std::string::size_type pos = in.find('|');

    std::vector<std::string> sequence;
    std::string f = "";
    sequence.push_back(f);
    int start = 1;
    std::string spans = in.substr(0, pos);
    std::string content = in.substr(pos+1, in.size());

    std::regex pattern("\\d+");
    std::sregex_iterator iter(spans.begin(), spans.end(), pattern);
    std::sregex_iterator end;
    std::vector<std::string> groups;

    for (; iter != end; ++iter) {
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
    
        std::string buff;
        buff.append(width+1, '.');

        std::regex words_regex(buff);
        auto words_begin = std::sregex_iterator(group.begin(), group.end(), words_regex);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; i++) {
            std::smatch match = *i;
            std::string value = match.str();
            std::string entry = int_dict2[tobase10(value)];

            if (previous.size() > 0) {
                int d_size = int_dict2.size();
                if (entry.size() > 0) {
                    sequence.push_back(entry);
                    std::string t = previous + entry[0];
                    int_dict2[d_size] = t;
                    string_dict2[t] = d_size;
                  
                } else {
                    entry = previous + previous[0];
                    sequence.push_back(entry);
                    int_dict2[d_size] = entry;
                    string_dict2[entry] = d_size;
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
    return unescape(result);
}




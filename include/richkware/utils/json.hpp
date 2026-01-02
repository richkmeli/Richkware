#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <stdexcept>

namespace nlohmann {

class json {
public:
    using value_type = std::variant<std::nullptr_t, bool, int, double, std::string, std::map<std::string, json>, std::vector<json>>;

    json() : value_(nullptr) {}
    json(std::nullptr_t) : value_(nullptr) {}
    json(bool b) : value_(b) {}
    json(int i) : value_(i) {}
    json(double d) : value_(d) {}
    json(const std::string& s) : value_(s) {}
    json(const char* s) : value_(std::string(s)) {}
    json(const std::map<std::string, json>& obj) : value_(obj) {}
    json(const std::vector<json>& arr) : value_(arr) {}

    // Copy constructor
    json(const json& other) : value_(other.value_) {}

    // Move constructor
    json(json&& other) noexcept : value_(std::move(other.value_)) {}

    json& operator[](const std::string& key) {
        if (!std::holds_alternative<std::map<std::string, json>>(value_)) {
            value_ = std::map<std::string, json>{};
        }
        return std::get<std::map<std::string, json>>(value_)[key];
    }

    const json& operator[](const std::string& key) const {
        if (std::holds_alternative<std::map<std::string, json>>(value_)) {
            const auto& obj = std::get<std::map<std::string, json>>(value_);
            auto it = obj.find(key);
            if (it != obj.end()) {
                return it->second;
            }
        }
        static const json null_json(nullptr);
        return null_json;
    }

    json& operator=(const json& other) { value_ = other.value_; return *this; }
    json& operator=(json&& other) { value_ = std::move(other.value_); return *this; }
    json& operator=(std::nullptr_t) { value_ = nullptr; return *this; }
    json& operator=(bool b) { value_ = b; return *this; }
    json& operator=(int i) { value_ = i; return *this; }
    json& operator=(double d) { value_ = d; return *this; }
    json& operator=(const std::string& s) { value_ = s; return *this; }
    json& operator=(const char* s) { value_ = std::string(s); return *this; }

    std::string dump() const {
        return to_string();
    }

    void push_back(const json& j) {
        if (!std::holds_alternative<std::vector<json>>(value_)) {
            value_ = std::vector<json>{};
        }
        std::get<std::vector<json>>(value_).push_back(j);
    }

    // Type conversion operators
    operator bool() const { return std::get<bool>(value_); }
    operator int() const { return std::get<int>(value_); }
    operator double() const { return std::get<double>(value_); }
    operator std::string() const { return std::get<std::string>(value_); }

    bool is_null() const { return std::holds_alternative<std::nullptr_t>(value_); }
    bool is_boolean() const { return std::holds_alternative<bool>(value_); }
    bool is_number() const { return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_); }
    bool is_string() const { return std::holds_alternative<std::string>(value_); }
    bool is_object() const { return std::holds_alternative<std::map<std::string, json>>(value_); }
    bool is_array() const { return std::holds_alternative<std::vector<json>>(value_); }

    bool contains(const std::string& key) const {
        if (is_object()) {
            const auto& obj = std::get<std::map<std::string, json>>(value_);
            return obj.find(key) != obj.end();
        }
        return false;
    }

    bool empty() const {
        if (is_object()) return std::get<std::map<std::string, json>>(value_).empty();
        if (is_array()) return std::get<std::vector<json>>(value_).empty();
        if (is_string()) return std::get<std::string>(value_).empty();
        return true;
    }

    // Iterator support for arrays
    using iterator = std::vector<json>::iterator;
    using const_iterator = std::vector<json>::const_iterator;

    iterator begin() {
        if (is_array()) return std::get<std::vector<json>>(value_).begin();
        static std::vector<json> empty_vec;
        return empty_vec.begin();
    }

    iterator end() {
        if (is_array()) return std::get<std::vector<json>>(value_).end();
        static std::vector<json> empty_vec;
        return empty_vec.end();
    }

    const_iterator begin() const {
        if (is_array()) return std::get<std::vector<json>>(value_).begin();
        static std::vector<json> empty_vec;
        return empty_vec.begin();
    }

    const_iterator end() const {
        if (is_array()) return std::get<std::vector<json>>(value_).end();
        static std::vector<json> empty_vec;
        return empty_vec.end();
    }

    static json parse(const std::string& s) {
        return parse_json(s);
    }

    class parse_error : public std::exception {
    public:
        explicit parse_error(const std::string& msg) : message_(msg) {}
        const char* what() const noexcept override { return message_.c_str(); }
    private:
        std::string message_;
    };

private:
    value_type value_;

    std::string to_string() const {
        if (std::holds_alternative<std::nullptr_t>(value_)) return "null";
        if (std::holds_alternative<bool>(value_)) return std::get<bool>(value_) ? "true" : "false";
        if (std::holds_alternative<int>(value_)) return std::to_string(std::get<int>(value_));
        if (std::holds_alternative<double>(value_)) return std::to_string(std::get<double>(value_));
        if (std::holds_alternative<std::string>(value_)) return "\"" + std::get<std::string>(value_) + "\"";
        if (std::holds_alternative<std::map<std::string, json>>(value_)) {
            std::string result = "{";
            const auto& obj = std::get<std::map<std::string, json>>(value_);
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                if (it != obj.begin()) result += ",";
                result += "\"" + it->first + "\":" + it->second.to_string();
            }
            result += "}";
            return result;
        }
        if (std::holds_alternative<std::vector<json>>(value_)) {
            std::string result = "[";
            const auto& arr = std::get<std::vector<json>>(value_);
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) result += ",";
                result += arr[i].to_string();
            }
            result += "]";
            return result;
        }
        return "{}";
    }

    static json parse_json(const std::string& s) {
        size_t pos = 0;
        return parse_value(s, pos);
    }

    static json parse_value(const std::string& s, size_t& pos) {
        skip_whitespace(s, pos);
        if (pos >= s.length()) throw parse_error("Unexpected end of input");

        char c = s[pos];
        if (c == '{') return parse_object(s, pos);
        if (c == '[') return parse_array(s, pos);
        if (c == '"') return parse_string(s, pos);
        if (c == 't' || c == 'f') return parse_bool(s, pos);
        if (c == 'n') return parse_null(s, pos);
        if (c == '-' || (c >= '0' && c <= '9')) return parse_number(s, pos);

        throw parse_error("Invalid JSON value");
    }

    static json parse_object(const std::string& s, size_t& pos) {
        std::map<std::string, json> obj;
        pos++; // Skip '{'
        skip_whitespace(s, pos);

        if (s[pos] == '}') { pos++; return json(obj); }

        while (true) {
            skip_whitespace(s, pos);
            if (s[pos] != '"') throw parse_error("Expected string key");
            std::string key = parse_string_value(s, pos);
            skip_whitespace(s, pos);
            if (s[pos] != ':') throw parse_error("Expected ':'");
            pos++;
            skip_whitespace(s, pos);
            json value = parse_value(s, pos);
            obj[key] = value;
            skip_whitespace(s, pos);
            if (s[pos] == '}') { pos++; break; }
            if (s[pos] != ',') throw parse_error("Expected ',' or '}'");
            pos++;
        }
        return json(obj);
    }

    static json parse_array(const std::string& s, size_t& pos) {
        std::vector<json> arr;
        pos++; // Skip '['
        skip_whitespace(s, pos);

        if (s[pos] == ']') { pos++; return json(arr); }

        while (true) {
            skip_whitespace(s, pos);
            arr.push_back(parse_value(s, pos));
            skip_whitespace(s, pos);
            if (s[pos] == ']') { pos++; break; }
            if (s[pos] != ',') throw parse_error("Expected ',' or ']'");
            pos++;
        }
        return json(arr);
    }

    static json parse_string(const std::string& s, size_t& pos) {
        return json(parse_string_value(s, pos));
    }

    static std::string parse_string_value(const std::string& s, size_t& pos) {
        pos++; // Skip opening quote
        std::string result;
        while (pos < s.length() && s[pos] != '"') {
            if (s[pos] == '\\') {
                pos++;
                if (pos >= s.length()) throw parse_error("Invalid escape sequence");
                switch (s[pos]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: throw parse_error("Invalid escape sequence");
                }
            } else {
                result += s[pos];
            }
            pos++;
        }
        if (pos >= s.length() || s[pos] != '"') throw parse_error("Unterminated string");
        pos++;
        return result;
    }

    static json parse_bool(const std::string& s, size_t& pos) {
        if (s.substr(pos, 4) == "true") { pos += 4; return json(true); }
        if (s.substr(pos, 5) == "false") { pos += 5; return json(false); }
        throw parse_error("Invalid boolean");
    }

    static json parse_null(const std::string& s, size_t& pos) {
        if (s.substr(pos, 4) == "null") { pos += 4; return json(nullptr); }
        throw parse_error("Invalid null");
    }

    static json parse_number(const std::string& s, size_t& pos) {
        size_t end = pos;
        bool is_float = false;
        while (end < s.length() && ((s[end] >= '0' && s[end] <= '9') || s[end] == '.' || s[end] == '-' || s[end] == '+' || s[end] == 'e' || s[end] == 'E')) {
            if (s[end] == '.' || s[end] == 'e' || s[end] == 'E') is_float = true;
            end++;
        }
        std::string num_str = s.substr(pos, end - pos);
        pos = end;
        if (is_float) {
            return json(std::stod(num_str));
        } else {
            return json(std::stoi(num_str));
        }
    }

    static void skip_whitespace(const std::string& s, size_t& pos) {
        while (pos < s.length() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r')) {
            pos++;
        }
    }
};

}
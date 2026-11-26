#include "Serializer.hpp"
#include "Exceptions.hpp"
#include <cctype>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace graph_engine {

// =============================================================================
// JSONValue Implementation
// =============================================================================

JSONValue::JSONValue() : type(JSONType::Null), d_val(0.0), b_val(false) {}

JSONValue::JSONValue(JSONType t) : type(t), d_val(0.0), b_val(false) {}

JSONValue::JSONValue(const std::string& s) : type(JSONType::String), s_val(s), d_val(0.0), b_val(false) {}

JSONValue::JSONValue(double d) : type(JSONType::Number), d_val(d), b_val(false) {}

JSONValue::JSONValue(bool b) : type(JSONType::Boolean), d_val(0.0), b_val(b) {}

// =============================================================================
// JSONParser Implementation (Lexer + Recursive Descent)
// =============================================================================

JSONParser::JSONParser(const std::string& input) 
    : json_str(input), pos(0), line(1), col(1) {}

JSONValue JSONParser::parse() {
    skipWhitespace();
    if (pos >= json_str.length()) {
        throw ParseException("Empty JSON input", line, col);
    }

    JSONValue root = parseValue();

    skipWhitespace();
    if (pos < json_str.length()) {
        throw ParseException("Extra characters after JSON root object", line, col);
    }

    return root;
}

void JSONParser::skipWhitespace() {
    while (pos < json_str.length() && std::isspace(static_cast<unsigned char>(json_str[pos]))) {
        if (json_str[pos] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        pos++;
    }
}

char JSONParser::peek() const {
    if (pos >= json_str.length()) return '\0';
    return json_str[pos];
}

char JSONParser::get() {
    if (pos >= json_str.length()) return '\0';
    char c = json_str[pos++];
    if (c == '\n') {
        line++;
        col = 1;
    } else {
        col++;
    }
    return c;
}

JSONValue JSONParser::parseValue() {
    skipWhitespace();
    char c = peek();

    switch (c) {
        case '{': return parseObject();
        case '[': return parseArray();
        case '"': return JSONValue(parseString());
        case 't':
        case 'f': return parseBoolean();
        case 'n': return parseNull();
        default:
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '-') {
                return JSONValue(parseNumber());
            }
            throw ParseException(std::string("Unexpected character: '") + c + "'", line, col);
    }
}

JSONValue JSONParser::parseObject() {
    JSONValue obj(JSONType::Object);
    
    if (get() != '{') throw ParseException("Expected '{'", line, col);
    skipWhitespace();

    if (peek() == '}') {
        get(); // consume '}'
        return obj;
    }

    while (true) {
        skipWhitespace();
        if (peek() != '"') throw ParseException("Expected string key in object", line, col);
        
        std::string key = parseString();
        skipWhitespace();

        if (get() != ':') throw ParseException("Expected ':' after key", line, col);
        
        obj.o_val[key] = parseValue();
        skipWhitespace();

        char next = peek();
        if (next == '}') {
            get();
            break;
        } else if (next == ',') {
            get();
        } else {
            throw ParseException("Expected ',' or '}' in object", line, col);
        }
    }

    return obj;
}

JSONValue JSONParser::parseArray() {
    JSONValue arr(JSONType::Array);

    if (get() != '[') throw ParseException("Expected '['", line, col);
    skipWhitespace();

    if (peek() == ']') {
        get();
        return arr;
    }

    while (true) {
        arr.a_val.push_back(parseValue());
        skipWhitespace();

        char next = peek();
        if (next == ']') {
            get();
            break;
        } else if (next == ',') {
            get();
        } else {
            throw ParseException("Expected ',' or ']' in array", line, col);
        }
    }

    return arr;
}

std::string JSONParser::parseString() {
    if (get() != '"') throw ParseException("Expected '\"'", line, col);
    
    std::string result;
    while (true) {
        if (pos >= json_str.length()) throw ParseException("Unterminated string", line, col);
        
        char c = get();
        if (c == '"') break;
        
        if (c == '\\') {
            if (pos >= json_str.length()) throw ParseException("Unterminated escape sequence", line, col);
            char escape = get();
            switch (escape) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case '/':  result += '/';  break;
                case 'b':  result += '\b'; break;
                case 'f':  result += '\f'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                case 'u': {
                    std::string hex;
                    for (int i = 0; i < 4; ++i) hex += get();
                    uint32_t code = std::stoul(hex, nullptr, 16);
                    result += static_cast<char>(code);
                    break;
                }
                default: throw ParseException("Invalid escape sequence", line, col);
            }
        } else {
            result += c;
        }
    }
    return result;
}

double JSONParser::parseNumber() {
    size_t start_pos = pos;
    bool has_decimal = false;
    bool has_exponent = false;

    if (peek() == '-') get();

    if (!std::isdigit(static_cast<unsigned char>(peek()))) {
        throw ParseException("Invalid number format", line, col);
    }

    while (pos < json_str.length()) {
        char c = peek();
        if (std::isdigit(static_cast<unsigned char>(c))) {
            get();
        } else if (c == '.') {
            if (has_decimal) throw ParseException("Multiple decimal points in number", line, col);
            has_decimal = true;
            get();
        } else if (c == 'e' || c == 'E') {
            if (has_exponent) throw ParseException("Multiple exponents in number", line, col);
            has_exponent = true;
            get();
            if (peek() == '+' || peek() == '-') get();
        } else {
            break;
        }
    }

    std::string num_str = json_str.substr(start_pos, pos - start_pos);
    try {
        return std::stod(num_str);
    } catch (...) {
        throw ParseException("Failed to parse numeric value: " + num_str, line, col);
    }
}

JSONValue JSONParser::parseBoolean() {
    if (json_str.compare(pos, 4, "true") == 0) {
        pos += 4;
        col += 4;
        return JSONValue(true);
    } else if (json_str.compare(pos, 5, "false") == 0) {
        pos += 5;
        col += 5;
        return JSONValue(false);
    }
    throw ParseException("Expected 'true' or 'false'", line, col);
}

JSONValue JSONParser::parseNull() {
    if (json_str.compare(pos, 4, "null") == 0) {
        pos += 4;
        col += 4;
        return JSONValue(JSONType::Null);
    }
    throw ParseException("Expected 'null'", line, col);
}

} // namespace graph_engine

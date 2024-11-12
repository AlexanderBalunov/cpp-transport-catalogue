#include "json.h"

#include <algorithm>
#include <sstream>

using namespace std;

namespace json {

namespace {

Node LoadNode(std::istream& input);

// ----------ПОЛУЧИТЬ NULL ИЗ ВХОДНОГО ПОТОКА----------        
    
Node LoadNull(std::istream& input) {
    string expected_chars = "null"s;
    for (const char ch : expected_chars) {
        if (input.get() != ch) {
            throw ParsingError("Failed to read null from stream"s);
        }
    }
    if (char next = input.peek(); next != EOF 
        && next != ' ' && next != ',' 
        && next != ']' && next != '}'
        && next != '\n' && next != '\t') {
        throw ParsingError("Failed to read null from stream"s);
    }   
    return Node();
}    
    
// ----------ПОЛУЧИТЬ BOOL ИЗ ВХОДНОГО ПОТОКА----------        
    
Node LoadBool(std::istream& input) {
    char c;
    input >> c;
    string true_expected_chars = "rue"s;
    string false_expected_chars = "alse"s;
    Node result;
    if (c == 't') {
        for (const char ch : true_expected_chars) {
            if (input.get() != ch) {
                 throw ParsingError("Failed to read boolean from stream"s);
            }
        }
        result = true;    
    } else if (c == 'f') {
        for (const char ch : false_expected_chars) {
            if (input.get() != ch) {
                 throw ParsingError("Failed to read boolean from stream"s);
            }
        }             
        result = false;        
    } else {
        throw ParsingError("Failed to read boolean from stream"s);
    }
    if (char next = input.peek(); next != EOF 
        && next != ' ' && next != ',' 
        && next != ']' && next != '}'
        && next != '\n' && next != '\t') {
        throw ParsingError("Failed to read boolean from stream"s);
    }
    return result;
}
    
// ----------ПОЛУЧИТЬ ЧИСЛО (INT или DOUBLE) ИЗ ВХОДНОГО ПОТОКА----------    
    
Node LoadNumber(std::istream& input) {
    using namespace std::literals;
    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };
    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}    

// ----------ПОЛУЧИТЬ STRING ИЗ ВХОДНОГО ПОТОКА----------    
    
Node LoadString(std::istream& input) {
    // Считывает содержимое строкового литерала JSON-документа
    // Функцию следует использовать после считывания открывающего символа ":    
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("Double quotes is not closed in the JSON texts"s);
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error"s);
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n': s.push_back('\n'); break;
                case 't': s.push_back('\t'); break;
                case 'r': s.push_back('\r'); break;
                case '"': s.push_back('"'); break;
                case '\\' : s.push_back('\\'); break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

// ----------ПОЛУЧИТЬ ARRAY(массив) ИЗ ВХОДНОГО ПОТОКА----------        
    
    Node LoadArray(istream& input) {
        Array result;
        
        char c;
        while (input >> c && c != ']') {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }
        
        if (c != ']') {
            throw ParsingError("Square brackets are not closed in the JSON texts"s);
        }

        return Node(move(result));
    }
    
// ----------ПОЛУЧИТЬ DICT(словарь) ИЗ ВХОДНОГО ПОТОКА----------        
    
Node LoadDict(istream& input) {
    Dict result;

    char c;
    while (input >> c && c != '}') {
        if (c == ',') {
            input >> c;
        }
        
        string key = LoadString(input).AsString();;
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    
    if (c != '}') {
        throw ParsingError("Сurly brackets is not closed in the JSON texts"s);
    }    

    return Node(move(result));
}

// ----------ПАРСЕР ДЛЯ ПОЛУЧЕНИЯ НУЖНОЙ Node ИЗ ВХОДНОГО ПОТОКА----------    
    
Node LoadNode(istream& input) {
    char c;
    input >> c;
    if (c == '[') {   
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else {        
        input.putback(c);
        if (c == 'n') {
            return LoadNull(input);
        } else if (c == 't' || c == 'f') {
            return LoadBool(input);
        } else {
            return LoadNumber(input);
        }
    }
}

}  // namespace
    
// ----------ГЕТТЕР для Node----------    
    
const Node::Value& Node::GetValue() const { 
    return *this;
}    

// ----------МЕТОДЫ ДЛЯ ПРОВЕРКИ ТИПА ЗНАЧЕНИЯ В Node----------
    
bool Node::IsInt() const {
    return holds_alternative<int>(*this);
}
    
bool Node::IsDouble() const {
    return IsInt() || holds_alternative<double>(*this);
}
    
bool Node::IsPureDouble() const {
    return holds_alternative<double>(*this);
}
    
bool Node::IsBool() const {
    return holds_alternative<bool>(*this);
}
bool Node::IsString() const {
    return holds_alternative<string>(*this);
}
    
bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(*this);
}
    
bool Node::IsArray() const {
    return holds_alternative<Array>(*this);
}
    
bool Node::IsMap() const {
    return holds_alternative<Dict>(*this);
}
    
// ----------МЕТОДЫ, ВОЗВРАЩАЮЩИЕ ЗНАЧЕНИЕ, ХРАНЯЩЕЕСЯ В Node----------
        
int Node::AsInt() const {
    if (!IsInt()) {
        throw logic_error("No int in std::variant variable"s);
    }
    return get<int>(*this);
}
    
bool Node::AsBool() const {
    if (!IsBool()) {
        throw logic_error("No bool in std::variant variable"s);
    }
    return get<bool>(*this);    
}    

double Node::AsDouble() const {
    if (IsPureDouble()) {
        return get<double>(*this);
    } else if (IsInt()) {
        return get<int>(*this);
    } else {
        throw logic_error("No double/int in std::variant variable"s);
    }   
}    
    
const string& Node::AsString() const {
    if (!IsString()) {
        throw logic_error("No string in std::variant variable"s);
    }
    return get<string>(*this);
}    
    
const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw logic_error("No vector in std::variant variable"s);
    }
    return get<Array>(*this);   
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw logic_error("No map in std::variant variable"s);
    }
    return get<Dict>(*this);   
}

// ----------ОПЕРАТОРЫ == и != ДЛЯ Node И Document----------
    
bool operator==(const Node& left, const Node& right) {
    return left.GetValue() == right.GetValue();
}
    
bool operator!=(const Node& left, const Node& right) {
    return !(left == right);
}
    
bool operator==(const Document& left, const Document& right) {
    return left.GetRoot() == right.GetRoot();
}
    
bool operator!=(const Document& left, const Document& right) {
    return !(left == right);
}
    
// ----------РАБОТА С Document (ЗАПОЛНЕНИЕ/ЗАГРУЗКА И ПЕЧАТЬ)----------   
    
Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

ostream& operator<<(std::ostream& output, const Node& node);    

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};    
    
struct NodePrinter {
    
    PrintContext ctx;
    
    void operator()(nullptr_t) const {
        ctx.out << "null"sv;
    }

    void operator()(int value) const {
        ctx.out << value;
    }
    
    void operator()(double value) const {
        ctx.out << value;
    }
    
    void operator()(bool value) const {
        ctx.out << (value ? "true"sv : "false"sv);
    }
    
    void operator()(const string& str) const {
        ctx.out << '"';
        istringstream ss(str);
        auto it = std::istreambuf_iterator<char>(ss);
        auto end = std::istreambuf_iterator<char>();
        while (it != end) {
            switch (*it) {
                case '\n': ctx.out << '\\' << 'n'; break;
                case '\t': ctx.out << '\\' << 't'; break;  
                case '\r': ctx.out << '\\' << 'r'; break;  
                case '"': ctx.out << '\\' << '"'; break;  
                case '\\': ctx.out << '\\' << '\\'; break;
                default: ctx.out << *it;
            }                               
            ++it;
        }
        ctx.out << '"';
    }    

    void operator()(const Array& array) const {
        ctx.out << "[\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& s : array) {
            if (first) {
                first = false;
            } else {
                ctx.out << ",\n"sv;
            }
            inner_ctx.PrintIndent();            
            ctx.out << s;
        }
        ctx.out << '\n';
        ctx.PrintIndent();
        ctx.out << ']';
    }

    void operator()(const Dict& map) const {
        ctx.out << "{\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& [key, node] : map) {
            if (first) {
                first = false;
            } else {
                ctx.out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            ctx.out << '"' << key << '"' << ": "sv << node;
        }
        ctx.out << '\n';
        ctx.PrintIndent();
        ctx.out << '}'; 
    }
};    

ostream& operator<<(std::ostream& output, const Node& node) {
    visit(NodePrinter{output}, node.GetValue());
    return output;
}
    
void Print(const Document& doc, ostream& output) {
   output << doc.GetRoot();
}

}  // namespace json

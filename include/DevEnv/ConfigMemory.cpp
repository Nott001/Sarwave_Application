#include "DevEnv/ConfigMemory.hpp"

#include <QString>
#include <charconv>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <utility>
#include <vector>

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::DevEnv {
namespace {

#ifdef _WIN32
constexpr const char* NewLine = "\r\n";
#else
constexpr const char* NewLine = "\n";
#endif

constexpr const char* CfgPathKey = "CfgPath";

const char* hostKey(bool sending) {
    return sending ? "LanListenHost" : "LanSenderHost";
}

const char* portKey(bool sending) {
    return sending ? "LanListenPort" : "LanSenderPort";
}

void appendUnicodeEscape(std::string& out, char16_t unit) {
    char buf[8];
    std::snprintf(buf, sizeof buf, "\\u%04X", static_cast<unsigned>(unit));
    out += buf;
}

/// A JSON string as System.Text.Json writes it with the default JavaScriptEncoder: printable
/// ASCII as is, except '"', '\\' and the HTML-sensitive & ' + < > ` (escaped as \uXXXX, like '"'),
/// \b \t \n \f \r short escapes, every other control and non-ASCII character as \uXXXX (UTF-16).
std::string jsonString(const std::string& utf8) {
    std::string out = "\"";
    for (const char16_t unit : QString::fromUtf8(utf8).toStdU16String()) {
        switch (unit) {
            case u'\\':
                out += "\\\\";
                continue;
            case u'\b':
                out += "\\b";
                continue;
            case u'\t':
                out += "\\t";
                continue;
            case u'\n':
                out += "\\n";
                continue;
            case u'\f':
                out += "\\f";
                continue;
            case u'\r':
                out += "\\r";
                continue;
            case u'"':
            case u'&':
            case u'\'':
            case u'+':
            case u'<':
            case u'>':
            case u'`':
                appendUnicodeEscape(out, unit);
                continue;
            default:
                break;
        }
        if (unit < 0x20 || unit >= 0x7F) {
            appendUnicodeEscape(out, unit);
        } else {
            out += static_cast<char>(unit);
        }
    }
    return out + "\"";
}

/// A JSON value as JsonNode keeps it: objects keep their members' order, and numbers and the
/// literals keep their text, so whatever else the file holds is written back as it was read.
struct Json {
    enum class Kind { String, Literal, Array, Object };
    Kind kind = Kind::Object;
    /// The decoded string, or a number's or literal's text.
    std::string text;
    std::vector<Json> items;
    std::vector<std::pair<std::string, Json>> members;

    static Json string(std::string value) {
        return {Kind::String, std::move(value), {}, {}};
    }
    static Json number(int value) {
        return {Kind::Literal, std::to_string(value), {}, {}};
    }

    [[nodiscard]] const Json* find(const std::string& key) const {
        for (const auto& [name, value] : members)
            if (name == key) return &value;
        return nullptr;
    }

    void set(const std::string& key, Json value) {
        for (auto& [name, existing] : members) {
            if (name == key) {
                existing = std::move(value);
                return;
            }
        }
        members.emplace_back(key, std::move(value));
    }
};

/// Strict JSON (RFC 8259, as JsonNode.Parse reads it by default); throws on anything else.
class Parser {
   public:
    explicit Parser(std::string_view text) : m_text(text) {
    }

    Json document() {
        Json value = parseValue(0);
        skipSpace();
        if (m_at != m_text.size()) fail();
        return value;
    }

   private:
    struct Malformed {};

    [[noreturn]] static void fail() {
        throw Malformed{};
    }

    void skipSpace() {
        while (m_at < m_text.size() && (m_text[m_at] == ' ' || m_text[m_at] == '\t' ||
                                        m_text[m_at] == '\n' || m_text[m_at] == '\r'))
            ++m_at;
    }

    char peek() {
        skipSpace();
        if (m_at >= m_text.size()) fail();
        return m_text[m_at];
    }

    void expect(char c) {
        if (peek() != c) fail();
        ++m_at;
    }

    Json parseValue(int depth) {
        if (depth > 64) fail();  // JsonReaderOptions.MaxDepth
        const char c = peek();
        if (c == '{') return parseObject(depth);
        if (c == '[') return parseArray(depth);
        if (c == '"') return Json::string(parseString());
        return {Json::Kind::Literal, parseLiteral(), {}, {}};
    }

    Json parseObject(int depth) {
        expect('{');
        Json object;
        if (peek() == '}') {
            ++m_at;
            return object;
        }
        while (true) {
            if (peek() != '"') fail();
            std::string key = parseString();
            if (object.find(key)) fail();
            expect(':');
            object.members.emplace_back(std::move(key), parseValue(depth + 1));
            if (peek() == ',') {
                ++m_at;
                continue;
            }
            expect('}');
            return object;
        }
    }

    Json parseArray(int depth) {
        expect('[');
        Json array{Json::Kind::Array, {}, {}, {}};
        if (peek() == ']') {
            ++m_at;
            return array;
        }
        while (true) {
            array.items.push_back(parseValue(depth + 1));
            if (peek() == ',') {
                ++m_at;
                continue;
            }
            expect(']');
            return array;
        }
    }

    unsigned hex4() {
        if (m_at + 4 > m_text.size()) fail();
        unsigned value = 0;
        const auto result =
            std::from_chars(m_text.data() + m_at, m_text.data() + m_at + 4, value, 16);
        if (result.ptr != m_text.data() + m_at + 4) fail();
        m_at += 4;
        return value;
    }

    std::string parseString() {
        expect('"');
        std::u16string units;
        std::string bytes;  // raw UTF-8 between escapes
        const auto flush = [&] {
            units += QString::fromUtf8(bytes).toStdU16String();
            bytes.clear();
        };
        while (true) {
            if (m_at >= m_text.size()) fail();
            const char c = m_text[m_at++];
            if (c == '"') break;
            if (static_cast<unsigned char>(c) < 0x20) fail();
            if (c != '\\') {
                bytes += c;
                continue;
            }
            if (m_at >= m_text.size()) fail();
            const char e = m_text[m_at++];
            flush();
            switch (e) {
                case '"':
                    units += u'"';
                    break;
                case '\\':
                    units += u'\\';
                    break;
                case '/':
                    units += u'/';
                    break;
                case 'b':
                    units += u'\b';
                    break;
                case 'f':
                    units += u'\f';
                    break;
                case 'n':
                    units += u'\n';
                    break;
                case 'r':
                    units += u'\r';
                    break;
                case 't':
                    units += u'\t';
                    break;
                case 'u':
                    units += static_cast<char16_t>(hex4());
                    break;
                default:
                    fail();
            }
        }
        flush();
        return QString::fromStdU16String(units).toStdString();
    }

    std::string parseLiteral() {
        for (const char* word : {"true", "false", "null"}) {
            const std::string_view w(word);
            if (m_text.substr(m_at, w.size()) == w) {
                m_at += w.size();
                return std::string(w);
            }
        }
        // -?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][+-]?[0-9]+)?
        const std::size_t start = m_at;
        const auto digits = [&] {
            const std::size_t from = m_at;
            while (m_at < m_text.size() && m_text[m_at] >= '0' && m_text[m_at] <= '9') ++m_at;
            if (m_at == from) fail();
        };
        if (m_at < m_text.size() && m_text[m_at] == '-') ++m_at;
        if (m_at < m_text.size() && m_text[m_at] == '0') {
            ++m_at;
        } else {
            digits();
        }
        if (m_at < m_text.size() && m_text[m_at] == '.') {
            ++m_at;
            digits();
        }
        if (m_at < m_text.size() && (m_text[m_at] == 'e' || m_text[m_at] == 'E')) {
            ++m_at;
            if (m_at < m_text.size() && (m_text[m_at] == '+' || m_text[m_at] == '-')) ++m_at;
            digits();
        }
        return std::string(m_text.substr(start, m_at - start));
    }

    std::string_view m_text;
    std::size_t m_at = 0;

    friend std::optional<Json> parse(std::string_view text);
};

std::optional<Json> parse(std::string_view text) {
    // A UTF-8 byte order mark is allowed, as JsonNode.Parse(string) never sees one but
    // File.ReadAllText drops it.
    if (text.starts_with("\xEF\xBB\xBF")) text.remove_prefix(3);
    try {
        return Parser(text).document();
    } catch (const Parser::Malformed&) {
        return std::nullopt;
    }
}

/// Utf8JsonWriter with Indented = true: two spaces a level, "key": value, empty containers as
/// {} and [].
void write(std::string& out, const Json& value, int depth) {
    const std::string indent(static_cast<std::size_t>(2 * (depth + 1)), ' ');
    const std::string closing(static_cast<std::size_t>(2 * depth), ' ');
    switch (value.kind) {
        case Json::Kind::String:
            out += jsonString(value.text);
            return;
        case Json::Kind::Literal:
            out += value.text;
            return;
        case Json::Kind::Array:
            if (value.items.empty()) {
                out += "[]";
                return;
            }
            out += "[";
            for (std::size_t i = 0; i < value.items.size(); ++i) {
                out += (i ? "," : "") + std::string(NewLine) + indent;
                write(out, value.items[i], depth + 1);
            }
            out += NewLine + closing + "]";
            return;
        case Json::Kind::Object:
            if (value.members.empty()) {
                out += "{}";
                return;
            }
            out += "{";
            for (std::size_t i = 0; i < value.members.size(); ++i) {
                out += (i ? "," : "") + std::string(NewLine) + indent +
                       jsonString(value.members[i].first) + ": ";
                write(out, value.members[i].second, depth + 1);
            }
            out += NewLine + closing + "}";
            return;
    }
}

std::string toText(const Json& value) {
    std::string out;
    write(out, value, 0);
    return out;
}

/// config.json's object; an empty one when there is no file or it is not a JSON object.
Json load() {
    if (!std::filesystem::is_regular_file(ConfigMemory::CfgFile)) return {};

    std::ifstream file(ConfigMemory::CfgFile, std::ios::binary);
    if (!file) throw Core::IOException(std::string("Could not read ") + ConfigMemory::CfgFile);
    const std::string text{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    std::optional<Json> document = parse(text);
    if (!document || document->kind != Json::Kind::Object) return {};
    return std::move(*document);
}

/// Changes some settings and keeps the rest.
void update(const std::function<void(Json&)>& change) {
    Json object = load();
    change(object);

    std::ofstream file(ConfigMemory::CfgFile, std::ios::binary | std::ios::trunc);
    if (!file) throw Core::IOException(std::string("Could not write ") + ConfigMemory::CfgFile);
    file << toText(object);
}

[[noreturn]] void wrongKind(const Json& value, const char* wanted) {
    const char* kind = value.kind == Json::Kind::Array                 ? "Array"
                       : value.kind == Json::Kind::Object              ? "Object"
                       : value.kind == Json::Kind::String              ? "String"
                       : value.text == "true" || value.text == "false" ? "Boolean"
                                                                       : "Number";
    throw Core::InvalidOperationException(std::string("An element of type '") + kind +
                                          "' cannot be converted to a '" + wanted + "'.");
}

/// JsonNode?.GetValue<string>(): nullopt for a missing member or JSON null.
std::optional<std::string> stringAt(const Json& object, const char* key) {
    const Json* value = object.find(key);
    if (!value || (value->kind == Json::Kind::Literal && value->text == "null"))
        return std::nullopt;
    if (value->kind != Json::Kind::String) wrongKind(*value, "System.String");
    return value->text;
}

/// JsonNode?.GetValue<int>(): nullopt for a missing member or JSON null.
std::optional<int> intAt(const Json& object, const char* key) {
    const Json* value = object.find(key);
    if (!value || (value->kind == Json::Kind::Literal && value->text == "null"))
        return std::nullopt;
    int result = 0;
    const std::string& text = value->text;
    if (value->kind == Json::Kind::Literal) {
        const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
        if (parsed.ec == std::errc() && parsed.ptr == text.data() + text.size()) return result;
    }
    wrongKind(*value, "System.Int32");
}

}  // namespace

std::string ConfigMemory::serialize(const std::string& value) {
    Json object;
    object.set(CfgPathKey, Json::string(value));
    return toText(object);
}

std::optional<std::string> ConfigMemory::getConfig() {
    return stringAt(load(), CfgPathKey);
}

void ConfigMemory::saveConfigLocation(const std::string& value) {
    update([&](Json& object) { object.set(CfgPathKey, Json::string(value)); });
}

std::optional<ConfigMemory::LanAddress> ConfigMemory::getLanAddress(bool sending) {
    const Json object = load();
    std::optional<std::string> host = stringAt(object, hostKey(sending));
    const std::optional<int> port = intAt(object, portKey(sending));
    if (!host || !port) return std::nullopt;
    return LanAddress{std::move(*host), *port};
}

void ConfigMemory::saveLanAddress(bool sending, const std::string& host, int port) {
    update([&](Json& object) {
        object.set(hostKey(sending), Json::string(host));
        object.set(portKey(sending), Json::number(port));
    });
}

}  // namespace Sarwave::DevEnv

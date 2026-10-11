#include "DevEnv/ConfigMemory.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QString>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::DevEnv {
namespace {

#ifdef _WIN32
constexpr const char* NewLine = "\r\n";
#else
constexpr const char* NewLine = "\n";
#endif

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

}  // namespace

std::string ConfigMemory::serialize(const std::string& value) {
    return std::string("{") + NewLine + "  \"CfgPath\": " + jsonString(value) + NewLine + "}";
}

std::optional<std::string> ConfigMemory::getConfig() {
    if (!std::filesystem::is_regular_file(CfgFile)) return std::nullopt;

    QFile file(QString::fromUtf8(CfgFile));
    if (!file.open(QIODevice::ReadOnly))
        throw Core::IOException(std::string("Could not read ") + CfgFile);

    QJsonParseError error{};
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        throw std::runtime_error(error.errorString().toStdString() + " at offset " +
                                 std::to_string(error.offset));
    }
    if (!doc.isObject()) {
        throw Core::InvalidOperationException(
            "The requested operation requires an element of type 'Object'.");
    }

    const QJsonObject root = doc.object();
    if (!root.contains(QStringLiteral("CfgPath"))) return std::nullopt;

    const QJsonValue cfgPath = root.value(QStringLiteral("CfgPath"));
    if (cfgPath.isNull()) return std::nullopt;
    if (!cfgPath.isString()) {
        throw Core::InvalidOperationException(
            "The requested operation requires an element of type 'String'.");
    }
    return cfgPath.toString().toStdString();
}

void ConfigMemory::saveConfigLocation(const std::string& value) {
    std::ofstream file(CfgFile, std::ios::binary | std::ios::trunc);
    if (!file) throw Core::IOException(std::string("Could not write ") + CfgFile);
    file << serialize(value);
}

}  // namespace Sarwave::DevEnv

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config {

/// A line the device ignores: a "%" comment, or a blank line. Kept in the layout so that writing
/// a config back out preserves its notes and spacing.
class Comment final : public ConfigRecord<Comment> {
   public:
    /// The character that starts a comment line.
    static constexpr char Marker = '%';

    /// An empty line.
    static const std::shared_ptr<const Comment>& blank();

    /// `text`: everything after the "%", kept as written (including a leading space).
    /// Nullopt for a blank line.
    explicit Comment(std::optional<std::string> text) : m_text(std::move(text)) {
    }

    [[nodiscard]] const std::optional<std::string>& text() const {
        return m_text;
    }

    [[nodiscard]] bool isBlank() const {
        return !m_text.has_value();
    }

    [[nodiscard]] std::string paramName() const override {
        return std::string(1, Marker);
    }

    [[nodiscard]] std::string toString() const override {
        return m_text ? Marker + *m_text : std::string();
    }

    bool operator==(const Comment&) const = default;

   private:
    std::optional<std::string> m_text;
};

}  // namespace Sarwave::Core::MMWave::Config

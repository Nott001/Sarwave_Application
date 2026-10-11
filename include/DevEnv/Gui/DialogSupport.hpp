#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class QQmlEngine;
class QQuickWindow;

namespace Sarwave::DevEnv::Gui::Detail {

/// Creates the QML window `typeName` of the Sarwave.DevEnv module with `controller` as its
/// `controller` property, shows it application-modal and centred on `owner` (Avalonia's
/// WindowStartupLocation="CenterOwner"), and processes events until it is closed -- the
/// equivalent of `await dialog.ShowDialog(owner)`.
void showDialog(QQmlEngine& engine, QQuickWindow* owner, const char* typeName, QObject* controller);

/// The value or exception a background job finished with.
template <class R>
class Outcome {
   public:
    static Outcome fromValue(R value) {
        Outcome o;
        o.m_value.emplace(std::move(value));
        return o;
    }

    static Outcome fromError(std::exception_ptr error) {
        Outcome o;
        o.m_error = std::move(error);
        return o;
    }

    /// The value, or rethrows the job's exception.
    R get() {
        if (m_error) std::rethrow_exception(m_error);
        return std::move(*m_value);
    }

   private:
    std::optional<R> m_value;
    std::exception_ptr m_error;
};

/// Runs blocking work (serial I/O) off the GUI thread, like the C# `await Task.Run(...)`.
/// `done(Outcome)` is then invoked on `context`'s thread. Destroying the runner waits for any
/// job still in flight; a completion arriving after `context` is gone is dropped (and with it
/// whatever the job produced).
class BackgroundRunner {
   public:
    ~BackgroundRunner() {
        std::lock_guard lock(m_gate);
        for (auto& thread : m_threads) {
            if (thread.joinable()) thread.join();
        }
    }

    template <class Work, class Done>
    void run(QObject* context, Work work, Done done) {
        using R = std::invoke_result_t<Work>;
        QPointer<QObject> target(context);
        std::lock_guard lock(m_gate);
        m_threads.emplace_back([target, work = std::move(work), done = std::move(done)]() mutable {
            auto outcome = [&] {
                try {
                    return Outcome<R>::fromValue(work());
                } catch (...) {
                    return Outcome<R>::fromError(std::current_exception());
                }
            }();
            if (!target) return;
            // Shared, so the queued call stays copyable even when the result is move-only.
            auto payload =
                std::make_shared<std::pair<Done, Outcome<R>>>(std::move(done), std::move(outcome));
            QMetaObject::invokeMethod(
                target.data(), [payload]() mutable { payload->first(std::move(payload->second)); },
                Qt::QueuedConnection);
        });
    }

   private:
    std::mutex m_gate;
    std::vector<std::thread> m_threads;
};

/// What a std::exception says (ex.Message).
inline QString messageOf(const std::exception_ptr& error) {
    try {
        std::rethrow_exception(error);
    } catch (const std::exception& e) {
        return QString::fromStdString(e.what());
    } catch (...) {
        return QStringLiteral("Unknown error");
    }
}

/// CLI replies are raw bytes; the C# code turns each byte into a char, i.e. Latin-1.
inline QString fromDeviceText(const std::string& bytes) {
    return QString::fromLatin1(bytes.data(), static_cast<qsizetype>(bytes.size()));
}

}  // namespace Sarwave::DevEnv::Gui::Detail

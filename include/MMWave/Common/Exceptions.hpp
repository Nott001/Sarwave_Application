#pragma once

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

/// The exception types the C# code throws, so the port can fail in the same places with the
/// same kind of error, and with the same messages wherever a message reaches the user (a window
/// title, a dialog, the console).
namespace Sarwave::Core {

/// System.InvalidOperationException
class InvalidOperationException : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

/// System.ArgumentException. Like .NET, a parameter name is appended as " (Parameter 'name')".
class ArgumentException : public std::runtime_error {
   public:
    explicit ArgumentException(const std::string& message) : std::runtime_error(message) {
    }

    ArgumentException(const std::string& message, const std::string& paramName)
        : std::runtime_error(message + " (Parameter '" + paramName + "')") {
    }
};

/// System.ArgumentOutOfRangeException.
class ArgumentOutOfRangeException : public ArgumentException {
   public:
    /// new ArgumentOutOfRangeException(paramName): .NET's stock message, naming the parameter.
    explicit ArgumentOutOfRangeException(const std::string& paramName)
        : ArgumentException("Specified argument was out of the range of valid values.", paramName) {
    }

    ArgumentOutOfRangeException(const std::string& paramName, const std::string& message)
        : ArgumentException(message, paramName) {
    }
};

/// System.Collections.Generic.KeyNotFoundException
class KeyNotFoundException : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;

    /// The message Dictionary's indexer gives for a missing key.
    static KeyNotFoundException forKey(const std::string& key) {
        return KeyNotFoundException("The given key '" + key +
                                    "' was not present in the dictionary.");
    }
};

/// System.ObjectDisposedException
class ObjectDisposedException : public InvalidOperationException {
   public:
    explicit ObjectDisposedException(const std::string& objectName)
        : InvalidOperationException("Cannot access a disposed object.\nObject name: '" +
                                    objectName + "'.") {
    }
};

/// System.NotSupportedException
class NotSupportedException : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

/// System.FormatException
class FormatException : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

/// System.IO.IOException
class IOException : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

/// System.IO.FileNotFoundException
class FileNotFoundException : public IOException {
   public:
    using IOException::IOException;
};

/// System.IO.DirectoryNotFoundException
class DirectoryNotFoundException : public IOException {
   public:
    using IOException::IOException;
};

/// System.UnauthorizedAccessException
class UnauthorizedAccessException : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

/// System.IO.InvalidDataException
class InvalidDataException : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

/// System.TimeoutException
class TimeoutException : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

/// System.OperationCanceledException
class OperationCanceledException : public std::runtime_error {
   public:
    OperationCanceledException() : std::runtime_error("The operation was canceled.") {
    }
    using std::runtime_error::runtime_error;
};

/// The .NET type name of the exception the C# code would have thrown at this point.
inline std::string dotNetTypeName(const std::exception& e) {
    if (dynamic_cast<const ObjectDisposedException*>(&e)) return "System.ObjectDisposedException";
    if (dynamic_cast<const InvalidOperationException*>(&e))
        return "System.InvalidOperationException";
    if (dynamic_cast<const ArgumentOutOfRangeException*>(&e))
        return "System.ArgumentOutOfRangeException";
    if (dynamic_cast<const ArgumentException*>(&e)) return "System.ArgumentException";
    if (dynamic_cast<const KeyNotFoundException*>(&e))
        return "System.Collections.Generic.KeyNotFoundException";
    if (dynamic_cast<const NotSupportedException*>(&e)) return "System.NotSupportedException";
    if (dynamic_cast<const FormatException*>(&e)) return "System.FormatException";
    if (dynamic_cast<const FileNotFoundException*>(&e)) return "System.IO.FileNotFoundException";
    if (dynamic_cast<const DirectoryNotFoundException*>(&e))
        return "System.IO.DirectoryNotFoundException";
    if (dynamic_cast<const UnauthorizedAccessException*>(&e))
        return "System.UnauthorizedAccessException";
    if (dynamic_cast<const IOException*>(&e)) return "System.IO.IOException";
    if (dynamic_cast<const InvalidDataException*>(&e)) return "System.IO.InvalidDataException";
    if (dynamic_cast<const TimeoutException*>(&e)) return "System.TimeoutException";
    if (dynamic_cast<const OperationCanceledException*>(&e))
        return "System.OperationCanceledException";
    return "System.Exception";
}

/// What the .NET runtime does with an exception nothing caught: print
/// "Unhandled exception. <Type>: <message>" to stderr and abort the process.
[[noreturn]] inline void failUnhandled(std::exception_ptr error) {
    try {
        std::rethrow_exception(error);
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception. " << dotNetTypeName(e) << ": " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unhandled exception." << std::endl;
    }
    std::abort();
}

}  // namespace Sarwave::Core

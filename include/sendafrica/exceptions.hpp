#pragma once
// SendAfrica SDK exception hierarchy.
// All SDK errors derive from sendafrica::Error so callers can do:
//
//   try { client.sms().send(...); }
//   catch (const sendafrica::Error& e) { ... }
//
// or catch a specific subtype for finer-grained handling.

#include <optional>
#include <stdexcept>
#include <string>

namespace sendafrica {

class Error : public std::runtime_error {
public:
    explicit Error(const std::string& message,
                    std::optional<int> status_code = std::nullopt,
                    std::optional<std::string> request_id = std::nullopt,
                    std::optional<std::string> response_body = std::nullopt)
        : std::runtime_error(message),
          message_(message),
          status_code_(status_code),
          request_id_(request_id),
          response_body_(response_body) {}

    const std::string& message() const noexcept { return message_; }
    std::optional<int> status_code() const noexcept { return status_code_; }
    const std::optional<std::string>& request_id() const noexcept { return request_id_; }
    const std::optional<std::string>& response_body() const noexcept { return response_body_; }

private:
    std::string message_;
    std::optional<int> status_code_;
    std::optional<std::string> request_id_;
    std::optional<std::string> response_body_;
};

class AuthenticationError : public Error { using Error::Error; };
class ValidationError : public Error { using Error::Error; };
class InvalidPhoneError : public ValidationError { using ValidationError::ValidationError; };
class InsufficientCreditsError : public Error { using Error::Error; };

class RateLimitError : public Error {
public:
    explicit RateLimitError(const std::string& message,
                              std::optional<int> status_code = std::nullopt,
                              std::optional<std::string> request_id = std::nullopt,
                              std::optional<std::string> response_body = std::nullopt,
                              std::optional<double> retry_after = std::nullopt)
        : Error(message, status_code, request_id, response_body), retry_after_(retry_after) {}

    std::optional<double> retry_after() const noexcept { return retry_after_; }

private:
    std::optional<double> retry_after_;
};

class NotFoundError : public Error { using Error::Error; };
class ServerError : public Error { using Error::Error; };
class ConnectionError : public Error { using Error::Error; };
class WebhookSignatureError : public Error { using Error::Error; };

}  // namespace sendafrica

#pragma once

#include <optional>
#include <string>
#include <utility>

namespace rodavarion::core {

template <typename T>
class Result final {
public:
    static Result success(T value) {
        return Result(std::move(value), {});
    }

    static Result failure(std::string error) {
        return Result(std::nullopt, std::move(error));
    }

    [[nodiscard]] bool hasValue() const noexcept {
        return value_.has_value();
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return hasValue();
    }

    [[nodiscard]] const T& value() const {
        return value_.value();
    }

    [[nodiscard]] T& value() {
        return value_.value();
    }

    [[nodiscard]] const std::string& error() const noexcept {
        return error_;
    }

private:
    Result(T value, std::string error)
        : value_(std::move(value)), error_(std::move(error)) {}

    Result(std::optional<T> value, std::string error)
        : value_(std::move(value)), error_(std::move(error)) {}

    std::optional<T> value_;
    std::string error_;
};

template <>
class Result<void> final {
public:
    static Result success() {
        return Result(true, {});
    }

    static Result failure(std::string error) {
        return Result(false, std::move(error));
    }

    [[nodiscard]] bool hasValue() const noexcept {
        return success_;
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return success_;
    }

    [[nodiscard]] const std::string& error() const noexcept {
        return error_;
    }

private:
    Result(bool success, std::string error)
        : success_(success), error_(std::move(error)) {}

    bool success_{false};
    std::string error_;
};

} // namespace rodavarion::core

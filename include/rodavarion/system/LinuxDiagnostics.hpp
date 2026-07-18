#pragma once

#include <QString>

namespace rodavarion::system {

class LinuxDiagnostics final {
public:
    [[nodiscard]] static QString report();
};

} // namespace rodavarion::system

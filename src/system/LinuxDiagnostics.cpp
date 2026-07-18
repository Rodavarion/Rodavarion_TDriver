#include "rodavarion/system/LinuxDiagnostics.hpp"

#include <QDir>
#include <QStringList>

#ifdef __linux__
#include <unistd.h>
#endif

namespace rodavarion::system {

QString LinuxDiagnostics::report() {
#ifdef __linux__
    QDir devDirectory("/dev");
    const auto hidrawEntries = devDirectory.entryList(
        {"hidraw*"},
        QDir::System | QDir::Files | QDir::NoDotAndDotDot
    );

    int readable = 0;
    int writable = 0;

    for (const auto& entry : hidrawEntries) {
        const auto path = devDirectory.absoluteFilePath(entry).toLocal8Bit();

        if (::access(path.constData(), R_OK) == 0) {
            ++readable;
        }

        if (::access(path.constData(), W_OK) == 0) {
            ++writable;
        }
    }

    return QString(
        "Operating system: Linux\n"
        "hidraw nodes found: %1\n"
        "Readable hidraw nodes: %2\n"
        "Writable hidraw nodes: %3\n"
        "HID backend: HIDAPI\n"
        "udev rule status: manual verification required\n"
        "Recommendation: do not run the entire GUI as root."
    ).arg(hidrawEntries.size())
     .arg(readable)
     .arg(writable);
#else
    return QString(
        "Operating system: non-Linux\n"
        "Linux hidraw and udev diagnostics are unavailable."
    );
#endif
}

} // namespace rodavarion::system

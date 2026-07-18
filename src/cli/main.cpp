#include <QCoreApplication>
#include <QProcess>
#include <QStringList>
#include <QTextStream>

namespace {

int runSystemctl(
    const QStringList& arguments,
    QTextStream& output
) {
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start("systemctl", arguments);

    if (!process.waitForStarted(2000)) {
        output << "Не вдалося запустити systemctl.\n";
        return 2;
    }

    process.waitForFinished(15000);
    output << QString::fromUtf8(process.readAll());

    return process.exitCode();
}

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication application(argc, argv);
    QTextStream output(stdout);

    const auto arguments = application.arguments();

    if (arguments.size() < 2) {
        output
            << "Rodavarion TDriver Control\n\n"
            << "Використання:\n"
            << "  rodavarion-tdriverctl status\n"
            << "  rodavarion-tdriverctl start\n"
            << "  rodavarion-tdriverctl stop\n"
            << "  rodavarion-tdriverctl restart\n"
            << "  rodavarion-tdriverctl logs\n";
        return 0;
    }

    const auto command = arguments.at(1);

    if (command == "status") {
        return runSystemctl(
            {
                "--user",
                "--no-pager",
                "--full",
                "status",
                "rodavarion-tdriverd.service"
            },
            output
        );
    }

    if (command == "start"
        || command == "stop"
        || command == "restart") {
        return runSystemctl(
            {
                "--user",
                command,
                "rodavarion-tdriverd.service"
            },
            output
        );
    }

    if (command == "logs") {
        QProcess journal;
        journal.setProcessChannelMode(QProcess::ForwardedChannels);
        journal.start(
            "journalctl",
            {
                "--user",
                "-u",
                "rodavarion-tdriverd.service",
                "-f"
            }
        );

        if (!journal.waitForStarted(2000)) {
            output << "Не вдалося запустити journalctl.\n";
            return 2;
        }

        return application.exec();
    }

    output << "Невідома команда: " << command << "\n";
    return 1;
}

#pragma once

#include <QObject>
#include <QString>

namespace rodavarion::context {

class ContextBridge final : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.rodavarion.TDriver.Context")
public:
    explicit ContextBridge(QObject* parent = nullptr);

public slots:
    void SetActiveWindow(const QString& identity);
    QString GetActiveWindow() const;
};

} // namespace rodavarion::context

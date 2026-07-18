#pragma once

#include <QDialog>

class QLabel;
class QPlainTextEdit;
class QPushButton;

namespace rodavarion::gui {

class DependencySetupDialog final : public QDialog {
public:
    explicit DependencySetupDialog(QWidget* parent = nullptr);

    [[nodiscard]] bool backendReady() const noexcept;

private:
    void refreshStatus();
    void installAndConfigure();

    QLabel* titleLabel_{nullptr};
    QLabel* explanationLabel_{nullptr};
    QPlainTextEdit* detailsEdit_{nullptr};
    QPushButton* installButton_{nullptr};
    QPushButton* refreshButton_{nullptr};
    QPushButton* closeButton_{nullptr};

    bool ready_{false};
};

} // namespace rodavarion::gui

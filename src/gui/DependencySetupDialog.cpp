#include "rodavarion/gui/DependencySetupDialog.hpp"

#include "rodavarion/setup/DependencySetupManager.hpp"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace rodavarion::gui {

DependencySetupDialog::DependencySetupDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Підготовка системи");
    resize(690, 500);
    setMinimumSize(560, 410);
    setModal(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(14);

    titleLabel_ = new QLabel(
        "<b>Потрібен системний модуль виконання дій</b>"
    );
    titleLabel_->setObjectName("pageTitle");

    explanationLabel_ = new QLabel(
        "Rodavarion TDriver уже бачить кнопки миші, але для команд "
        "«Копіювати», «Вставити» та інших дій у Wayland потрібен "
        "офіційний пакет ydotool, модуль uinput і користувацька служба.\n\n"
        "Програма може сама перевірити, встановити й активувати все "
        "необхідне. Перед адміністративними змінами Linux покаже "
        "стандартне вікно Polkit для підтвердження."
    );
    explanationLabel_->setWordWrap(true);

    detailsEdit_ = new QPlainTextEdit;
    detailsEdit_->setReadOnly(true);

    auto* buttons = new QHBoxLayout;
    installButton_ = new QPushButton(
        "Встановити й налаштувати автоматично"
    );
    refreshButton_ = new QPushButton("Перевірити знову");
    closeButton_ = new QPushButton("Закрити");

    buttons->addWidget(installButton_);
    buttons->addWidget(refreshButton_);
    buttons->addStretch();
    buttons->addWidget(closeButton_);

    layout->addWidget(titleLabel_);
    layout->addWidget(explanationLabel_);
    layout->addWidget(detailsEdit_, 1);
    layout->addLayout(buttons);

    connect(
        installButton_,
        &QPushButton::clicked,
        this,
        &DependencySetupDialog::installAndConfigure
    );
    connect(
        refreshButton_,
        &QPushButton::clicked,
        this,
        &DependencySetupDialog::refreshStatus
    );
    connect(
        closeButton_,
        &QPushButton::clicked,
        this,
        &QDialog::accept
    );

    refreshStatus();
}

bool DependencySetupDialog::backendReady() const noexcept {
    return ready_;
}

void DependencySetupDialog::refreshStatus() {
    const auto status =
        setup::DependencySetupManager::inspect();

    ready_ = status.ready;
    detailsEdit_->setPlainText(status.summary);

    installButton_->setEnabled(!ready_);
    installButton_->setText(
        ready_
            ? "Система готова"
            : "Встановити й налаштувати автоматично"
    );

    if (ready_) {
        explanationLabel_->setText(
            "Усі необхідні компоненти вже встановлено й активовано. "
            "Після закриття цього вікна Rodavarion TDriver перезапустить "
            "runtime призначених дій."
        );
    }
}

void DependencySetupDialog::installAndConfigure() {
    const auto answer = QMessageBox::question(
        this,
        "Підтвердження налаштування",
        "Програма встановить офіційний пакет ydotool через pacman, "
        "завантажить модуль uinput і ввімкне упаковану з ydotool "
        "користувацьку службу.\n\n"
        "Скрипти користувача не запускатимуться, а прошивка миші "
        "не змінюватиметься.\n\n"
        "Продовжити?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
    );

    if (answer != QMessageBox::Yes) {
        return;
    }

    installButton_->setEnabled(false);
    refreshButton_->setEnabled(false);
    detailsEdit_->setPlainText(
        "Виконується налаштування. Підтвердьте стандартний "
        "системний запит Polkit…"
    );
    QApplication::processEvents();

    const auto result =
        setup::DependencySetupManager::installAndConfigure();

    detailsEdit_->setPlainText(result.details);
    refreshButton_->setEnabled(true);

    if (result.success) {
        ready_ = true;
        installButton_->setText("Система готова");
        QMessageBox::information(
            this,
            "Налаштування завершено",
            "Необхідні компоненти встановлено й активовано. "
            "Rodavarion TDriver може виконувати призначені стандартні дії."
        );
    } else {
        ready_ = false;
        installButton_->setEnabled(true);
        QMessageBox::warning(
            this,
            "Налаштування не завершено",
            "Автоматичне налаштування не вдалося завершити. "
            "Точна причина показана у вікні діагностики."
        );
    }
}

} // namespace rodavarion::gui

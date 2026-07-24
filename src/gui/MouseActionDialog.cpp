#include "rodavarion/gui/MouseActionDialog.hpp"

#include "rodavarion/action/MouseMappingStore.hpp"
#include "rodavarion/i18n/LocalizationService.hpp"
#include "rodavarion/safety/ActionSafetyPolicy.hpp"

#include <QAbstractItemView>
#include <QBrush>
#include <QColor>
#include <QComboBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFont>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cerrno>
#include <cstring>

namespace rodavarion::gui {

namespace {

QString buttonText(const action::MouseButton button) {
    const bool ukrainian =
        i18n::LocalizationService::instance().effectiveLanguage()
        == i18n::Language::Ukrainian;

    if (!ukrainian) {
        switch (button) {
            case action::MouseButton::Back: return "Thumb button 1 — nearest palm";
            case action::MouseButton::Forward: return "Thumb button 2 — middle";
            case action::MouseButton::Extra1: return "Thumb button 3 — nearest front";
            case action::MouseButton::Gesture: return "Haptic button — mouse menu";
            case action::MouseButton::Extra2: return "MagSpeed mode button";
            default: return QString::fromUtf8(action::mouseButtonName(button).data());
        }
    }

    switch (button) {
        case action::MouseButton::Left:       return "Ліва кнопка";
        case action::MouseButton::Right:      return "Права кнопка";
        case action::MouseButton::Middle:     return "Середня кнопка";
        case action::MouseButton::Back:       return "Бічна кнопка 1 — ближча до кисті";
        case action::MouseButton::Forward:    return "Бічна кнопка 2 — середня";
        case action::MouseButton::Gesture:    return "Віброкнопка — меню миші";
        case action::MouseButton::WheelLeft:  return "Колесо ліворуч";
        case action::MouseButton::WheelRight: return "Колесо праворуч";
        case action::MouseButton::Extra1:     return "Бічна кнопка 3 — ближча до носика";
        case action::MouseButton::Extra2:     return "Кнопка режиму магнітного колеса";
        case action::MouseButton::WheelUp:    return "Колесо вгору";
        case action::MouseButton::WheelDown:  return "Колесо вниз";
    }

    return "Невідома кнопка";
}

QString actionText(const action::ActionType value) {
    const bool ukrainian =
        i18n::LocalizationService::instance().effectiveLanguage()
        == i18n::Language::Ukrainian;

    if (!ukrainian) {
        return QString::fromUtf8(action::actionTypeName(value).data());
    }

    switch (value) {
        case action::ActionType::Default:             return "За замовчуванням";
        case action::ActionType::Disabled:            return "Вимкнено";
        case action::ActionType::Copy:                return "Копіювати";
        case action::ActionType::Paste:               return "Вставити";
        case action::ActionType::Cut:                 return "Вирізати";
        case action::ActionType::Undo:                return "Скасувати дію";
        case action::ActionType::Redo:                return "Повторити дію";
        case action::ActionType::SelectAll:           return "Виділити все";
        case action::ActionType::Back:                return "Назад";
        case action::ActionType::Forward:             return "Вперед";
        case action::ActionType::ShowDesktop:         return "Показати стільницю";
        case action::ActionType::ApplicationSwitcher: return "Перемикач програм";
        case action::ActionType::VolumeUp:            return "Збільшити гучність";
        case action::ActionType::VolumeDown:          return "Зменшити гучність";
        case action::ActionType::Mute:                return "Вимкнути звук";
        case action::ActionType::PlayPause:           return "Відтворення / пауза";
        case action::ActionType::CustomShortcut:      return "Власна комбінація";
        case action::ActionType::RunScript:           return "Запустити скрипт";
    }

    return "За замовчуванням";
}

QString automaticInterfacesText() {
    return i18n::LocalizationService::instance().effectiveLanguage()
        == i18n::Language::Ukrainian
        ? "Автоматично — усі сумісні інтерфейси миші"
        : "Automatic — all compatible mouse interfaces";
}

QString monitorErrorText() {
    return i18n::LocalizationService::instance().effectiveLanguage()
        == i18n::Language::Ukrainian
        ? "Помилка монітора"
        : "Monitor error";
}

QString noCompatibleInterfacesText() {
    return i18n::LocalizationService::instance().effectiveLanguage()
        == i18n::Language::Ukrainian
        ? "Немає доступних сумісних інтерфейсів миші"
        : "No readable compatible mouse interfaces";
}

QString activeInterfacesText(const int count) {
    return i18n::LocalizationService::instance().effectiveLanguage()
        == i18n::Language::Ukrainian
        ? QString("Активний безпечний монітор: інтерфейсів %1").arg(count)
        : QString("Safe read-only monitor active: %1 interface(s)").arg(count);
}

} // namespace

MouseActionDialog::MouseActionDialog(
    const device::PeripheralDescriptor& peripheral,
    QWidget* parent
)
    : QDialog(parent),
      peripheral_(peripheral) {
    buildInterface();
    loadMappings();
    refreshInputDevices();
    applyResponsiveSizing();
}

MouseActionDialog::~MouseActionDialog() {
    stopAllMonitors();
}

void MouseActionDialog::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);
    applyResponsiveSizing();
}

void MouseActionDialog::buildInterface() {
    setWindowTitle(
        i18n::LocalizationService::instance().text("mouse.title")
    );
    resize(1040, 760);
    setMinimumSize(680, 520);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(18, 16, 18, 16);
    rootLayout->setSpacing(10);

    deviceLabel_ = new QLabel(
        QString("<b>%1</b><br>%2")
            .arg(QString::fromStdString(
                peripheral_.physicalDevice.productName
            ))
            .arg(QString::fromStdString(
                peripheral_.physicalDevice.manufacturer
            ))
    );

    noteLabel_ = new QLabel(
        i18n::LocalizationService::instance().text("mouse.protected")
    );
    noteLabel_->setWordWrap(true);
    noteLabel_->setObjectName("warningPanel");

    auto* monitorGrid = new QGridLayout;
    monitorGrid->setHorizontalSpacing(8);
    monitorGrid->setVerticalSpacing(6);

    auto* inputLabel = new QLabel(
        i18n::LocalizationService::instance().text("mouse.input_device")
    );

    inputDeviceCombo_ = new QComboBox;
    inputDeviceCombo_->setSizeAdjustPolicy(
        QComboBox::AdjustToMinimumContentsLengthWithIcon
    );
    inputDeviceCombo_->setMinimumContentsLength(18);

    refreshInputButton_ = new QPushButton(
        i18n::LocalizationService::instance().text("button.refresh_input")
    );
    liveMonitorButton_ = new QPushButton(
        i18n::LocalizationService::instance().text("button.start_monitor")
    );
    liveMonitorStatus_ = new QLabel(
        i18n::LocalizationService::instance().text("mouse.monitor_stopped")
    );
    liveMonitorStatus_->setWordWrap(true);

    monitorGrid->addWidget(inputLabel, 0, 0);
    monitorGrid->addWidget(inputDeviceCombo_, 0, 1, 1, 3);
    monitorGrid->addWidget(refreshInputButton_, 1, 1);
    monitorGrid->addWidget(liveMonitorButton_, 1, 2);
    monitorGrid->addWidget(liveMonitorStatus_, 1, 3);
    monitorGrid->setColumnStretch(1, 2);
    monitorGrid->setColumnStretch(3, 1);

    mappingTable_ = new QTableWidget(0, 6);
    mappingTable_->setHorizontalHeaderLabels({
        i18n::LocalizationService::instance().text("mouse.row_button"),
        i18n::LocalizationService::instance().text("mouse.state"),
        i18n::LocalizationService::instance().text("mouse.action"),
        i18n::LocalizationService::instance().text("mouse.parameter"),
        "Файл",
        i18n::LocalizationService::instance().text("mouse.dry_test")
    });

    mappingTable_->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::ResizeToContents
    );
    mappingTable_->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents
    );
    mappingTable_->horizontalHeader()->setSectionResizeMode(
        2, QHeaderView::Interactive
    );
    mappingTable_->horizontalHeader()->setSectionResizeMode(
        3, QHeaderView::Stretch
    );
    mappingTable_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    mappingTable_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);

    mappingTable_->setColumnWidth(2, 190);
    mappingTable_->setAlternatingRowColors(true);
    mappingTable_->verticalHeader()->setVisible(false);
    mappingTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    mappingTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    mappingTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mappingTable_->setHorizontalScrollMode(
        QAbstractItemView::ScrollPerPixel
    );

    auto* testHeader = new QGridLayout;
    auto* testTitle = new QLabel(
        "<b>"
        + i18n::LocalizationService::instance().text("mouse.log")
        + "</b>"
    );
    clearTestButton_ = new QPushButton(
        i18n::LocalizationService::instance().text("button.clear")
    );
    testHeader->addWidget(testTitle, 0, 0);
    testHeader->addWidget(clearTestButton_, 0, 1);
    testHeader->setColumnStretch(0, 1);

    testLog_ = new QListWidget;
    testLog_->setMinimumHeight(110);

    buttons_ = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel
    );

    rootLayout->addWidget(deviceLabel_);
    rootLayout->addWidget(noteLabel_);
    rootLayout->addLayout(monitorGrid);
    rootLayout->addWidget(mappingTable_, 1);
    rootLayout->addLayout(testHeader);
    rootLayout->addWidget(testLog_);
    rootLayout->addWidget(buttons_);

    connect(
        mappingTable_,
        &QTableWidget::currentCellChanged,
        this,
        [this](int currentRow, int, int, int) {
            selectMappingRow(currentRow);
        }
    );

    connect(
        clearTestButton_,
        &QPushButton::clicked,
        testLog_,
        &QListWidget::clear
    );

    connect(
        refreshInputButton_,
        &QPushButton::clicked,
        this,
        &MouseActionDialog::refreshInputDevices
    );

    connect(
        liveMonitorButton_,
        &QPushButton::clicked,
        this,
        &MouseActionDialog::toggleLiveMonitor
    );

    connect(
        buttons_,
        &QDialogButtonBox::accepted,
        this,
        &MouseActionDialog::saveMappings
    );

    connect(
        buttons_,
        &QDialogButtonBox::rejected,
        this,
        &QDialog::reject
    );
}

std::set<action::MouseButton>
MouseActionDialog::inferredButtons() const {
    std::set<action::MouseButton> buttons{
        action::MouseButton::Left,
        action::MouseButton::Right,
        action::MouseButton::Middle
    };

    const auto product = QString::fromStdString(
        peripheral_.physicalDevice.productName
    ).toLower();

    if (product.contains("master")) {
        buttons.insert(action::MouseButton::Back);
        buttons.insert(action::MouseButton::Forward);
        buttons.insert(action::MouseButton::Gesture);
        buttons.insert(action::MouseButton::WheelLeft);
        buttons.insert(action::MouseButton::WheelRight);
        buttons.insert(action::MouseButton::Extra1);
        buttons.insert(action::MouseButton::WheelUp);
        buttons.insert(action::MouseButton::WheelDown);
    }

    return buttons;
}

bool MouseActionDialog::deviceMatchesPeripheral(
    const input::EvdevDeviceInfo& device
) const {
    if (!device.readable || !device.mouseCandidate) {
        return false;
    }

    const QString inputName = device.name.toLower();
    const QString productName = QString::fromStdString(
        peripheral_.physicalDevice.productName
    ).toLower();
    const QString manufacturer = QString::fromStdString(
        peripheral_.physicalDevice.manufacturer
    ).toLower();

    if (inputName.contains("mx master")
        || inputName.contains("logitech")) {
        return productName.contains("master")
            || manufacturer.contains("logitech");
    }

    return inputName.contains("mouse")
        || inputName.contains("trackball");
}

void MouseActionDialog::loadMappings() {
    action::MouseMappingStore store;
    const auto loaded = store.load(
        action::MouseMappingStore::defaultFilePath(),
        peripheral_.physicalDevice.key
    );

    if (!loaded) {
        QMessageBox::warning(
            this,
            "Mouse actions",
            QString::fromStdString(loaded.error())
        );
        return;
    }

    profile_ = loaded.value();
    const auto inferred = inferredButtons();
    mappingTable_->setRowCount(
        static_cast<int>(profile_.mappings.size())
    );

    const auto actions = action::availableMouseActions();

    for (int row = 0;
         row < static_cast<int>(profile_.mappings.size());
         ++row) {
        const auto& mapping =
            profile_.mappings[static_cast<std::size_t>(row)];
        const bool available = inferred.contains(mapping.button);

        auto* buttonItem = new QTableWidgetItem(
            QString("%1 — %2")
                .arg(row + 1)
                .arg(buttonText(mapping.button))
        );
        buttonItem->setFlags(
            buttonItem->flags() & ~Qt::ItemIsEditable
        );
        mappingTable_->setItem(row, 0, buttonItem);

        auto* stateItem = new QTableWidgetItem(
            available
                ? i18n::LocalizationService::instance().text(
                    "state.inferred"
                )
                : i18n::LocalizationService::instance().text(
                    "state.unavailable"
                )
        );
        stateItem->setFlags(
            stateItem->flags() & ~Qt::ItemIsEditable
        );
        mappingTable_->setItem(row, 1, stateItem);

        auto* combo = new QComboBox;
        combo->setSizeAdjustPolicy(
            QComboBox::AdjustToMinimumContentsLengthWithIcon
        );
        combo->setMinimumContentsLength(10);

        for (const auto actionValue : actions) {
            combo->addItem(
                actionText(actionValue),
                static_cast<int>(actionValue)
            );
        }

        const int actionIndex = combo->findData(
            static_cast<int>(mapping.action)
        );
        combo->setCurrentIndex(actionIndex >= 0 ? actionIndex : 0);
        combo->setEnabled(available);
        mappingTable_->setCellWidget(row, 2, combo);

        auto* parameter = new QLineEdit(
            QString::fromStdString(mapping.parameter)
        );
        parameter->setEnabled(available);
        mappingTable_->setCellWidget(row, 3, parameter);

        auto* browseButton = new QPushButton("…");
        browseButton->setToolTip("Обрати виконуваний скрипт");
        browseButton->setEnabled(available);
        mappingTable_->setCellWidget(row, 4, browseButton);

        auto* testButton = new QPushButton(i18n::LocalizationService::instance().text("button.test"));
        testButton->setEnabled(available);
        mappingTable_->setCellWidget(row, 5, testButton);

        connect(browseButton, &QPushButton::clicked, this, [this, row]() {
            auto* combo = qobject_cast<QComboBox*>(mappingTable_->cellWidget(row, 2));
            auto* editor = qobject_cast<QLineEdit*>(mappingTable_->cellWidget(row, 3));
            if (combo == nullptr || editor == nullptr) {
                return;
            }

            const auto selectedAction = static_cast<action::ActionType>(
                combo->currentData().toInt()
            );
            if (selectedAction != action::ActionType::RunScript) {
                QMessageBox::information(
                    this,
                    "Вибір скрипту",
                    "Спочатку оберіть дію «Запустити скрипт» для цієї кнопки."
                );
                return;
            }

            const QFileInfo current(editor->text().trimmed());
            const QString initialPath = current.exists()
                ? current.absoluteFilePath()
                : (current.absolutePath().isEmpty() || current.absolutePath() == "."
                    ? QDir::homePath()
                    : current.absolutePath());

            const QString file = QFileDialog::getOpenFileName(
                this,
                "Оберіть виконуваний скрипт",
                initialPath,
                "Скрипти (*.sh *.bash *.py *.pl *.rb);;Усі файли (*)"
            );
            if (file.isEmpty()) {
                return;
            }

            const QFileInfo selected(file);
            if (!selected.isFile()) {
                QMessageBox::warning(this, "Вибір скрипту", "Обраний шлях не є звичайним файлом.");
                return;
            }

            editor->setText(selected.absoluteFilePath());
            if (!selected.isExecutable()) {
                QMessageBox::information(
                    this,
                    "Скрипт обрано",
                    QString(
                        "Файл обрано, але він не має права на виконання. "
                        "Перед запуском виконайте:\n\nchmod +x \"%1\""
                    ).arg(selected.absoluteFilePath())
                );
            }
        });

        connect(
            combo,
            &QComboBox::currentIndexChanged,
            this,
            [this, row]() {
                updateParameterEditor(row);
            }
        );

        connect(
            testButton,
            &QPushButton::clicked,
            this,
            [this, row]() {
                runDryTest(row);
            }
        );

        updateParameterEditor(row);
    }

    if (mappingTable_->rowCount() > 0) {
        mappingTable_->selectRow(0);
    }
}

void MouseActionDialog::updateParameterEditor(const int row) {
    auto* combo = qobject_cast<QComboBox*>(
        mappingTable_->cellWidget(row, 2)
    );
    auto* editor = qobject_cast<QLineEdit*>(
        mappingTable_->cellWidget(row, 3)
    );
    auto* browseButton = qobject_cast<QPushButton*>(
        mappingTable_->cellWidget(row, 4)
    );
    if (combo == nullptr || editor == nullptr) {
        return;
    }

    const auto actionValue = static_cast<action::ActionType>(
        combo->currentData().toInt()
    );

    const bool customShortcut =
        actionValue == action::ActionType::CustomShortcut;
    const bool script =
        actionValue == action::ActionType::RunScript;

    editor->setEnabled(
        combo->isEnabled() && (customShortcut || script)
    );
    if (browseButton != nullptr) {
        browseButton->setEnabled(combo->isEnabled() && script);
        browseButton->setToolTip(
            script
                ? "Обрати виконуваний скрипт"
                : "Спочатку оберіть дію «Запустити скрипт»"
        );
    }

    if (customShortcut) {
        editor->setPlaceholderText("Ctrl+Shift+V");
    } else if (script) {
        editor->setPlaceholderText(
            i18n::LocalizationService::instance().effectiveLanguage()
                == i18n::Language::Ukrainian
                ? "Шлях до скрипту; виконання поки вимкнено"
                : "Script path; execution remains disabled"
        );
    } else {
        editor->clear();
        editor->setPlaceholderText(
            i18n::LocalizationService::instance().effectiveLanguage()
                == i18n::Language::Ukrainian
                ? "Параметр не потрібен"
                : "No parameter required"
        );
    }
}

void MouseActionDialog::selectMappingRow(const int row) {
    if (row >= 0 && row < mappingTable_->rowCount()) {
        mappingTable_->selectRow(row);
    }
}

void MouseActionDialog::runDryTest(const int row) {
    if (row < 0
        || row >= static_cast<int>(profile_.mappings.size())) {
        return;
    }

    auto* combo = qobject_cast<QComboBox*>(
        mappingTable_->cellWidget(row, 2)
    );
    auto* editor = qobject_cast<QLineEdit*>(
        mappingTable_->cellWidget(row, 3)
    );
    if (combo == nullptr || editor == nullptr) {
        return;
    }

    auto mapping =
        profile_.mappings[static_cast<std::size_t>(row)];
    mapping.action = static_cast<action::ActionType>(
        combo->currentData().toInt()
    );
    mapping.parameter =
        editor->text().trimmed().toStdString();

    const auto decision =
        safety::ActionSafetyPolicy::validateForDryRun(mapping);

    mappingTable_->selectRow(row);
    flashMappingRow(row);

    appendTestMessage(
        QString("[%1] DRY RUN — %2 — %3")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
            .arg(buttonText(mapping.button))
            .arg(QString::fromStdString(decision.reason))
    );
}

void MouseActionDialog::appendTestMessage(
    const QString& message
) {
    testLog_->addItem(message);
    testLog_->scrollToBottom();
}

void MouseActionDialog::refreshInputDevices() {
    stopAllMonitors();
    inputDeviceCombo_->clear();

    discoveredInputDevices_ =
        input::EvdevMouseMonitor::enumerateInputDevices();

    int readableCount = 0;
    int compatibleCount = 0;

    inputDeviceCombo_->addItem(automaticInterfacesText(), "__ALL__");
    inputDeviceCombo_->setItemData(0, true, Qt::UserRole + 1);

    for (const auto& device : discoveredInputDevices_) {
        QString state;

        if (device.readable) {
            state = i18n::LocalizationService::instance().text(
                "state.readable"
            );
            ++readableCount;
        } else if (device.openError == EACCES) {
            state = i18n::LocalizationService::instance().text(
                "state.permission_denied"
            );
        } else {
            state = QString::fromLocal8Bit(
                std::strerror(device.openError)
            );
        }

        const bool compatible = deviceMatchesPeripheral(device);
        if (compatible) {
            ++compatibleCount;
        }

        const QString label = QString("%1 — %2 — %3%4")
            .arg(device.name)
            .arg(device.path)
            .arg(state)
            .arg(
                compatible
                    ? (i18n::LocalizationService::instance()
                           .effectiveLanguage()
                       == i18n::Language::Ukrainian
                       ? " — сумісний"
                       : " — compatible")
                    : ""
            );

        inputDeviceCombo_->addItem(label, device.path);
        const int index = inputDeviceCombo_->count() - 1;
        inputDeviceCombo_->setItemData(
            index,
            device.readable,
            Qt::UserRole + 1
        );
        inputDeviceCombo_->setItemData(
            index,
            compatible,
            Qt::UserRole + 2
        );
    }

    liveMonitorButton_->setEnabled(
        readableCount > 0 && compatibleCount > 0
    );

    if (discoveredInputDevices_.isEmpty()) {
        liveMonitorStatus_->setText(
            i18n::LocalizationService::instance().text(
                "mouse.no_devices"
            )
        );
    } else if (readableCount == 0) {
        liveMonitorStatus_->setText(
            i18n::LocalizationService::instance().text(
                "mouse.no_readable"
            )
        );
        appendTestMessage(
            "[INFO] "
            + i18n::LocalizationService::instance().text(
                "mouse.permission_hint"
            )
        );
    } else if (compatibleCount == 0) {
        liveMonitorStatus_->setText(
            noCompatibleInterfacesText()
        );
    } else {
        liveMonitorStatus_->setText(
            i18n::LocalizationService::instance()
                .text("mouse.refresh_done")
                .arg(discoveredInputDevices_.size())
                .arg(readableCount)
            + QString(" Compatible: %1.").arg(compatibleCount)
        );
    }
}

void MouseActionDialog::toggleLiveMonitor() {
    if (!monitors_.empty()) {
        stopAllMonitors();
        liveMonitorStatus_->setText(
            i18n::LocalizationService::instance().text(
                "mouse.monitor_stopped"
            )
        );
        liveMonitorButton_->setText(
            i18n::LocalizationService::instance().text(
                "button.start_monitor"
            )
        );
        return;
    }

    startSelectedMonitors();
}

void MouseActionDialog::startSelectedMonitors() {
    const QString selection =
        inputDeviceCombo_->currentData().toString();

    std::vector<QString> paths;

    if (selection == "__ALL__") {
        for (const auto& device : discoveredInputDevices_) {
            if (deviceMatchesPeripheral(device)) {
                paths.push_back(device.path);
            }
        }
    } else {
        const int index = inputDeviceCombo_->currentIndex();
        const bool readable = inputDeviceCombo_->itemData(
            index,
            Qt::UserRole + 1
        ).toBool();

        if (!readable) {
            appendTestMessage(
                "[ERROR] "
                + i18n::LocalizationService::instance().text(
                    "mouse.permission_hint"
                )
            );
            return;
        }

        paths.push_back(selection);
    }

    if (paths.empty()) {
        appendTestMessage("[ERROR] " + noCompatibleInterfacesText());
        return;
    }

    int started = 0;

    for (const auto& path : paths) {
        auto monitor =
            std::make_unique<input::EvdevMouseMonitor>();
        connectMonitor(*monitor);

        if (monitor->start(path, true, true)) {
            ++started;
            appendTestMessage(
                "[SAFE] "
                + i18n::LocalizationService::instance()
                    .text("mouse.started")
                    .arg(path)
            );
            monitors_.push_back(std::move(monitor));
        }
    }

    if (started == 0) {
        stopAllMonitors();
        liveMonitorStatus_->setText(monitorErrorText());
        return;
    }

    liveMonitorStatus_->setText(activeInterfacesText(started));
    liveMonitorButton_->setText(
        i18n::LocalizationService::instance().text(
            "button.stop_monitor"
        )
    );
}

void MouseActionDialog::connectMonitor(
    input::EvdevMouseMonitor& monitor
) {
    connect(
        &monitor,
        &input::EvdevMouseMonitor::buttonEvent,
        this,
        &MouseActionDialog::handleLiveButton
    );

    connect(
        &monitor,
        &input::EvdevMouseMonitor::monitorError,
        this,
        [this](const QString& message) {
            appendTestMessage("[ERROR] " + message);
        }
    );
}

void MouseActionDialog::stopAllMonitors() noexcept {
    for (auto& monitor : monitors_) {
        if (monitor) {
            monitor->stop();
        }
    }
    monitors_.clear();
}

void MouseActionDialog::handleLiveButton(
    const action::MouseButton button,
    const bool pressed,
    const int linuxCode
) {
    markButtonConfirmed(button);

    const auto buttons = action::defaultMouseButtons();
    int row = -1;

    for (int index = 0;
         index < static_cast<int>(buttons.size());
         ++index) {
        if (buttons[static_cast<std::size_t>(index)] == button) {
            row = index;
            break;
        }
    }

    if (row >= 0) {
        mappingTable_->selectRow(row);
        mappingTable_->scrollToItem(
            mappingTable_->item(row, 0),
            QAbstractItemView::PositionAtCenter
        );

        if (pressed) {
            flashMappingRow(row);
        }
    }

    appendTestMessage(
        QString("[%1] LIVE READ-ONLY — %2 — %3 — code %4")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
            .arg(buttonText(button))
            .arg(pressed ? "pressed" : "released")
            .arg(linuxCode)
    );
}

void MouseActionDialog::markButtonConfirmed(
    const action::MouseButton button
) {
    confirmedButtons_.insert(button);
    const auto buttons = action::defaultMouseButtons();

    for (int row = 0;
         row < static_cast<int>(buttons.size());
         ++row) {
        if (buttons[static_cast<std::size_t>(row)] != button) {
            continue;
        }

        if (auto* stateItem = mappingTable_->item(row, 1)) {
            stateItem->setText(
                i18n::LocalizationService::instance().text(
                    "state.confirmed"
                )
            );
        }

        if (auto* combo = qobject_cast<QComboBox*>(
                mappingTable_->cellWidget(row, 2))) {
            combo->setEnabled(true);
        }

        if (auto* test = qobject_cast<QPushButton*>(
                mappingTable_->cellWidget(row, 5))) {
            test->setEnabled(true);
        }

        updateParameterEditor(row);
        break;
    }
}

void MouseActionDialog::flashMappingRow(const int row) {
    if (row < 0 || row >= mappingTable_->rowCount()) {
        return;
    }

    const QString activeStyle =
        "background:#f2c85b;"
        "color:#0b1728;"
        "border:2px solid #fff2bd;";

    for (int column = 0;
         column < mappingTable_->columnCount();
         ++column) {
        if (auto* item = mappingTable_->item(row, column)) {
            item->setBackground(QColor("#f2c85b"));
            item->setForeground(QColor("#0b1728"));
        }

        if (auto* widget = mappingTable_->cellWidget(row, column)) {
            widget->setStyleSheet(activeStyle);
        }
    }

    QTimer::singleShot(
        650,
        this,
        [this, row]() {
            if (row < 0 || row >= mappingTable_->rowCount()) {
                return;
            }

            for (int column = 0;
                 column < mappingTable_->columnCount();
                 ++column) {
                if (auto* item = mappingTable_->item(row, column)) {
                    item->setBackground(QBrush());
                    item->setForeground(QBrush());
                }

                if (auto* widget =
                        mappingTable_->cellWidget(row, column)) {
                    widget->setStyleSheet({});
                }
            }

            mappingTable_->selectRow(row);
        }
    );
}

void MouseActionDialog::applyResponsiveSizing() {
    const int widthValue = width();
    const int pointSize = std::clamp(widthValue / 105, 8, 12);
    const int compactPointSize = std::max(8, pointSize - 1);
    const int rowHeight = std::clamp(height() / 24, 27, 38);

    QFont normalFont = font();
    normalFont.setPointSize(pointSize);

    QFont compactFont = font();
    compactFont.setPointSize(compactPointSize);

    setFont(normalFont);
    noteLabel_->setFont(compactFont);
    liveMonitorStatus_->setFont(compactFont);
    mappingTable_->setFont(compactFont);
    mappingTable_->horizontalHeader()->setFont(compactFont);
    testLog_->setFont(compactFont);

    for (int row = 0; row < mappingTable_->rowCount(); ++row) {
        mappingTable_->setRowHeight(row, rowHeight);

        for (int column : {2, 3, 4, 5}) {
            if (auto* widget = mappingTable_->cellWidget(row, column)) {
                widget->setFont(compactFont);
                widget->setMinimumHeight(
                    std::max(24, rowHeight - 4)
                );
            }
        }
    }

    refreshInputButton_->setFont(compactFont);
    liveMonitorButton_->setFont(compactFont);
    clearTestButton_->setFont(compactFont);
    inputDeviceCombo_->setFont(compactFont);

    const int pixelSize = std::clamp(widthValue / 90, 10, 15);
    const QString compactStyle = QString(
        "font-size:%1px;"
        "padding:4px 7px;"
    ).arg(pixelSize);

    refreshInputButton_->setStyleSheet(compactStyle);
    liveMonitorButton_->setStyleSheet(compactStyle);
    clearTestButton_->setStyleSheet(compactStyle);
    inputDeviceCombo_->setStyleSheet(compactStyle);

    for (auto* button : buttons_->buttons()) {
        button->setStyleSheet(compactStyle);
        button->setMinimumWidth(0);
    }

    for (int row = 0; row < mappingTable_->rowCount(); ++row) {
        if (auto* combo = qobject_cast<QComboBox*>(
                mappingTable_->cellWidget(row, 2))) {
            combo->setStyleSheet(compactStyle);
        }
        if (auto* editor = qobject_cast<QLineEdit*>(
                mappingTable_->cellWidget(row, 3))) {
            editor->setStyleSheet(compactStyle);
        }
        if (auto* test = qobject_cast<QPushButton*>(
                mappingTable_->cellWidget(row, 5))) {
            test->setStyleSheet(compactStyle);
            test->setMinimumWidth(0);
        }
    }

    const int buttonHeight = std::clamp(height() / 22, 28, 40);
    refreshInputButton_->setMinimumHeight(buttonHeight);
    liveMonitorButton_->setMinimumHeight(buttonHeight);
    clearTestButton_->setMinimumHeight(buttonHeight);

    if (widthValue < 820) {
        mappingTable_->setColumnWidth(2, 145);
        testLog_->setMinimumHeight(80);
    } else if (widthValue < 1000) {
        mappingTable_->setColumnWidth(2, 170);
        testLog_->setMinimumHeight(100);
    } else {
        mappingTable_->setColumnWidth(2, 210);
        testLog_->setMinimumHeight(130);
    }
}

void MouseActionDialog::saveMappings() {
    const bool monitorWasActive = !monitors_.empty();
    stopAllMonitors();
    liveMonitorStatus_->setText("Монітор автоматично зупинено для безпечного збереження.");
    auto allowedButtons = inferredButtons();
    allowedButtons.insert(
        confirmedButtons_.begin(),
        confirmedButtons_.end()
    );

    for (int row = 0;
         row < static_cast<int>(profile_.mappings.size());
         ++row) {
        auto& mapping =
            profile_.mappings[static_cast<std::size_t>(row)];

        if (!allowedButtons.contains(mapping.button)) {
            mapping.action = action::ActionType::Default;
            mapping.parameter.clear();
            continue;
        }

        auto* combo = qobject_cast<QComboBox*>(
            mappingTable_->cellWidget(row, 2)
        );
        auto* editor = qobject_cast<QLineEdit*>(
            mappingTable_->cellWidget(row, 3)
        );

        if (combo == nullptr || editor == nullptr) {
            continue;
        }

        mapping.action = static_cast<action::ActionType>(
            combo->currentData().toInt()
        );
        mapping.parameter =
            editor->text().trimmed().toStdString();

        const auto decision =
            safety::ActionSafetyPolicy::validateForDryRun(mapping);

        if (!decision.allowed) {
            QMessageBox::warning(this, "Налаштування кнопок", QString("%1: %2").arg(buttonText(mapping.button)).arg(QString::fromStdString(decision.reason)));
            if (monitorWasActive) startSelectedMonitors();
            return;
        }
    }

    action::MouseMappingStore store;
    const auto result = store.save(
        action::MouseMappingStore::defaultFilePath(),
        profile_
    );

    if (!result) {
        QMessageBox::critical(this, "Налаштування кнопок", QString::fromStdString(result.error()));
        if (monitorWasActive) startSelectedMonitors();
        return;
    }

    accept();
}

} // namespace rodavarion::gui

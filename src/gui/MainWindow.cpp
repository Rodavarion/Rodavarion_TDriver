#include "rodavarion/gui/MainWindow.hpp"

#include "rodavarion/capability/DeviceCapability.hpp"
#include "rodavarion/context/ApplicationContext.hpp"
#include "rodavarion/system/LinuxDiagnostics.hpp"
#include "rodavarion/i18n/LocalizationService.hpp"
#include "rodavarion/runtime/MouseRuntimeController.hpp"
#include "rodavarion/runtime/DesktopActionExecutor.hpp"
#include "rodavarion/setup/DependencySetupManager.hpp"
#include "rodavarion/setup/DependencyMaintenanceManager.hpp"
#include "rodavarion/gui/DependencySetupDialog.hpp"
#include "rodavarion/gui/MouseActionDialog.hpp"
#include "rodavarion/transport/TransportType.hpp"

#include <QProcess>
#include <QAbstractItemView>
#include <QActionGroup>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QDateTime>
#include <QDesktopServices>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QUrl>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QSplitter>
#include <QScrollArea>
#include <QFrame>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSystemTrayIcon>
#include <QPainter>
#include <QPainterPath>
#include <QAction>
#include <QPixmap>
#include <QIcon>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <stdexcept>

namespace rodavarion::gui {

namespace {


QIcon createRodavarionIcon() {
    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath shield;
    shield.moveTo(64, 8);
    shield.lineTo(108, 24);
    shield.lineTo(102, 78);
    shield.quadTo(94, 106, 64, 120);
    shield.quadTo(34, 106, 26, 78);
    shield.lineTo(20, 24);
    shield.closeSubpath();

    painter.setPen(QPen(QColor("#f3ca62"), 7));
    painter.setBrush(QColor("#0b1728"));
    painter.drawPath(shield);

    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(58);
    painter.setFont(font);
    painter.setPen(QColor("#e8eef7"));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "V");

    return QIcon(pixmap);
}

QString connectionName(const device::ConnectionType type) {
    switch (type) {
        case device::ConnectionType::Usb:
            return "USB";
        case device::ConnectionType::Bluetooth:
            return "Bluetooth";
        case device::ConnectionType::WirelessReceiver:
            return "Бездротовий приймач";
        case device::ConnectionType::Virtual:
            return "Віртуальний";
        case device::ConnectionType::Unknown:
            return "Невідомо";
    }

    return "Невідомо";
}

QLabel* createTitle(const QString& text) {
    auto* label = new QLabel(text);
    label->setObjectName("pageTitle");
    return label;
}

QString profileSetting(
    const profile::Profile& profile,
    const std::string& key,
    const QString& fallback
) {
    const auto iterator = profile.settings.find(key);
    return iterator == profile.settings.end()
        ? fallback
        : QString::fromStdString(iterator->second);
}

QString capabilityList(
    const capability::DeviceClassification& classification
) {
    if (classification.capabilities.empty()) {
        return "Ще не визначено";
    }

    QStringList values;
    for (const auto value : classification.capabilities) {
        values.append(
            QString::fromUtf8(
                capability::capabilityName(value).data()
            )
        );
    }

    return values.join(", ");
}

} // namespace

MainWindow::MainWindow(core::Application& application)
    : application_(application) {
    buildInterface();
    buildLanguageMenu();
    applyLanguage();
    applyTheme();
    restoreWindowState();
    createTrayIcon();

    application_.events().subscribe(
        "profiles.saved",
        [this](const event::Event& event) {
            statusLabel_->setText(
                "Профілі збережено: "
                + QString::fromStdString(event.payload)
            );
        }
    );

    mouseRuntime_ = std::make_unique<runtime::MouseRuntimeController>(this);
    refreshDevices();
    restartMouseRuntime();
    QTimer::singleShot(0, this, [this]() {
        ensureActionBackend(false);
    });

    if (profileList_->count() > 0) {
        profileList_->setCurrentRow(0);
        loadSelectedProfile();
    }

    QTimer::singleShot(350, this, [this]() {
        showWelcomeDialogIfNeeded();
    });
}


MainWindow::~MainWindow() = default;

void MainWindow::buildInterface() {
    setWindowTitle(i18n::LocalizationService::instance().text("app.title"));
    resize(1360, 860);
    setMinimumSize(1080, 700);

    auto* central = new QWidget(this);
    auto* rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    navigation_ = new QListWidget(central);
    navigation_->setObjectName("navigation");
    navigation_->setFixedWidth(240);
    navigation_->addItems({"", "", "", "", "", "", ""});
    navigation_->setCurrentRow(0);

    pages_ = new QStackedWidget(central);

    auto* devicesPage = new QWidget;
    auto* devicesLayout = new QVBoxLayout(devicesPage);
    devicesLayout->setContentsMargins(28, 24, 28, 24);
    devicesLayout->setSpacing(12);

    auto* devicesHeader = new QHBoxLayout;
    devicesHeader->addWidget(createTitle(""));
    devicesHeader->addStretch();

    refreshButton_ = new QPushButton;
    devicesHeader->addWidget(refreshButton_);

    auto* splitter = new QSplitter(Qt::Vertical);

    deviceTable_ = new QTableWidget(0, 8);
    deviceTable_->setHorizontalHeaderLabels({
        "Device",
        "Manufacturer",
        "Class",
        "Transport",
        "Capabilities",
        "Interfaces",
        "Backend",
        "Status"
    });
    deviceTable_->horizontalHeader()->setSectionResizeMode(
        0,
        QHeaderView::Stretch
    );
    deviceTable_->horizontalHeader()->setSectionResizeMode(
        4,
        QHeaderView::Stretch
    );
    for (int column : {1, 2, 3, 5, 6, 7}) {
        deviceTable_->horizontalHeader()->setSectionResizeMode(
            column,
            QHeaderView::ResizeToContents
        );
    }
    deviceTable_->setSelectionBehavior(
        QAbstractItemView::SelectRows
    );
    deviceTable_->setSelectionMode(
        QAbstractItemView::SingleSelection
    );
    deviceTable_->setEditTriggers(
        QAbstractItemView::NoEditTriggers
    );
    deviceTable_->setAlternatingRowColors(true);

    auto* detailsWidget = new QWidget;
    auto* detailsLayout = new QVBoxLayout(detailsWidget);
    detailsLayout->setContentsMargins(0, 6, 0, 0);
    detailsLayout->setSpacing(8);

    deviceDetails_ = new QLabel(
        "Оберіть пристрій. Двічі клацніть мишу, щоб налаштувати дії кнопок."
    );
    deviceDetails_->setObjectName("diagnosticsPanel");
    deviceDetails_->setTextInteractionFlags(
        Qt::TextSelectableByMouse
    );
    deviceDetails_->setAlignment(
        Qt::AlignTop | Qt::AlignLeft
    );

    interfaceTable_ = new QTableWidget(0, 5);
    interfaceTable_->setHorizontalHeaderLabels({
        "Backend",
        "Connection",
        "Vendor / Product",
        "Serial",
        "System path"
    });
    interfaceTable_->horizontalHeader()->setSectionResizeMode(
        4,
        QHeaderView::Stretch
    );
    for (int column = 0; column < 4; ++column) {
        interfaceTable_->horizontalHeader()->setSectionResizeMode(
            column,
            QHeaderView::ResizeToContents
        );
    }
    interfaceTable_->setEditTriggers(
        QAbstractItemView::NoEditTriggers
    );

    detailsLayout->addWidget(deviceDetails_);
    detailsLayout->addWidget(interfaceTable_);

    splitter->addWidget(deviceTable_);
    splitter->addWidget(detailsWidget);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 2);

    statusLabel_ = new QLabel("Ready.");

    devicesLayout->addLayout(devicesHeader);
    devicesLayout->addWidget(splitter);
    devicesLayout->addWidget(statusLabel_);

    auto* profilesPage = new QWidget;
    auto* profilesLayout = new QVBoxLayout(profilesPage);
    profilesLayout->setContentsMargins(28, 24, 28, 24);
    profilesLayout->setSpacing(16);

    profilesLayout->addWidget(createTitle(""));

    auto* profileContent = new QHBoxLayout;

    profileList_ = new QListWidget;
    profileList_->setMinimumWidth(260);

    for (const auto& profile : application_.profiles().all()) {
        auto* item = new QListWidgetItem(
            QString::fromStdString(profile.displayName)
        );
        item->setData(
            Qt::UserRole,
            QString::fromStdString(profile.id)
        );
        profileList_->addItem(item);
    }

    auto* profileEditor = new QWidget;
    auto* profileForm = new QFormLayout(profileEditor);

    profileNameLabel_ = new QLabel;
    pointerSpeedLabel_ = new QLabel;
    scrollModeLabel_ = new QLabel;
    profileNameEdit_ = new QLineEdit;
    pointerSpeedEdit_ = new QLineEdit;

    scrollModeCombo_ = new QComboBox;
    scrollModeCombo_->addItems({
        "standard",
        "smooth",
        "precise"
    });

    saveProfileButton_ = new QPushButton;

    profileForm->addRow(profileNameLabel_, profileNameEdit_);
    profileForm->addRow(pointerSpeedLabel_, pointerSpeedEdit_);
    profileForm->addRow(scrollModeLabel_, scrollModeCombo_);
    profileForm->addRow("", saveProfileButton_);

    profileContent->addWidget(profileList_, 1);
    profileContent->addWidget(profileEditor, 2);

    profilesLayout->addLayout(profileContent);
    profilesLayout->addStretch();

    // Application-aware shortcuts for terminals and IDEs.
    auto* applicationsPage = new QWidget;
    auto* applicationsLayout = new QVBoxLayout(applicationsPage);
    applicationsLayout->setContentsMargins(28, 24, 28, 24);
    applicationsLayout->setSpacing(14);
    applicationsLayout->addWidget(createTitle("Контекстні профілі програм"));

    auto* applicationsDescription = new QLabel(
        "Rodavarion TDriver визначає активну програму та підставляє відповідні "
        "комбінації. Для терміналів типово використовуються Ctrl+Shift+C/V, "
        "а для IDE — звичайні Ctrl+C/V. Поля можна змінювати."
    );
    applicationsDescription->setWordWrap(true);
    applicationsLayout->addWidget(applicationsDescription);

    applicationProfilesTable_ = new QTableWidget(0, 10);
    applicationProfilesTable_->setHorizontalHeaderLabels({
        "Увімкнено", "Профіль", "Процеси", "Копіювати", "Вставити",
        "Вирізати", "Скасувати", "Повторити", "Виділити все", "ID"
    });
    applicationProfilesTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    applicationProfilesTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    for (int column : {0, 3, 4, 5, 6, 7, 8, 9})
        applicationProfilesTable_->horizontalHeader()->setSectionResizeMode(column, QHeaderView::ResizeToContents);
    applicationProfilesTable_->setAlternatingRowColors(true);

    const auto fillApplicationProfiles = [this](const QVector<context::ApplicationProfile>& profiles) {
        applicationProfilesTable_->setRowCount(0);
        for (const auto& profile : profiles) {
            const int row = applicationProfilesTable_->rowCount();
            applicationProfilesTable_->insertRow(row);
            auto* enabled = new QTableWidgetItem;
            enabled->setFlags((enabled->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
            enabled->setCheckState(profile.enabled ? Qt::Checked : Qt::Unchecked);
            applicationProfilesTable_->setItem(row, 0, enabled);
            applicationProfilesTable_->setItem(row, 1, new QTableWidgetItem(profile.displayName));
            applicationProfilesTable_->setItem(row, 2, new QTableWidgetItem(profile.processPatterns.join(", ")));
            applicationProfilesTable_->setItem(row, 3, new QTableWidgetItem(profile.copyShortcut));
            applicationProfilesTable_->setItem(row, 4, new QTableWidgetItem(profile.pasteShortcut));
            applicationProfilesTable_->setItem(row, 5, new QTableWidgetItem(profile.cutShortcut));
            applicationProfilesTable_->setItem(row, 6, new QTableWidgetItem(profile.undoShortcut));
            applicationProfilesTable_->setItem(row, 7, new QTableWidgetItem(profile.redoShortcut));
            applicationProfilesTable_->setItem(row, 8, new QTableWidgetItem(profile.selectAllShortcut));
            auto* id = new QTableWidgetItem(profile.id);
            id->setFlags(id->flags() & ~Qt::ItemIsEditable);
            applicationProfilesTable_->setItem(row, 9, id);
        }
    };

    const auto loadedApplicationProfiles = context::ApplicationContext::load();
    fillApplicationProfiles(loadedApplicationProfiles ? loadedApplicationProfiles.value() : context::ApplicationContext::defaultProfiles());

    applicationContextStatusLabel_ = new QLabel(context::ApplicationContext::diagnosticReport());
    applicationContextStatusLabel_->setObjectName("diagnosticsPanel");
    applicationContextStatusLabel_->setWordWrap(true);

    auto* applicationButtons = new QHBoxLayout;
    saveApplicationProfilesButton_ = new QPushButton("Зберегти профілі програм");
    resetApplicationProfilesButton_ = new QPushButton("Відновити типові");
    applicationButtons->addWidget(saveApplicationProfilesButton_);
    applicationButtons->addWidget(resetApplicationProfilesButton_);
    applicationButtons->addStretch();

    applicationsLayout->addWidget(applicationProfilesTable_, 1);
    applicationsLayout->addWidget(applicationContextStatusLabel_);
    applicationsLayout->addLayout(applicationButtons);

    connect(saveApplicationProfilesButton_, &QPushButton::clicked, this, [this]() {
        QVector<context::ApplicationProfile> profiles;
        for (int row = 0; row < applicationProfilesTable_->rowCount(); ++row) {
            const auto text = [this, row](int column) {
                const auto* item = applicationProfilesTable_->item(row, column);
                return item == nullptr ? QString() : item->text().trimmed();
            };
            context::ApplicationProfile profile;
            profile.enabled = applicationProfilesTable_->item(row, 0) != nullptr
                && applicationProfilesTable_->item(row, 0)->checkState() == Qt::Checked;
            profile.displayName = text(1);
            profile.processPatterns = text(2).split(',', Qt::SkipEmptyParts);
            for (auto& pattern : profile.processPatterns) pattern = pattern.trimmed().toLower();
            profile.copyShortcut = text(3); profile.pasteShortcut = text(4);
            profile.cutShortcut = text(5); profile.undoShortcut = text(6);
            profile.redoShortcut = text(7); profile.selectAllShortcut = text(8);
            profile.id = text(9);
            profiles.append(profile);
        }
        const auto result = context::ApplicationContext::save(profiles);
        applicationContextStatusLabel_->setText(result
            ? "Профілі програм збережено.\n" + context::ApplicationContext::diagnosticReport()
            : "Помилка: " + QString::fromStdString(result.error()));
    });

    connect(resetApplicationProfilesButton_, &QPushButton::clicked, this, [fillApplicationProfiles]() {
        fillApplicationProfiles(context::ApplicationContext::defaultProfiles());
    });

    // General startup and session behaviour.
    auto* generalPage = new QWidget;
    auto* generalLayout = new QVBoxLayout(generalPage);
    generalLayout->setContentsMargins(32, 28, 32, 28);
    generalLayout->setSpacing(14);
    generalLayout->addWidget(createTitle("Загальні налаштування"));

    auto* generalDescription = new QLabel(
        "Налаштуйте автоматичний запуск і відновлення стану Rodavarion TDriver. "
        "За замовчуванням програма запускається після входу в систему лише у системному треї."
    );
    generalDescription->setWordWrap(true);
    generalLayout->addWidget(generalDescription);

    QSettings startupSettings;
    const QString autostartFile = QStandardPaths::writableLocation(
        QStandardPaths::ConfigLocation
    ) + "/autostart/rodavarion-tdriver.desktop";

    autostartCheckBox_ = new QCheckBox("Запускати Rodavarion TDriver після входу в систему");
    autostartCheckBox_->setChecked(QFile::exists(autostartFile));
    startInTrayCheckBox_ = new QCheckBox("Запускати лише у системному треї");
    startInTrayCheckBox_->setChecked(startupSettings.value("startup/startInTray", true).toBool());
    rememberGeometryCheckBox_ = new QCheckBox("Запам’ятовувати розмір і положення вікна");
    rememberGeometryCheckBox_->setChecked(startupSettings.value("startup/rememberGeometry", true).toBool());
    rememberLastPageCheckBox_ = new QCheckBox("Запам’ятовувати останню відкриту вкладку");
    rememberLastPageCheckBox_->setChecked(startupSettings.value("startup/rememberLastPage", true).toBool());
    restoreProfileCheckBox_ = new QCheckBox("Відновлювати останній активний профіль");
    restoreProfileCheckBox_->setChecked(startupSettings.value("startup/restoreProfile", true).toBool());

    saveGeneralSettingsButton_ = new QPushButton("Зберегти загальні налаштування");
    generalSettingsStatusLabel_ = new QLabel("Налаштування завантажено.");
    generalSettingsStatusLabel_->setObjectName("diagnosticsPanel");
    generalSettingsStatusLabel_->setWordWrap(true);

    generalLayout->addWidget(autostartCheckBox_);
    generalLayout->addWidget(startInTrayCheckBox_);
    generalLayout->addWidget(rememberGeometryCheckBox_);
    generalLayout->addWidget(rememberLastPageCheckBox_);
    generalLayout->addWidget(restoreProfileCheckBox_);
    generalLayout->addSpacing(8);
    generalLayout->addWidget(saveGeneralSettingsButton_);
    generalLayout->addWidget(generalSettingsStatusLabel_);
    generalLayout->addStretch();

    connect(saveGeneralSettingsButton_, &QPushButton::clicked, this, &MainWindow::saveGeneralSettings);

    auto* diagnosticsPage = new QWidget;
    auto* diagnosticsLayout = new QVBoxLayout(
        diagnosticsPage
    );
    diagnosticsLayout->setContentsMargins(
        28,
        24,
        28,
        24
    );
    diagnosticsLayout->setSpacing(16);

    diagnosticsLayout->addWidget(createTitle(""));

    diagnosticsLabel_ = new QLabel(
        "Core: initialized\n"
        "GUI: Qt 6 Widgets\n"
        "Peripheral model: universal\n"
        "Current backend: HIDAPI + mock provider\n"
        "Planned backends: CUPS, SANE, evdev, serial, network\n"
        "Physical grouping: enabled\n"
        "Capability classifier: enabled\n"
        "Transport classifier: enabled\n"
        "Persistent profiles: enabled\n"
        "Plugin API: v1\n\n"
        + system::LinuxDiagnostics::report()
    );
    diagnosticsLabel_->setObjectName(
        "diagnosticsPanel"
    );
    diagnosticsLabel_->setTextInteractionFlags(
        Qt::TextSelectableByMouse
    );

    diagnosticsLayout->addWidget(diagnosticsLabel_);
    diagnosticsLayout->addStretch();


    // Information / Open Source page.
    auto* aboutPage = new QWidget;
    auto* aboutOuterLayout = new QVBoxLayout(aboutPage);
    aboutOuterLayout->setContentsMargins(0, 0, 0, 0);

    auto* aboutScroll = new QScrollArea(aboutPage);
    aboutScroll->setWidgetResizable(true);
    aboutScroll->setFrameShape(QFrame::NoFrame);

    auto* aboutContent = new QWidget;
    auto* aboutLayout = new QVBoxLayout(aboutContent);
    aboutLayout->setContentsMargins(42, 34, 42, 34);
    aboutLayout->setSpacing(18);

    aboutTitleLabel_ = createTitle("");
    aboutCompanyLabel_ = new QLabel;
    aboutCompanyLabel_->setObjectName("aboutCompany");

    aboutMottoLabel_ = new QLabel;
    aboutMottoLabel_->setObjectName("aboutMotto");
    aboutMottoLabel_->setWordWrap(true);

    aboutDescriptionLabel_ = new QLabel;
    aboutDescriptionLabel_->setObjectName("aboutDescription");
    aboutDescriptionLabel_->setWordWrap(true);
    aboutDescriptionLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    aboutVersionLabel_ = new QLabel;
    aboutVersionLabel_->setObjectName("aboutVersion");
    aboutVersionLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto* aboutCard = new QWidget;
    aboutCard->setObjectName("aboutCard");
    auto* aboutCardLayout = new QVBoxLayout(aboutCard);
    aboutCardLayout->setContentsMargins(28, 26, 28, 26);
    aboutCardLayout->setSpacing(16);
    aboutCardLayout->addWidget(aboutCompanyLabel_);
    aboutCardLayout->addWidget(aboutMottoLabel_);
    aboutCardLayout->addSpacing(8);
    aboutCardLayout->addWidget(aboutDescriptionLabel_);
    aboutCardLayout->addSpacing(8);
    aboutCardLayout->addWidget(aboutVersionLabel_);

    auto* helpCard = new QWidget;
    helpCard->setObjectName("aboutCard");
    auto* helpLayout = new QVBoxLayout(helpCard);
    helpLayout->setContentsMargins(28, 24, 28, 24);
    helpLayout->setSpacing(12);
    auto* helpTitle = new QLabel("🤝 Як допомогти проєкту");
    helpTitle->setObjectName("aboutCompany");
    auto* helpText = new QLabel(
        "⭐ Поставити зірку на GitHub   •   🐞 Повідомити про помилку   •   "
        "💡 Запропонувати ідею   •   🛠️ Долучитися до розробки   •   "
        "☕ Підтримати розвиток"
    );
    helpText->setWordWrap(true);
    auto* githubButton = new QPushButton("⭐ GitHub — після публікації репозиторію");
    githubButton->setToolTip("Кнопка буде активована після створення офіційного репозиторію Rodavarion TDriver.");
    githubButton->setEnabled(false);
    helpLayout->addWidget(helpTitle);
    helpLayout->addWidget(helpText);
    helpLayout->addWidget(githubButton, 0, Qt::AlignLeft);

    auto* donationCard = new QWidget;
    donationCard->setObjectName("aboutCard");
    auto* donationLayout = new QVBoxLayout(donationCard);
    donationLayout->setContentsMargins(28, 24, 28, 24);
    donationLayout->setSpacing(12);
    auto* donationTitle = new QLabel("☕ Підтримати Rodavarion Technologies");
    donationTitle->setObjectName("aboutCompany");
    auto* donationText = new QLabel(
        "<b>RT Open Source</b><br>"
        "На підтримку розвитку Open Source ПЗ Rodavarion Technologies.<br><br>"
        "Rodavarion TDriver створюється ветераном-ентузіастом. "
        "Rodavarion Technologies перебуває на етапі створення. "
        "Добровільна підтримка допомагає продовжувати розробку, тестування та документацію."
    );
    donationText->setWordWrap(true);
    donationText->setTextFormat(Qt::RichText);
    auto* bankButton = new QPushButton("🔗 Відкрити Банку Monobank");
    connect(bankButton, &QPushButton::clicked, this, [this]() {
        openExternalUrl("https://send.monobank.ua/jar/3CuXhBgmhr");
    });
    auto* cardRow = new QHBoxLayout;
    auto* cardLabel = new QLabel("Картка Банки: 4874 1000 3970 6000");
    cardLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    auto* copyCardButton = new QPushButton("📋 Скопіювати номер");
    connect(copyCardButton, &QPushButton::clicked, this, &MainWindow::copyDonationCard);
    cardRow->addWidget(cardLabel);
    cardRow->addStretch();
    cardRow->addWidget(copyCardButton);
    supportStatusLabel_ = new QLabel("Підтримка є добровільною. Дякуємо кожному, хто допомагає проєкту ❤️");
    supportStatusLabel_->setWordWrap(true);
    supportStatusLabel_->setObjectName("supportStatus");
    donationLayout->addWidget(donationTitle);
    donationLayout->addWidget(donationText);
    donationLayout->addWidget(bankButton, 0, Qt::AlignLeft);
    donationLayout->addLayout(cardRow);
    donationLayout->addWidget(supportStatusLabel_);

    auto* thanksLabel = new QLabel(
        "Дякуємо кожному, хто повідомляє про помилки, пропонує ідеї, "
        "допомагає кодом або підтримує розвиток проєкту. Саме завдяки "
        "таким людям Open Source стає кращим. ❤️"
    );
    thanksLabel->setWordWrap(true);
    thanksLabel->setAlignment(Qt::AlignCenter);

    aboutLayout->addWidget(aboutTitleLabel_);
    aboutLayout->addWidget(aboutCard);
    aboutLayout->addWidget(helpCard);
    aboutLayout->addWidget(donationCard);
    aboutLayout->addWidget(thanksLabel);
    aboutLayout->addStretch();
    aboutScroll->setWidget(aboutContent);
    aboutOuterLayout->addWidget(aboutScroll);


    auto* maintenancePage = new QWidget;
    auto* maintenanceLayout = new QVBoxLayout(maintenancePage);
    maintenanceLayout->setContentsMargins(32, 28, 32, 28);
    maintenanceLayout->setSpacing(14);

    maintenanceTitleLabel_ = createTitle("");
    maintenanceDescriptionLabel_ = new QLabel;
    maintenanceDescriptionLabel_->setWordWrap(true);

    maintenanceStatusLabel_ = new QLabel;
    maintenanceStatusLabel_->setObjectName("diagnosticsPanel");
    maintenanceStatusLabel_->setWordWrap(true);
    maintenanceStatusLabel_->setTextInteractionFlags(
        Qt::TextSelectableByMouse
    );

    maintenanceOperationLabel_ = new QLabel(
        "Готово до виконання операцій."
    );
    maintenanceOperationLabel_->setWordWrap(true);
    maintenanceOperationLabel_->setObjectName(
        "maintenanceOperationStatus"
    );
    maintenanceOperationLabel_->setStyleSheet(
        "QLabel#maintenanceOperationStatus {"
        " padding: 10px 12px;"
        " border-radius: 7px;"
        " background: rgba(255,255,255,0.06);"
        "}"
    );

    maintenanceProgressBar_ = new QProgressBar;
    maintenanceProgressBar_->setRange(0, 0);
    maintenanceProgressBar_->setTextVisible(false);
    maintenanceProgressBar_->setVisible(false);
    maintenanceProgressBar_->setMaximumHeight(8);

    maintenanceLastRunLabel_ = new QLabel(
        "Остання операція: ще не виконувалась"
    );
    maintenanceLastRunLabel_->setStyleSheet(
        "color: palette(mid); font-size: 11px;"
    );

    auto* maintenanceLists = new QHBoxLayout;

    auto* missingBox = new QVBoxLayout;
    auto* missingTitle = new QLabel(
        "<b>Потрібно довстановити</b>"
    );
    maintenanceMissingList_ = new QListWidget;
    missingBox->addWidget(missingTitle);
    missingBox->addWidget(maintenanceMissingList_);

    auto* removableBox = new QVBoxLayout;
    auto* removableTitle = new QLabel(
        "<b>Безпечно можна видалити</b>"
    );
    maintenanceRemovableList_ = new QListWidget;
    removableBox->addWidget(removableTitle);
    removableBox->addWidget(maintenanceRemovableList_);

    maintenanceLists->addLayout(missingBox, 1);
    maintenanceLists->addLayout(removableBox, 1);

    auto* maintenanceButtons = new QHBoxLayout;
    maintenanceRefreshButton_ = new QPushButton(
        "Перевірити залежності"
    );
    maintenanceInstallButton_ = new QPushButton(
        "Довстановити потрібне"
    );
    maintenanceCleanupButton_ = new QPushButton(
        "Видалити безпечний баласт"
    );

    maintenanceButtons->addWidget(
        maintenanceRefreshButton_
    );
    maintenanceButtons->addWidget(
        maintenanceInstallButton_
    );
    maintenanceButtons->addWidget(
        maintenanceCleanupButton_
    );
    maintenanceButtons->addStretch();

    maintenanceLayout->addWidget(
        maintenanceTitleLabel_
    );
    maintenanceLayout->addWidget(
        maintenanceDescriptionLabel_
    );
    maintenanceLayout->addWidget(
        maintenanceStatusLabel_
    );
    maintenanceLayout->addWidget(
        maintenanceOperationLabel_
    );
    maintenanceLayout->addWidget(
        maintenanceProgressBar_
    );
    maintenanceLayout->addWidget(
        maintenanceLastRunLabel_
    );
    maintenanceLayout->addLayout(
        maintenanceLists,
        1
    );
    maintenanceLayout->addLayout(
        maintenanceButtons
    );

    pages_->addWidget(devicesPage);
    pages_->addWidget(profilesPage);
    pages_->addWidget(applicationsPage);
    pages_->addWidget(generalPage);
    pages_->addWidget(diagnosticsPage);
    pages_->addWidget(maintenancePage);
    pages_->addWidget(aboutPage);

    rootLayout->addWidget(navigation_);
    rootLayout->addWidget(pages_, 1);

    setCentralWidget(central);

    connect(
        navigation_,
        &QListWidget::currentRowChanged,
        pages_,
        &QStackedWidget::setCurrentIndex
    );
    connect(
        refreshButton_,
        &QPushButton::clicked,
        this,
        &MainWindow::refreshDevices
    );
    connect(
        deviceTable_,
        &QTableWidget::itemSelectionChanged,
        this,
        &MainWindow::showSelectedDeviceDetails
    );
    connect(
        deviceTable_,
        &QTableWidget::cellDoubleClicked,
        this,
        [this](int, int) {
            openSelectedPeripheral();
        }
    );
    connect(
        profileList_,
        &QListWidget::currentRowChanged,
        this,
        &MainWindow::loadSelectedProfile
    );
    connect(
        saveProfileButton_,
        &QPushButton::clicked,
        this,
        &MainWindow::saveSelectedProfile
    );
    connect(
        maintenanceRefreshButton_,
        &QPushButton::clicked,
        this,
        &MainWindow::refreshDependencyMaintenance
    );
    connect(
        maintenanceInstallButton_,
        &QPushButton::clicked,
        this,
        &MainWindow::installMissingDependencies
    );
    connect(
        maintenanceCleanupButton_,
        &QPushButton::clicked,
        this,
        &MainWindow::removeUnusedDependencies
    );

    refreshDependencyMaintenance();
}


void MainWindow::openExternalUrl(const QString& url) {
    const bool opened = QDesktopServices::openUrl(QUrl(url));
    if (supportStatusLabel_ != nullptr) {
        supportStatusLabel_->setText(
            opened
                ? "✓ Посилання передано системному браузеру."
                : "Не вдалося відкрити посилання. Скопіюйте його вручну."
        );
    }
}

void MainWindow::copyDonationCard() {
    constexpr auto cardNumber = "4874100039706000";
    QApplication::clipboard()->setText(cardNumber);
    if (supportStatusLabel_ != nullptr) {
        supportStatusLabel_->setText("✓ Номер картки Банки скопійовано.");
    }
    if (trayIcon_ != nullptr && trayIcon_->isVisible()) {
        trayIcon_->showMessage(
            "Rodavarion TDriver",
            "Номер картки Банки скопійовано.",
            QSystemTrayIcon::Information,
            2500
        );
    }
}

void MainWindow::showWelcomeDialogIfNeeded() {
    QSettings settings;
    if (settings.value("welcome/openSourceBeta8Shown", false).toBool()) {
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Ласкаво просимо до Rodavarion TDriver");
    dialog.setModal(true);
    dialog.setMinimumWidth(560);

    auto* layout = new QVBoxLayout(&dialog);
    auto* title = new QLabel("Rodavarion TDriver — Open Source");
    title->setObjectName("pageTitle");
    auto* text = new QLabel(
        "Дякуємо, що встановили Rodavarion TDriver.\n\n"
        "Це відкритий проєкт, який створюється ветераном-ентузіастом. "
        "Ви можете допомогти тестуванням, повідомленнями про помилки, "
        "новими ідеями, внесками до коду або добровільною підтримкою."
    );
    text->setWordWrap(true);
    auto* doNotShow = new QCheckBox("Більше не показувати це вітання");
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    buttons->button(QDialogButtonBox::Ok)->setText("Почати роботу");
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

    layout->addWidget(title);
    layout->addWidget(text);
    layout->addWidget(doNotShow);
    layout->addWidget(buttons);

    dialog.exec();
    if (doNotShow->isChecked()) {
        settings.setValue("welcome/openSourceBeta8Shown", true);
        settings.sync();
    }
}


void MainWindow::buildLanguageMenu() {
    auto& localizer = i18n::LocalizationService::instance();

    settingsMenu_ = menuBar()->addMenu(QString());
    languageMenu_ = settingsMenu_->addMenu(QString());
    languageGroup_ = new QActionGroup(this);
    languageGroup_->setExclusive(true);

    const auto addLanguage = [this, &localizer](
        const i18n::Language language
    ) {
        auto* action = languageMenu_->addAction(
            localizer.languageName(language)
        );
        action->setCheckable(true);
        action->setData(static_cast<int>(language));
        languageGroup_->addAction(action);

        if (localizer.configuredLanguage() == language) {
            action->setChecked(true);
        }

        connect(action, &QAction::triggered, this, [this, language]() {
            i18n::LocalizationService::instance().setLanguage(language);
            applyLanguage();
        });
    };

    addLanguage(i18n::Language::Automatic);
    addLanguage(i18n::Language::Ukrainian);
    addLanguage(i18n::Language::English);
}

void MainWindow::applyLanguage() {
    auto& t = i18n::LocalizationService::instance();

    setWindowTitle(t.text("app.title"));
    navigation_->item(0)->setText(t.text("nav.peripherals"));
    navigation_->item(1)->setText(t.text("nav.profiles"));
    navigation_->item(2)->setText("Програми та IDE");
    navigation_->item(3)->setText("Загальні");
    navigation_->item(4)->setText(t.text("nav.diagnostics"));
    navigation_->item(5)->setText(t.text("nav.maintenance"));
    navigation_->item(6)->setText(t.text("nav.about"));

    refreshButton_->setText(t.text("button.scan"));
    saveProfileButton_->setText(t.text("profile.save"));
    profileNameLabel_->setText(t.text("profile.name"));
    pointerSpeedLabel_->setText(t.text("profile.pointer_speed"));
    scrollModeLabel_->setText(t.text("profile.scroll_mode"));

    deviceTable_->setHorizontalHeaderLabels({
        t.text("table.device"),
        t.text("table.manufacturer"),
        t.text("table.class"),
        t.text("table.transport"),
        t.text("table.capabilities"),
        t.text("table.interfaces"),
        t.text("table.backend"),
        t.text("table.status")
    });

    interfaceTable_->setHorizontalHeaderLabels({
        t.text("table.backend"),
        t.text("table.connection"),
        t.text("table.vendor_product"),
        t.text("table.serial"),
        t.text("table.system_path")
    });

    const auto titles = findChildren<QLabel*>("pageTitle");
    if (titles.size() >= 5) {
        titles[0]->setText(t.text("page.peripherals"));
        titles[1]->setText(t.text("page.profiles"));
        titles[2]->setText("Контекстні профілі програм");
        titles[3]->setText("Загальні налаштування");
        titles[4]->setText(t.text("page.diagnostics"));
    }

    settingsMenu_->setTitle(t.text("menu.settings"));
    languageMenu_->setTitle(t.text("menu.language"));

    const auto actions = languageGroup_->actions();
    for (auto* action : actions) {
        const auto language = static_cast<i18n::Language>(
            action->data().toInt()
        );
        action->setText(t.languageName(language));
        action->setChecked(t.configuredLanguage() == language);
    }

    diagnosticsLabel_->setText(
        t.text("diagnostics.summary")
        + system::LinuxDiagnostics::report()
        + "\n\nAction backend:\n"
        + runtime::DesktopActionExecutor::diagnosticReport()
        + "\n\nApplication context:\n"
        + context::ApplicationContext::diagnosticReport()
    );

    if (maintenanceTitleLabel_ != nullptr) {
        maintenanceTitleLabel_->setText(
            t.text("maintenance.title")
        );
    }
    if (maintenanceDescriptionLabel_ != nullptr) {
        maintenanceDescriptionLabel_->setText(
            t.text("maintenance.description")
        );
    }

    if (aboutTitleLabel_ != nullptr) {
        aboutTitleLabel_->setText(t.text("about.title"));
    }
    if (aboutCompanyLabel_ != nullptr) {
        aboutCompanyLabel_->setText(t.text("about.company"));
    }
    if (aboutMottoLabel_ != nullptr) {
        aboutMottoLabel_->setText(t.text("about.motto"));
    }
    if (aboutDescriptionLabel_ != nullptr) {
        aboutDescriptionLabel_->setText(t.text("about.description"));
    }
    if (aboutVersionLabel_ != nullptr) {
        aboutVersionLabel_->setText(
            t.text("about.version").arg(QApplication::applicationVersion())
        );
    }
}

void MainWindow::applyTheme() {
    qApp->setStyleSheet(R"(
        QMainWindow, QWidget {
            background: #0b1728;
            color: #e8eef7;
            font-family: "Inter", "Noto Sans", "Segoe UI", sans-serif;
            font-size: 14px;
        }

        QListWidget#navigation {
            background: #08111f;
            border: none;
            padding: 18px 10px;
            font-size: 15px;
        }

        QListWidget#navigation::item {
            padding: 13px 14px;
            margin: 3px 0;
            border-radius: 8px;
        }

        QListWidget#navigation::item:selected {
            background: #145f93;
            color: white;
        }

        QLabel#pageTitle {
            font-size: 24px;
            font-weight: 700;
            color: #f3ca62;
        }

        QPushButton {
            background: #166fa8;
            color: white;
            border: none;
            border-radius: 7px;
            padding: 9px 16px;
            font-weight: 600;
        }

        QPushButton:hover {
            background: #2185c5;
        }

        QLineEdit, QComboBox {
            background: #101f33;
            border: 1px solid #314865;
            border-radius: 7px;
            padding: 8px;
            min-height: 22px;
        }

        QTableWidget, QListWidget {
            background: #101f33;
            alternate-background-color: #0d1a2b;
            border: 1px solid #263b55;
            border-radius: 8px;
            gridline-color: #263b55;
            selection-background-color: #145f93;
        }

        QHeaderView::section {
            background: #14263d;
            color: #dfe9f5;
            border: none;
            border-bottom: 1px solid #314865;
            padding: 9px;
            font-weight: 600;
        }

        QLabel#diagnosticsPanel {
            background: #101f33;
            border: 1px solid #263b55;
            border-radius: 8px;
            padding: 16px;
            font-family: "JetBrains Mono", "Noto Sans Mono", monospace;
        }

        QSplitter::handle {
            background: #263b55;
            height: 3px;
        }

        QWidget#aboutCard {
            background: #101f33;
            border: 1px solid #314865;
            border-radius: 12px;
        }

        QLabel#aboutCompany {
            color: #f3ca62;
            font-size: 24px;
            font-weight: 700;
        }

        QLabel#aboutMotto {
            color: #d8c17c;
            font-size: 16px;
            font-style: italic;
        }

        QLabel#aboutDescription {
            color: #e8eef7;
            font-size: 15px;
            line-height: 1.45;
        }

        QLabel#aboutVersion {
            color: #9eb2c7;
            font-size: 13px;
        }
    )");
}

void MainWindow::createTrayIcon() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    const auto icon = createRodavarionIcon();
    setWindowIcon(icon);

    trayIcon_ = new QSystemTrayIcon(icon, this);
    trayIcon_->setToolTip("Rodavarion TDriver");

    trayMenu_ = new QMenu(this);
    auto* showAction = trayMenu_->addAction("Відкрити Rodavarion TDriver");
    auto* hideAction = trayMenu_->addAction("Сховати вікно");
    trayMenu_->addSeparator();
    auto* restartAction = trayMenu_->addAction("Перезапустити обробку миші");
    auto* statusAction = trayMenu_->addAction("Показати стан");
    trayMenu_->addSeparator();
    auto* quitAction = trayMenu_->addAction("Вийти з програми");

    connect(showAction, &QAction::triggered, this, [this]() {
        showFromTray();
    });
    connect(hideAction, &QAction::triggered, this, [this]() {
        hide();
    });
    connect(restartAction, &QAction::triggered, this, [this]() {
        restartMouseRuntime();
        trayIcon_->showMessage(
            "Rodavarion TDriver",
            mouseRuntime_ ? mouseRuntime_->statusText() : "Обробку миші перезапущено.",
            QSystemTrayIcon::Information,
            3500
        );
    });
    connect(statusAction, &QAction::triggered, this, [this]() {
        trayIcon_->showMessage(
            "Rodavarion TDriver — стан",
            mouseRuntime_ ? mouseRuntime_->statusText() : "Виконавчий модуль ще не створено.",
            QSystemTrayIcon::Information,
            5000
        );
    });
    connect(quitAction, &QAction::triggered, this, [this]() {
        quitApplication();
    });

    trayIcon_->setContextMenu(trayMenu_);
    connect(
        trayIcon_,
        &QSystemTrayIcon::activated,
        this,
        [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger
                || reason == QSystemTrayIcon::DoubleClick) {
                isVisible() ? hide() : showFromTray();
            }
        }
    );

    trayIcon_->show();
}

void MainWindow::showFromTray() {
    showNormal();
    raise();
    activateWindow();
}

void MainWindow::quitApplication() {
    quitting_ = true;
    if (trayIcon_ != nullptr) {
        trayIcon_->hide();
    }
    close();
    qApp->quit();
}

void MainWindow::restoreWindowState() {
    QSettings settings;
    if (settings.value("startup/rememberGeometry", true).toBool()) {
        restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
        restoreState(settings.value("mainWindow/state").toByteArray());
    }
    if (settings.value("startup/rememberLastPage", true).toBool()) {
        const int page = settings.value("mainWindow/currentPage", 0).toInt();
        if (page >= 0 && page < navigation_->count()) {
            navigation_->setCurrentRow(page);
        }
    }
}

void MainWindow::updateAutostartEntry(const bool enabled) {
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    const QString autostartDir = configDir + "/autostart";
    const QString path = autostartDir + "/rodavarion-tdriver.desktop";

    if (!enabled) {
        if (QFile::exists(path) && !QFile::remove(path)) {
            throw std::runtime_error("Не вдалося видалити файл автозапуску.");
        }
        return;
    }

    QDir().mkpath(autostartDir);
    const QString executable = QStandardPaths::findExecutable("rodavarion-tdriver");
    const QString resolvedExecutable = executable.isEmpty()
        ? QDir::homePath() + "/.local/bin/rodavarion-tdriver"
        : executable;
    const QString icon = QDir::homePath()
        + "/.local/share/rodavarion-tdriver/resources/icons/rodavarion-tdriver.png";

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        throw std::runtime_error("Не вдалося створити файл автозапуску.");
    }
    QTextStream stream(&file);
    stream << "[Desktop Entry]\n"
           << "Type=Application\n"
           << "Version=1.0\n"
           << "Name=Rodavarion TDriver\n"
           << "Comment=Автоматичний запуск менеджера кнопок миші\n"
           << "Exec=" << resolvedExecutable << " --tray\n"
           << "TryExec=" << resolvedExecutable << "\n"
           << "Icon=" << icon << "\n"
           << "Terminal=false\n"
           << "Categories=Settings;HardwareSettings;\n"
           << "X-GNOME-Autostart-enabled=true\n"
           << "X-KDE-autostart-after=panel\n";
    file.close();
}

void MainWindow::saveGeneralSettings() {
    try {
        QSettings settings;
        settings.setValue("startup/startInTray", startInTrayCheckBox_->isChecked());
        settings.setValue("startup/rememberGeometry", rememberGeometryCheckBox_->isChecked());
        settings.setValue("startup/rememberLastPage", rememberLastPageCheckBox_->isChecked());
        settings.setValue("startup/restoreProfile", restoreProfileCheckBox_->isChecked());
        updateAutostartEntry(autostartCheckBox_->isChecked());
        settings.sync();
        generalSettingsStatusLabel_->setText(
            "✓ Налаштування збережено. Зміни автозапуску набудуть чинності під час наступного входу в систему."
        );
    } catch (const std::exception& error) {
        generalSettingsStatusLabel_->setText(
            "Помилка збереження: " + QString::fromUtf8(error.what())
        );
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    QSettings settings;
    if (settings.value("startup/rememberGeometry", true).toBool()) {
        settings.setValue("mainWindow/geometry", saveGeometry());
        settings.setValue("mainWindow/state", saveState());
    }
    if (settings.value("startup/rememberLastPage", true).toBool()) {
        settings.setValue("mainWindow/currentPage", navigation_->currentRow());
    }

    if (!quitting_ && trayIcon_ != nullptr && trayIcon_->isVisible()) {
        hide();
        event->ignore();

        if (!trayHintShown_) {
            trayHintShown_ = true;
            trayIcon_->showMessage(
                "Rodavarion TDriver працює у фоні",
                "Вікно приховано. Натисніть значок у треї, щоб відкрити програму, або скористайтеся меню правої кнопки миші.",
                QSystemTrayIcon::Information,
                5000
            );
        }
        return;
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::refreshDevices() {
    peripherals_ = application_.scanPeripherals();

    deviceTable_->setRowCount(
        static_cast<int>(peripherals_.size())
    );

    for (int row = 0;
         row < static_cast<int>(peripherals_.size());
         ++row) {
        const auto& peripheral =
            peripherals_[static_cast<std::size_t>(row)];
        const auto& device = peripheral.physicalDevice;

        QStringList backends;
        for (const auto& interfaceInfo : device.interfaces) {
            const auto backend = QString::fromStdString(
                interfaceInfo.backendName
            );
            if (!backends.contains(backend)) {
                backends.append(backend);
            }
        }

        deviceTable_->setItem(
            row, 0,
            new QTableWidgetItem(
                QString::fromStdString(device.productName)
            )
        );
        deviceTable_->setItem(
            row, 1,
            new QTableWidgetItem(
                QString::fromStdString(device.manufacturer)
            )
        );
        deviceTable_->setItem(
            row, 2,
            new QTableWidgetItem(
                QString::fromUtf8(
                    capability::deviceClassName(
                        peripheral.classification.deviceClass
                    ).data()
                )
            )
        );
        deviceTable_->setItem(
            row, 3,
            new QTableWidgetItem(
                QString::fromUtf8(
                    transport::transportName(
                        peripheral.transport
                    ).data()
                )
            )
        );
        deviceTable_->setItem(
            row, 4,
            new QTableWidgetItem(
                capabilityList(peripheral.classification)
            )
        );
        deviceTable_->setItem(
            row, 5,
            new QTableWidgetItem(
                QString::number(device.interfaceCount())
            )
        );
        deviceTable_->setItem(
            row, 6,
            new QTableWidgetItem(backends.join(", "))
        );
        deviceTable_->setItem(
            row, 7,
            new QTableWidgetItem(
                device.hasRealBackend()
                    ? (i18n::LocalizationService::instance().effectiveLanguage() == i18n::Language::Ukrainian ? "Виявлено" : "Detected")
                    : (i18n::LocalizationService::instance().effectiveLanguage() == i18n::Language::Ukrainian ? "Тестовий пристрій" : "Test device")
            )
        );
    }

    if (!peripherals_.empty()) {
        deviceTable_->selectRow(0);
    } else {
        deviceDetails_->setText(
            i18n::LocalizationService::instance().effectiveLanguage() == i18n::Language::Ukrainian
                ? "Периферійних пристроїв не виявлено."
                : "No peripherals were detected."
        );
        interfaceTable_->setRowCount(0);
    }

    statusLabel_->setText(
        i18n::LocalizationService::instance().text("status.peripherals_found")
            .arg(peripherals_.size())
    );

    setup::DependencyMaintenanceManager::
        recordDeviceRequirements(peripherals_);

    if (maintenanceStatusLabel_ != nullptr) {
        refreshDependencyMaintenance();
    }
}

void MainWindow::showSelectedDeviceDetails() {
    const int row = deviceTable_->currentRow();

    if (row < 0
        || row >= static_cast<int>(peripherals_.size())) {
        interfaceTable_->setRowCount(0);
        return;
    }

    const auto& peripheral =
        peripherals_[static_cast<std::size_t>(row)];
    const auto& device = peripheral.physicalDevice;

    deviceDetails_->setText(
        QString(
            "Product: %1\n"
            "Manufacturer: %2\n"
            "Class: %3\n"
            "Transport: %4\n"
            "Capabilities: %5\n"
            "Interfaces: %6\n\n"
            "Double-click a mouse to configure button actions. "
            "Mappings are stored safely, but physical interception is not enabled yet."
        )
        .arg(QString::fromStdString(device.productName))
        .arg(QString::fromStdString(device.manufacturer))
        .arg(
            QString::fromUtf8(
                capability::deviceClassName(
                    peripheral.classification.deviceClass
                ).data()
            )
        )
        .arg(
            QString::fromUtf8(
                transport::transportName(
                    peripheral.transport
                ).data()
            )
        )
        .arg(capabilityList(peripheral.classification))
        .arg(device.interfaceCount())
    );

    interfaceTable_->setRowCount(
        static_cast<int>(device.interfaces.size())
    );

    for (int index = 0;
         index < static_cast<int>(device.interfaces.size());
         ++index) {
        const auto& interfaceInfo =
            device.interfaces[
                static_cast<std::size_t>(index)
            ];

        const auto ids = QString("%1 / %2")
            .arg(
                interfaceInfo.id.vendorId,
                4,
                16,
                QLatin1Char('0')
            )
            .arg(
                interfaceInfo.id.productId,
                4,
                16,
                QLatin1Char('0')
            )
            .toUpper();

        const auto serial =
            interfaceInfo.id.serialNumber.empty()
                ? QString("(not available)")
                : QString::fromStdString(
                    interfaceInfo.id.serialNumber
                );

        interfaceTable_->setItem(
            index, 0,
            new QTableWidgetItem(
                QString::fromStdString(
                    interfaceInfo.backendName
                )
            )
        );
        interfaceTable_->setItem(
            index, 1,
            new QTableWidgetItem(
                connectionName(
                    interfaceInfo.connection
                )
            )
        );
        interfaceTable_->setItem(
            index, 2,
            new QTableWidgetItem(ids)
        );
        interfaceTable_->setItem(
            index, 3,
            new QTableWidgetItem(serial)
        );
        interfaceTable_->setItem(
            index, 4,
            new QTableWidgetItem(
                QString::fromStdString(
                    interfaceInfo.systemPath
                )
            )
        );
    }
}


void MainWindow::openSelectedPeripheral() {
    const int row = deviceTable_->currentRow();

    if (row < 0
        || row >= static_cast<int>(peripherals_.size())) {
        return;
    }

    const auto& peripheral =
        peripherals_[static_cast<std::size_t>(row)];

    if (peripheral.classification.deviceClass
        != capability::DeviceClass::Mouse) {
        statusLabel_->setText(
            i18n::LocalizationService::instance().text("status.mouse_only")
        );
        return;
    }

    ensureActionBackend(true);

    // Only one process may own an EVIOCGRAB on the mouse interface. The
    // background daemon normally owns it, so stop the daemon before opening
    // the read-only live monitor. This fixes the monitor appearing dead while
    // the installed service is active. The service is restarted afterwards.
    QProcess serviceControl;
    serviceControl.start("systemctl", {"--user", "stop", "rodavarion-tdriverd.service"});
    serviceControl.waitForFinished(3000);

    if (mouseRuntime_ != nullptr) {
        mouseRuntime_->stop();
    }

    MouseActionDialog dialog(peripheral, this);
    dialog.exec();

    serviceControl.start("systemctl", {"--user", "restart", "rodavarion-tdriverd.service"});
    serviceControl.waitForFinished(4000);
    restartMouseRuntime();

    statusLabel_->setText(
        mouseRuntime_ != nullptr
            ? mouseRuntime_->statusText()
            : QString()
    );
}



void MainWindow::setMaintenanceBusy(
    const bool busy,
    const QString& actionText
) {
    maintenanceBusy_ = busy;

    if (maintenanceProgressBar_ != nullptr) {
        maintenanceProgressBar_->setVisible(busy);
    }

    if (maintenanceRefreshButton_ != nullptr) {
        maintenanceRefreshButton_->setEnabled(!busy);
        maintenanceRefreshButton_->setText(
            busy && actionText == "refresh"
                ? "Перевіряю..."
                : "Перевірити залежності"
        );
    }

    if (maintenanceInstallButton_ != nullptr) {
        maintenanceInstallButton_->setEnabled(!busy);
        maintenanceInstallButton_->setText(
            busy && actionText == "install"
                ? "Встановлюю..."
                : "Довстановити потрібне"
        );
    }

    if (maintenanceCleanupButton_ != nullptr) {
        maintenanceCleanupButton_->setEnabled(!busy);
        maintenanceCleanupButton_->setText(
            busy && actionText == "cleanup"
                ? "Видаляю..."
                : "Видалити безпечний баласт"
        );
    }

    QApplication::processEvents();
}

void MainWindow::setMaintenanceFeedback(
    const QString& message,
    const bool success,
    const bool warning
) {
    if (maintenanceOperationLabel_ != nullptr) {
        const QString marker = success
            ? QString::fromUtf8("✓")
            : (warning ? QString::fromUtf8("⚠") : QString::fromUtf8("✕"));
        const QString background = success
            ? "rgba(54, 179, 126, 0.16)"
            : (warning
                ? "rgba(255, 184, 77, 0.16)"
                : "rgba(224, 82, 82, 0.16)");
        const QString border = success
            ? "rgba(54, 179, 126, 0.55)"
            : (warning
                ? "rgba(255, 184, 77, 0.55)"
                : "rgba(224, 82, 82, 0.55)");

        maintenanceOperationLabel_->setText(
            QString("%1 %2").arg(marker).arg(message)
        );
        maintenanceOperationLabel_->setStyleSheet(
            QString(
                "QLabel#maintenanceOperationStatus {"
                " padding: 10px 12px;"
                " border-radius: 7px;"
                " background: %1;"
                " border: 1px solid %2;"
                "}"
            ).arg(background, border)
        );
    }

    if (maintenanceLastRunLabel_ != nullptr) {
        maintenanceLastRunLabel_->setText(
            QString("Остання операція: %1")
                .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss"))
        );
    }
}

void MainWindow::refreshDependencyMaintenance() {
    if (maintenanceBusy_) {
        return;
    }

    setMaintenanceBusy(true, "refresh");
    if (maintenanceOperationLabel_ != nullptr) {
        maintenanceOperationLabel_->setText("Перевіряю встановлені залежності...");
    }

    const auto audit =
        setup::DependencyMaintenanceManager::audit();

    maintenanceStatusLabel_->setText(
        audit.summary
    );

    maintenanceMissingList_->clear();
    maintenanceMissingList_->addItems(
        audit.missingRequiredPackages
    );

    maintenanceRemovableList_->clear();
    maintenanceRemovableList_->addItems(
        audit.removablePackages
    );

    setMaintenanceBusy(false);

    if (audit.missingRequiredPackages.isEmpty()
        && audit.removablePackages.isEmpty()) {
        setMaintenanceFeedback(
            "Перевірку завершено. Усі залежності актуальні, змін не потрібно.",
            true
        );
    } else {
        setMaintenanceFeedback(
            QString(
                "Перевірку завершено. Потрібно встановити: %1; "
                "безпечно можна видалити: %2."
            )
                .arg(audit.missingRequiredPackages.size())
                .arg(audit.removablePackages.size()),
            true
        );
    }
}

void MainWindow::installMissingDependencies() {
    if (maintenanceBusy_) {
        return;
    }

    const auto audit =
        setup::DependencyMaintenanceManager::audit();

    if (audit.missingRequiredPackages.isEmpty()) {
        setMaintenanceFeedback(
            "Усі потрібні залежності вже встановлено. Система не змінена.",
            true
        );
        QMessageBox::information(
            this,
            "Залежності",
            "Усі потрібні залежності вже встановлено.\n\nСистема не змінена."
        );
        return;
    }

    const auto answer = QMessageBox::question(
        this,
        "Довстановлення залежностей",
        QString(
            "Програма встановить лише відсутні пакети, "
            "потрібні активним пристроям:\n\n%1\n\n"
            "Продовжити?"
        ).arg(
            audit.missingRequiredPackages.join("\n")
        ),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
    );

    if (answer != QMessageBox::Yes) {
        setMaintenanceFeedback(
            "Довстановлення скасовано користувачем. Система не змінена.",
            false,
            true
        );
        return;
    }

    setMaintenanceBusy(true, "install");
    maintenanceOperationLabel_->setText(
        QString("Встановлюю пакетів: %1...")
            .arg(audit.missingRequiredPackages.size())
    );

    const auto result =
        setup::DependencyMaintenanceManager::
            installMissingRequiredPackages();

    setMaintenanceBusy(false);
    setMaintenanceFeedback(
        result.success
            ? QString("Довстановлення завершено. %1").arg(result.details)
            : QString("Не вдалося довстановити залежності. %1").arg(result.details),
        result.success
    );

    QMessageBox::information(
        this,
        result.success ? "Готово" : "Помилка",
        result.details
    );

    refreshDependencyMaintenance();
}

void MainWindow::removeUnusedDependencies() {
    if (maintenanceBusy_) {
        return;
    }

    const auto audit =
        setup::DependencyMaintenanceManager::audit();

    if (audit.removablePackages.isEmpty()) {
        setMaintenanceFeedback(
            "Безпечного баласту для видалення не знайдено. Система не змінена.",
            true
        );
        QMessageBox::information(
            this,
            "Очищення",
            "Безпечного баласту для видалення не знайдено.\n\nСистема не змінена."
        );
        return;
    }

    const auto answer = QMessageBox::warning(
        this,
        "Безпечне очищення",
        QString(
            "Буде запропоновано видалити лише пакети, які:\n"
            "• були встановлені саме Rodavarion TDriver;\n"
            "• не потрібні активним пристроям;\n"
            "• не входять до захищеного списку;\n"
            "• не мають інших залежних пакетів.\n\n"
            "Пакети:\n%1\n\nПродовжити?"
        ).arg(audit.removablePackages.join("\n")),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (answer != QMessageBox::Yes) {
        setMaintenanceFeedback(
            "Очищення скасовано користувачем. Система не змінена.",
            false,
            true
        );
        return;
    }

    setMaintenanceBusy(true, "cleanup");
    maintenanceOperationLabel_->setText(
        QString("Перевіряю та видаляю пакетів: %1...")
            .arg(audit.removablePackages.size())
    );

    const auto result =
        setup::DependencyMaintenanceManager::
            removeUnusedRodavarionPackages();

    setMaintenanceBusy(false);
    setMaintenanceFeedback(
        result.success
            ? QString("Безпечне очищення завершено. %1").arg(result.details)
            : QString("Очищення не завершено. %1").arg(result.details),
        result.success
    );

    QMessageBox::information(
        this,
        result.success ? "Готово" : "Помилка",
        result.details
    );

    refreshDependencyMaintenance();
}

void MainWindow::ensureActionBackend(
    const bool userInitiated
) {
    const auto status =
        setup::DependencySetupManager::inspect();

    if (status.ready) {
        return;
    }

    QSettings settings;
    const bool dismissed =
        settings.value(
            "setup/actionBackendDismissedForVersion",
            false
        ).toBool();

    if (!userInitiated && dismissed) {
        return;
    }

    DependencySetupDialog dialog(this);
    dialog.exec();

    if (dialog.backendReady()) {
        settings.setValue(
            "setup/actionBackendDismissedForVersion",
            false
        );
        restartMouseRuntime();

        if (mouseRuntime_ != nullptr) {
            statusLabel_->setText(
                mouseRuntime_->statusText()
            );
        }
    } else if (!userInitiated) {
        settings.setValue(
            "setup/actionBackendDismissedForVersion",
            true
        );
    }
}

void MainWindow::restartMouseRuntime() {
    if (mouseRuntime_ == nullptr) {
        return;
    }

    // The installed daemon is the single owner of the physical mouse grab.
    // Running a second controller inside the GUI races for EVIOCGRAB, breaks
    // the live monitor and can let original Back/Forward events leak through.
    QProcess serviceState;
    serviceState.start(
        "systemctl",
        {"--user", "is-active", "--quiet", "rodavarion-tdriverd.service"}
    );
    serviceState.waitForFinished(1500);
    const bool daemonActive =
        serviceState.exitStatus() == QProcess::NormalExit
        && serviceState.exitCode() == 0;

    if (daemonActive) {
        mouseRuntime_->stop();
        return;
    }

    // Development/fallback mode when the user service is not installed.
    for (const auto& peripheral : peripherals_) {
        if (peripheral.classification.deviceClass
            == capability::DeviceClass::Mouse
            && peripheral.physicalDevice.hasRealBackend()) {
            mouseRuntime_->start(peripheral);
            return;
        }
    }

    mouseRuntime_->stop();
}

void MainWindow::loadSelectedProfile() {
    const auto* item = profileList_->currentItem();
    if (item == nullptr) {
        return;
    }

    const auto profile = application_.profiles().find(
        item->data(Qt::UserRole)
            .toString()
            .toStdString()
    );

    if (!profile) {
        return;
    }

    profileNameEdit_->setText(
        QString::fromStdString(profile->displayName)
    );
    pointerSpeedEdit_->setText(
        profileSetting(
            *profile,
            "pointer.speed",
            "1.0"
        )
    );
    scrollModeCombo_->setCurrentText(
        profileSetting(
            *profile,
            "scroll.mode",
            "standard"
        )
    );
}

void MainWindow::saveSelectedProfile() {
    auto* item = profileList_->currentItem();
    if (item == nullptr) {
        return;
    }

    profile::Profile profile;
    profile.id = item->data(Qt::UserRole)
        .toString()
        .toStdString();
    profile.displayName =
        profileNameEdit_->text()
            .trimmed()
            .toStdString();
    profile.settings["pointer.speed"] =
        pointerSpeedEdit_->text()
            .trimmed()
            .toStdString();
    profile.settings["scroll.mode"] =
        scrollModeCombo_->currentText()
            .toStdString();

    if (profile.displayName.empty()) {
        statusLabel_->setText(
            "Profile name cannot be empty."
        );
        return;
    }

    if (!application_.profiles().update(profile)
        || !application_.saveProfiles()) {
        statusLabel_->setText(
            "Failed to save profile."
        );
        return;
    }

    item->setText(
        QString::fromStdString(profile.displayName)
    );
}

} // namespace rodavarion::gui

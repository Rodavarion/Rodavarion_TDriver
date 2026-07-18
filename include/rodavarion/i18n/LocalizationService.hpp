#pragma once

#include <QLocale>
#include <QString>

namespace rodavarion::i18n {

enum class Language {
    Automatic,
    Ukrainian,
    English
};

class LocalizationService final {
public:
    static LocalizationService& instance();

    void load();
    void setLanguage(Language language);

    [[nodiscard]] Language configuredLanguage() const noexcept;
    [[nodiscard]] Language effectiveLanguage() const noexcept;
    [[nodiscard]] QString text(const QString& key) const;
    [[nodiscard]] QString languageName(Language language) const;

private:
    LocalizationService() = default;

    Language configured_{Language::Automatic};
};

} // namespace rodavarion::i18n

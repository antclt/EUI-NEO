#include "eui_neo.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace app {

const DslAppConfig& dslAppConfig() {
    static const DslAppConfig config = DslAppConfig{}
        .title("EUI Starter App")
        .pageId("sidebar_starter")
        .clearColor({0.07f, 0.08f, 0.10f, 1.0f})
        .windowSize(1380, 960)
        .fps(90.0);
    return config;
}

namespace {

enum class Page { Home, Settings };

Page currentPage = Page::Home;
bool darkMode = true;
eui::Color accentColor = components::theme::defaultPrimary();

constexpr float kSidebarWidth = 260.0f;
constexpr float kPad = 34.0f;
constexpr const char* kDocsUrl = "https://sudoevolve.github.io/EUI-NEO/";

void composeSidebar(eui::Ui& ui, float height);
void composeContent(eui::Ui& ui, float width, float height);
void composeHome(eui::Ui& ui, float width);
void composeSettings(eui::Ui& ui, float width);
void pageTitle(eui::Ui& ui, const std::string& title, const std::string& subtitle, float width);
void panel(eui::Ui& ui, const std::string& id, float x, float y, float width, float height);
void navItem(eui::Ui& ui, const std::string& id, const std::string& label, unsigned int icon, Page page);
void colorSwatch(eui::Ui& ui, const std::string& id, float x, float y, eui::Color color);
components::theme::ThemeColorTokens theme();
eui::Transition motion();
eui::Color textPrimary();
eui::Color textMuted();
eui::Color borderColor(float alpha = 1.0f);
eui::Color shadowColor();
std::string colorHex(eui::Color color);

} // namespace

// Root UI.
void compose(eui::Ui& ui, const eui::Screen& screen) {
    ui.row("root")
        .size(screen.width, screen.height)
        .content([&] {
            composeSidebar(ui, screen.height);
            composeContent(ui, std::max(0.0f, screen.width - kSidebarWidth), screen.height);
        })
        .build();
}

namespace {

// Left sidebar.
void composeSidebar(eui::Ui& ui, float height) {
    ui.stack("sidebar")
        .size(kSidebarWidth, height)
        .content([&] {
            ui.rect("sidebar.bg")
                .size(kSidebarWidth, height)
                .color(darkMode ? eui::Color{0.070f, 0.082f, 0.105f, 1.0f}
                                : eui::Color{0.965f, 0.972f, 0.984f, 1.0f})
                .transition(motion())
                .animate(eui::AnimProperty::Color)
                .build();

            ui.text("brand.icon").position(28.0f, 48.0f).size(204.0f, 34.0f)
                .icon(0xF5FD).fontSize(27.0f).lineHeight(32.0f)
                .color(theme().primary).horizontalAlign(eui::HorizontalAlign::Center)
                .transition(motion()).build();

            ui.text("brand.title").position(28.0f, 116.0f).size(204.0f, 38.0f)
                .text("Starter App").fontSize(28.0f).lineHeight(34.0f).fontWeight(820)
                .color(textPrimary()).horizontalAlign(eui::HorizontalAlign::Center)
                .transition(motion()).build();

            ui.text("brand.path").position(28.0f, 170.0f).size(204.0f, 24.0f)
                .text("apps/sidebar_starter").fontSize(14.0f).lineHeight(20.0f)
                .color(textMuted()).horizontalAlign(eui::HorizontalAlign::Center).build();

            ui.stack("nav.home.wrap").position(28.0f, 242.0f).size(204.0f, 50.0f)
                .content([&] { navItem(ui, "nav.home", "Home", 0xF015, Page::Home); }).build();
            ui.stack("nav.settings.wrap").position(28.0f, 314.0f).size(204.0f, 50.0f)
                .content([&] { navItem(ui, "nav.settings", "Settings", 0xF013, Page::Settings); }).build();
        })
        .build();
}

// Right page host.
void composeContent(eui::Ui& ui, float width, float height) {
    ui.stack("content")
        .size(width, height)
        .content([&] {
            ui.rect("content.bg").size(width, height).color(theme().background)
                .transition(motion()).animate(eui::AnimProperty::Color).build();
            currentPage == Page::Home ? composeHome(ui, width) : composeSettings(ui, width);
        })
        .build();
}

// Home page.
void composeHome(eui::Ui& ui, float width) {
    const float contentW = std::max(0.0f, width - kPad * 2.0f);
    const float cardTop = kPad + 154.0f;
    const float cardH = 230.0f;
    const float buttonW = 128.0f;
    const float buttonH = 48.0f;

    pageTitle(ui, "Home", "A tiny app shell for building real EUI-NEO desktop apps.", width);
    panel(ui, "home.welcome.bg", kPad, cardTop, contentW, cardH);

    ui.text("home.welcome.title").position(kPad + 24.0f, cardTop + 26.0f).size(contentW - 48.0f, 32.0f)
        .text("Welcome to EUI-NEO").fontSize(24.0f).lineHeight(30.0f).fontWeight(800)
        .color(textPrimary()).build();

    ui.text("home.welcome.body").position(kPad + 24.0f, cardTop + 76.0f).size(contentW - 48.0f, 72.0f)
        .text("This starter keeps the app structure small: a sidebar, a Home page, and a Settings page. Use it as the simplest base for building EUI-NEO desktop apps under apps/.")
        .fontSize(16.0f).lineHeight(24.0f).wrap(true).color(textMuted()).build();

    ui.stack("home.welcome.docs.wrap")
        .position(kPad + contentW - 24.0f - buttonW, cardTop + cardH - 24.0f - buttonH)
        .size(buttonW, buttonH)
        .content([&] {
            components::button(ui, "home.welcome.docs")
                .size(buttonW, buttonH).text("Docs").icon(0xF35D).theme(theme())
                .transition(motion()).onClick([] { eui::platform::openUrl(kDocsUrl); }).build();
        })
        .build();
}

// Settings page.
void composeSettings(eui::Ui& ui, float width) {
    const float cardW = std::min(620.0f, std::max(0.0f, width - kPad * 2.0f));
    const float cardTop = kPad + 154.0f;

    pageTitle(ui, "Settings", "Tune the starter theme without leaving the app.", width);
    panel(ui, "settings.appearance.bg", kPad, cardTop, cardW, 254.0f);

    ui.text("settings.title").position(kPad + 24.0f, cardTop + 24.0f).size(cardW - 48.0f, 32.0f)
        .text("Appearance").fontSize(22.0f).lineHeight(28.0f).fontWeight(800).color(textPrimary()).build();

    ui.stack("settings.dark.wrap").position(kPad + 24.0f, cardTop + 76.0f).size(cardW - 48.0f, 38.0f)
        .content([&] {
            components::toggleSwitch(ui, "settings.dark")
                .theme(theme()).size(cardW - 48.0f, 38.0f).checked(darkMode).text("Night mode")
                .transition(motion()).onChange([](bool value) { darkMode = value; }).build();
        })
        .build();

    ui.text("settings.accent.label").position(kPad + 24.0f, cardTop + 138.0f).size(cardW - 48.0f, 24.0f)
        .text("Theme color").fontSize(14.0f).lineHeight(20.0f).fontWeight(720).color(textMuted()).build();

    ui.text("settings.accent.value").position(kPad + 24.0f, cardTop + 166.0f).size(120.0f, 26.0f)
        .text(colorHex(accentColor)).fontSize(14.0f).lineHeight(20.0f).color(textMuted())
        .transition(motion()).build();

    const char* ids[] = {"blue", "teal", "green", "gold", "rose"};
    const eui::Color colors[] = {
        components::theme::defaultPrimary(),
        {0.12f, 0.72f, 0.78f, 1.0f},
        {0.15f, 0.78f, 0.48f, 1.0f},
        {0.96f, 0.68f, 0.18f, 1.0f},
        {0.92f, 0.28f, 0.46f, 1.0f},
    };
    for (int i = 0; i < 5; ++i) {
        colorSwatch(ui, std::string("settings.color.") + ids[i], kPad + 150.0f + i * 56.0f, cardTop + 170.0f, colors[i]);
    }
}

// Shared UI pieces.
void pageTitle(eui::Ui& ui, const std::string& title, const std::string& subtitle, float width) {
    ui.text("page.title").position(kPad, kPad + 18.0f).size(width - kPad * 2.0f, 44.0f)
        .text(title).fontSize(36.0f).lineHeight(42.0f).fontWeight(840)
        .color(theme().primary).transition(motion()).build();

    ui.text("page.subtitle").position(kPad, kPad + 84.0f).size(width - kPad * 2.0f, 30.0f)
        .text(subtitle).fontSize(18.0f).lineHeight(25.0f)
        .color(textMuted()).transition(motion()).build();
}

void panel(eui::Ui& ui, const std::string& id, float x, float y, float width, float height) {
    components::panel(ui, id, theme())
        .position(x, y).size(width, height).radius(14.0f).border(1.0f, borderColor(0.72f))
        .shadow(18.0f, 0.0f, 8.0f, shadowColor()).transition(motion())
        .animate(eui::AnimProperty::Color | eui::AnimProperty::Border | eui::AnimProperty::Shadow).build();
}

void navItem(eui::Ui& ui, const std::string& id, const std::string& label, unsigned int icon, Page page) {
    const bool active = currentPage == page;
    const auto t = theme();
    const eui::Color base = active ? t.primary : t.surface;

    components::button(ui, id)
        .size(204.0f, 50.0f).text(label).icon(icon).iconSize(16.0f).fontSize(16.0f)
        .colors(base, active ? components::theme::buttonHover(t, t.primary) : t.surfaceHover,
                active ? components::theme::buttonPressed(t, t.primary) : t.surfaceActive)
        .textColor(active || darkMode ? eui::Color{0.96f, 0.98f, 1.0f, 1.0f} : textPrimary())
        .iconColor(active ? eui::Color{1.0f, 1.0f, 1.0f, 1.0f} : t.primary)
        .radius(12.0f).border(1.0f, active ? components::theme::withAlpha(t.primary, 0.62f) : borderColor(0.70f))
        .shadow(12.0f, 0.0f, 4.0f, shadowColor()).transition(motion())
        .onClick([page] { currentPage = page; }).build();
}

void colorSwatch(eui::Ui& ui, const std::string& id, float x, float y, eui::Color color) {
    const bool active = std::fabs(color.r - accentColor.r) < 0.01f &&
                        std::fabs(color.g - accentColor.g) < 0.01f &&
                        std::fabs(color.b - accentColor.b) < 0.01f;
    ui.rect(id + ".ring").position(x - 4.0f, y - 4.0f).size(46.0f, 46.0f)
        .color(active ? theme().primary : borderColor(0.60f)).radius(14.0f)
        .transition(motion()).animate(eui::AnimProperty::Color).build();
    ui.rect(id).position(x, y).size(38.0f, 38.0f)
        .states(color, eui::mixColor(color, eui::Color{1.0f, 1.0f, 1.0f, 1.0f}, 0.16f),
                eui::mixColor(color, eui::Color{0.0f, 0.0f, 0.0f, 1.0f}, 0.16f))
        .radius(11.0f).onClick([color] { accentColor = color; }).build();
}

// Theme helpers.
components::theme::ThemeColorTokens theme() {
    auto t = darkMode ? components::theme::dark() : components::theme::light();
    t.primary = accentColor;
    return t;
}

eui::Transition motion() {
    auto transition = eui::Transition::make(0.18f, eui::Ease::OutCubic);
    transition.animate(eui::AnimProperty::Color | eui::AnimProperty::TextColor |
                       eui::AnimProperty::Border | eui::AnimProperty::Shadow);
    return transition;
}

eui::Color textPrimary() {
    return components::theme::pageVisuals(theme()).titleColor;
}

eui::Color textMuted() {
    return components::theme::pageVisuals(theme()).subtitleColor;
}

eui::Color borderColor(float alpha) {
    return components::theme::withOpacity(theme().border, alpha);
}

eui::Color shadowColor() {
    return darkMode ? eui::Color{0.0f, 0.0f, 0.0f, 0.24f}
                    : eui::Color{0.10f, 0.14f, 0.22f, 0.12f};
}

std::string colorHex(eui::Color color) {
    const int r = static_cast<int>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f + 0.5f);
    const int g = static_cast<int>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f + 0.5f);
    const int b = static_cast<int>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f + 0.5f);
    char buffer[8] = {};
    std::snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", r, g, b);
    return std::string(buffer);
}

} // namespace

} // namespace app

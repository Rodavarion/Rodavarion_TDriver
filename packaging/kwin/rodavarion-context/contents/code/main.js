function normalize(value) {
    if (value === undefined || value === null) return "";
    return String(value).trim().toLowerCase();
}

function publish(window) {
    var parts = [];
    if (window) {
        var values = [
            window.resourceClass,
            window.resourceName,
            window.desktopFileName,
            window.caption
        ];
        for (var i = 0; i < values.length; ++i) {
            var value = normalize(values[i]);
            if (value.length > 0 && parts.indexOf(value) < 0) parts.push(value);
        }
    }
    callDBus(
        "org.rodavarion.TDriver",
        "/Context",
        "org.rodavarion.TDriver.Context",
        "SetActiveWindow",
        parts.join(" | ")
    );
}

workspace.windowActivated.connect(publish);
publish(workspace.activeWindow);

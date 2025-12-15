#include <QApplication>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QStringList>
#include <QWindow>
#include <QProcess>

#include "app/window.hpp"

int main(int argc, char *argv[])
{
    // ---- WebEngine stability tweaks for Raspberry Pi 4 ----
    qputenv("QTWEBENGINE_DISABLE_SANDBOX", "1");
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
            "--no-sandbox "
            "--disable-gpu-sandbox "
            "--disable-features=VaapiVideoDecoder,WebRtcHWDecoding "
            "--disable-software-rasterizer "
            "--ignore-gpu-blocklist "
            "--use-gl=egl");

    qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
    QApplication dash(argc, argv);

    QWebEngineProfile::defaultProfile()->settings()->setAttribute(
        QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(
        QWebEngineSettings::WebGLEnabled, true);
        
    dash.setOrganizationName("openDsh");
    dash.setApplicationName("dash");

    QSize size = dash.primaryScreen()->size();
    QPoint pos = dash.primaryScreen()->geometry().topLeft();

    QSettings settings;
    DASH_LOG(info) << "loaded config: " << settings.fileName().toStdString();

    QStringList args = dash.arguments();

    dash.processEvents();

    MainWindow window(QRect(pos, size));
    window.setWindowIcon(QIcon(":/logo.png"));
    window.setWindowFlags(Qt::FramelessWindowHint);
    window.setWindowState(Qt::WindowFullScreen);

    window.show();
    QProcess::execute("plymouth quit --wait");

    return dash.exec();
}

#include <QApplication>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QStringList>
#include <QWindow>
#include <QProcess>
#include <QCursor>
#include <QScreen>
#include <QSettings>

#include "DashLog.hpp"
#include "app/window.hpp"

int main(int argc, char *argv[])
{
    // ---- WebEngine stability tweaks for Raspberry Pi 4 ----
    qputenv("QTWEBENGINE_DISABLE_SANDBOX", "1");
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
            "--no-sandbox "
            "--disable-gpu-sandbox "
            "--disable-features=VaapiVideoDecoder,WebRtcHWDecoding "
            "--enable-gpu-rasterization "
            "--enable-zero-copy "
            "--ignore-gpu-blocklist "
            "--use-gl=egl"
            "--enable-features=VaapiVideoEncoder "
            "--num-raster-threads=4 "        // Pi 4 has 4 cores, use them
            "--enable-main-frame-before-activation "
            "--renderer-process-limit=2 "    // limit renderer processes, saves RAM
            "--disable-dev-shm-usage "       // Pi /dev/shm is small, avoid it
            "--memory-pressure-off");        // stop Chromium throttling itself
    qputenv("QT_QPA_EGLFS_HIDECURSOR", "1");

    qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
    QApplication dash(argc, argv);

    QWebEngineProfile::defaultProfile()->settings()->setAttribute(
        QWebEngineSettings::Accelerated2dCanvasEnabled, true);
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
    window.setWindowFlags(Qt::FramelessWindowHint);
    window.setWindowState(Qt::WindowFullScreen);

    window.show();
    QProcess::execute("plymouth", {"quit", "--wait"});

    return dash.exec();
}

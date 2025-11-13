#include <QApplication>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QStringList>
#include <QWindow>

#include "app/window.hpp"
#include "app/action.hpp"

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
            "--enable-features=UseOzonePlatform "
            "--ozone-platform=egl "
            "--use-gl=egl");

    qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
    QApplication dash(argc, argv);

    QWebEngineProfile::defaultProfile()->settings()->setAttribute(
        QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(
        QWebEngineSettings::WebGLEnabled, true);
        
    dash.setOrganizationName("openDsh");
    dash.setApplicationName("dash");
    dash.installEventFilter(ActionEventFilter::get_instance());

    QSize size = dash.primaryScreen()->size();
    QPoint pos = dash.primaryScreen()->geometry().topLeft();

    QSettings settings;
    DASH_LOG(info) << "loaded config: " << settings.fileName().toStdString();

    QStringList args = dash.arguments();

    QPixmap pixmap(QPixmap(":/splash.png").scaledToHeight(size.height() / 2));
    QSplashScreen splash(pixmap);
    splash.setMask(pixmap.mask());
    splash.move(pos.x() + ((size.width() / 2) - (splash.width() / 2)), pos.y() + ((size.height() / 2) - (splash.height() / 2)));
    splash.show();
    dash.processEvents();

    MainWindow window(QRect(pos, size));
    window.setWindowIcon(QIcon(":/logo.png"));
    window.setWindowFlags(Qt::FramelessWindowHint);
    window.setWindowState(Qt::WindowFullScreen);

    window.show();
    splash.finish(&window);

    return dash.exec();
}

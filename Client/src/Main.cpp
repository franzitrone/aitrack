#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif // !_DEBUG


#include <QtWidgets/QApplication>
#include "view/WindowMain.h"
#include "presenter/presenter.h"
#include "model/Config.h"
#include <omp.h>
#include "tracker/TrackerFactory.h"

#include "model/UpdateChecker.h"

#include <cstring>

int main(int argc, char *argv[])
{

#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    auto conf_mgr = std::make_unique<ConfigMgr>("./prefs.ini");

#ifdef __linux
    char num_threads_env[] = "OMP_NUM_THREADS=XXXXXX";
    std::memset(&num_threads_env, 0, sizeof(num_threads_env));
#endif
    auto state = conf_mgr->getConfig();
    if (state.onnx_set_env_threads) {
#ifdef _WIN32
        std::wstring ws = std::to_wstring(state.onnx_env_threads);
        SetEnvironmentVariable(LPWSTR(L"OMP_NUM_THREADS"), ws.c_str());
#else
        sprintf(num_threads_env, "OMP_NUM_THREADS=%d", state.onnx_env_threads);
        putenv(num_threads_env);
#endif
    }
    if (state.onnx_set_num_threads) {
        omp_set_num_threads(state.onnx_num_threads);
    }
    if (state.onnx_set_dynamic) {
        omp_set_dynamic(state.onnx_dynamic);
    }

    QApplication app(argc, argv);

    WindowMain w;
    w.show();

    auto t_factory = std::make_unique<TrackerFactory>("./models/");

    Presenter p((IView&)w, std::move(t_factory), std::move(conf_mgr));

    return app.exec();
}

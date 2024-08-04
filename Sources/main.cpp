#include "Parsers/ParseScene.hpp"
#include "Parallel.hpp"
#include "Image.hpp"
#include "Render.hpp"
#include "Timer.hpp"
#include "Common/Error.hpp"
#include <embree4/rtcore.h>
#include <memory>
#include <thread>
#include <vector>
//#include "./Common/Screen.hpp"
#include "./Common/Application.hpp"

using namespace elma;

inline int RunApp(int argc, char* argv[])
{
    AppConfig config;
    config.windowDesc.title           = "Elma - Path Tracing";
    config.windowDesc.resizableWindow = false;

    Application app(config);
    
    return app.run();
    //    if (argc <= 1) {
    //        LogInfo("使用方法 Elma [-t num_threads] [-o output_file_name] filename.xml");
    //        return 1;
    //    }
    //
    //    auto numThreads = static_cast<int>(std::thread::hardware_concurrency()) - 1;
    //    std::string outputFile;
    //    std::vector<std::string> filenames;
    //    for (int i = 1; i < argc; ++i) {
    //        if (std::string(argv[i]) == "-t") {
    //            numThreads = std::stoi(std::string(argv[++i]));
    //        }
    //        else if (std::string(argv[i]) == "-o") {
    //            outputFile = std::string(argv[++i]);
    //        }
    //        else {
    //            filenames.emplace_back(argv[i]);
    //        }
    //    }
    //
    //    RTCDevice embree_device = rtcNewDevice(nullptr);
    //    parallel_init(numThreads);
    //
    //    for (const std::string& filename : filenames) {
    //        Timer timer;
    //        tick(timer);
    //        LogInfo("解析并构造场景 '{}'...", filename);
    //        std::unique_ptr<Scene> scene = parse_scene(filename, embree_device);
    //        LogInfo("场景构造完成，花费 '{}' 秒", tick(timer));
    //
    //        Image3 img;
    //        Image3f viewImg(scene->camera.width, scene->camera.height);
    //
    //        nanogui::init();
    //        auto* screen = new ElmaScreen(viewImg);
    //
    //        std::thread thread([&] {
    //            tick(timer);
    //            LogInfo("开始进行渲染...");
    //            img = render(*scene, viewImg);
    //            LogInfo("渲染完毕, 花费 '{}' 秒", tick(timer));
    //        });
    //
    //        nanogui::mainloop(50.f);
    //
    //        thread.join();
    //
    //        delete screen;
    //        nanogui::shutdown();
    //
    //        if (outputFile.empty()) {
    //            outputFile = scene->output_filename;
    //        }
    //        imwrite(outputFile, img);
    //        std::cout << "Done. Image written to " << outputFile << std::endl;
    //    }
    //
    //    parallel_cleanup();
    //    rtcReleaseDevice(embree_device);

    return 0;
}

int main(int argc, char* argv[])
{
    return CatchAndReportAllExceptions([&] { return RunApp(argc, argv); });
}

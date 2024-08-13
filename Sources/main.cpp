#include "Common/Error.hpp"
#include <thread>
#include <vector>
#include "./Common/Application.hpp"

using namespace elma;

inline int RunApp(int argc, char* argv[])
{
    //    if (argc <= 1) {
    //        LogInfo("使用方法 Elma [-t num_threads] [-o output_file_name] filename.xml");
    //        return 1;
    //    }

    AppConfig config;
    config.windowDesc.title           = "Elma - Path Tracing";
    config.windowDesc.resizableWindow = false;
    config.inputSceneFilename         = "Data/Scenes/disney_bsdf_test/disney_sheen.xml";

    config.numThreads = static_cast<int>(std::thread::hardware_concurrency()) - 1;
//    std::string outputFile;
//    std::vector<std::string> filenames;
//    for (int i = 1; i < argc; ++i) {
//        if (std::string(argv[i]) == "-t") {
//            config.numThreads = std::stoi(std::string(argv[++i]));
//        }
//        else if (std::string(argv[i]) == "-o") {
//            config.outputFilename = std::string(argv[++i]);
//        }
//        else {
//            config.inputSceneFilename = argv[i];
//        }
//    }

    Application app(config);

    return app.run();
}

int main(int argc, char* argv[])
{
    return CatchAndReportAllExceptions([&] { return RunApp(argc, argv); });
}

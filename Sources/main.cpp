#include "Parsers/ParseScene.hpp"
#include "Parallel.hpp"
#include "Image.hpp"
#include "Render.hpp"
#include "Timer.hpp"
#include <embree4/rtcore.h>
#include <memory>
#include <thread>
#include <vector>

using namespace elma;

int main(int argc, char* argv[])
{
    if (argc <= 1) {
        std::cout << "[Usage] ./lajolla [-t num_threads] [-o output_file_name] filename.xml" << std::endl;
        return 0;
    }

    auto num_threads = static_cast<int>(std::thread::hardware_concurrency());
    std::string outputFile;
    std::vector<std::string> filenames;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-t") {
            num_threads = std::stoi(std::string(argv[++i]));
        }
        else if (std::string(argv[i]) == "-o") {
            outputFile = std::string(argv[++i]);
        }
        else {
            filenames.emplace_back(argv[i]);
        }
    }

    RTCDevice embree_device = rtcNewDevice(nullptr);
    parallel_init(num_threads);

    for (const std::string& filename : filenames) {
        Timer timer;
        tick(timer);
        std::cout << "Parsing and constructing scene " << filename << "." << std::endl;
        std::unique_ptr<Scene> scene = parse_scene(filename, embree_device);
        std::cout << "Done. Took " << tick(timer) << " seconds." << std::endl;
        std::cout << "Rendering..." << std::endl;
        Image3 img = render(*scene);
        if (outputFile.empty()) {
            outputFile = scene->output_filename;
        }
        std::cout << "Done. Took " << tick(timer) << " seconds." << std::endl;
        imwrite(outputFile, img);
        std::cout << "Image written to " << outputFile << std::endl;
    }

    parallel_cleanup();
    rtcReleaseDevice(embree_device);
    return 0;
}

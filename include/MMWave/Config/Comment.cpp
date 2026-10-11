#include "MMWave/Config/Comment.hpp"

namespace Sarwave::Core::MMWave::Config {

const std::shared_ptr<const Comment>& Comment::blank() {
    static const auto instance = std::make_shared<const Comment>(std::nullopt);
    return instance;
}

}  // namespace Sarwave::Core::MMWave::Config

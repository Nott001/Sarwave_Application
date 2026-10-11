// The .cfg model's parity harness: C++ twin of csharp-config/Program.cs (built against the
// original Sarwave.Core.MMWave), printing the same output line for line. Arguments:
//   <corpus.txt> <cfg dir>... -- <cfg file>...
// ConfigTests runs it on src/tests/data/config and compares with golden_cs.txt, the C# output.
#include <cxxabi.h>

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "MMWave/Config/BaseConfigParameters.hpp"
#include "MMWave/Config/Comment.hpp"
#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"
#include "MMWave/Config/Parameters/FlushCfg.hpp"
#include "MMWave/Config/Parameters/SensorStart.hpp"
#include "MMWave/Config/Parameters/SensorStop.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/AopBoard.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/PeopleTrackingConfigReader.hpp"

using namespace Sarwave::Core;
using namespace Sarwave::Core::MMWave::Config;
using namespace Sarwave::Core::MMWave::Config::Parameters;
using namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843;
namespace PT = Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters;

static std::ostringstream o;

static std::string E(std::string_view s) {
    std::string b;
    for (char ch : s) {
        auto c = static_cast<unsigned char>(ch);
        switch (c) {
            case '\\':
                b += "\\\\";
                break;
            case '\n':
                b += "\\n";
                break;
            case '\r':
                b += "\\r";
                break;
            case '\t':
                b += "\\t";
                break;
            default:
                if (c < 0x20 || c == 0x7f)
                    b += std::format("\\x{:02x}", c);
                else
                    b += ch;
        }
    }
    return b;
}

static void appendUtf8(std::string& out, char32_t c) {
    if (c < 0x80)
        out += char(c);
    else if (c < 0x800) {
        out += char(0xC0 | (c >> 6));
        out += char(0x80 | (c & 0x3F));
    } else {
        out += char(0xE0 | (c >> 12));
        out += char(0x80 | ((c >> 6) & 0x3F));
        out += char(0x80 | (c & 0x3F));
    }
}

static std::string Unescape(std::string_view s) {
    std::string b;
    for (std::size_t i = 0; i < s.size(); i++) {
        if (s[i] != '\\' || i + 1 >= s.size()) {
            b += s[i];
            continue;
        }
        char n = s[++i];
        switch (n) {
            case 'n':
                b += '\n';
                break;
            case 'r':
                b += '\r';
                break;
            case 't':
                b += '\t';
                break;
            case '0':
                b += '\0';
                break;
            case '\\':
                b += '\\';
                break;
            case 'u':
                appendUtf8(b, char32_t(std::stoi(std::string(s.substr(i + 1, 4)), nullptr, 16)));
                i += 4;
                break;
            default:
                b += '\\';
                b += n;
                break;
        }
    }
    return b;
}

static std::string B(bool v) {
    return v ? "True" : "False";
}

template <class T>
static std::string typeName(const T& value) {
    int status = 0;
    char* name = abi::__cxa_demangle(typeid(value).name(), nullptr, nullptr, &status);
    std::string full = name ? name : typeid(value).name();
    std::free(name);
    auto pos = full.rfind("::");
    return pos == std::string::npos ? full : full.substr(pos + 2);
}

static std::string excName(const std::exception& ex) {
    if (dynamic_cast<const ConfigFormatException*>(&ex)) return "ConfigFormatException";
    if (dynamic_cast<const FileNotFoundException*>(&ex)) return "FileNotFoundException";
    if (dynamic_cast<const ArgumentException*>(&ex)) return "ArgumentException";
    return typeName(ex);
}

static std::string msg(const std::exception& ex) {
    if (auto* c = dynamic_cast<const ConfigFormatException*>(&ex)) return c->message();
    return ex.what();
}

static void Exc(const std::exception& ex) {
    int line = -1;
    if (auto* c = dynamic_cast<const ConfigFormatException*>(&ex)) line = c->lineNumber();
    o << "  EXC " << excName(ex) << " line=" << line << ": " << E(msg(ex)) << "\n";
    try {
        std::rethrow_if_nested(ex);
    } catch (const std::exception& inner) {
        o << "    inner " << excName(inner) << ": " << E(msg(inner)) << "\n";
    }
}

static void P(const std::string& name, const IConfigParameter* p) {
    if (!p)
        o << "  ." << name << " = null\n";
    else
        o << "  ." << name << " = " << typeName(*p) << " " << E(p->toString()) << "\n";
}

static void Dump(const PeopleTrackingConfig& config, bool allProps) {
    o << "  lines=" << config.parameters().size() << "\n";
    int i = 0;
    for (const auto& p : config.parameters()) {
        std::string extra;
        if (auto* c = dynamic_cast<const Comment*>(p.get())) {
            extra = " blank=" + B(c->isBlank()) + " text=" + (c->text() ? E(*c->text()) : "null");
        } else if (auto* r = dynamic_cast<const RawParameter*>(p.get())) {
            std::string joined;
            for (std::size_t k = 0; k < r->arguments().size(); k++) {
                if (k) joined += "|";
                joined += E(r->arguments()[k]);
            }
            extra = " args=" + std::to_string(r->arguments().size()) + " [" + joined + "]";
        }
        o << "  [" << i++ << "] " << typeName(*p) << " " << E(p->paramName()) << " | "
          << E(p->toString()) << extra << "\n";
    }

    std::vector<std::pair<std::string, const IConfigParameter*>> props{
        {"DfeDataOutputMode", config.dfeDataOutputMode()},
        {"ChannelCfg", config.channelCfg()},
        {"AdcCfg", config.adcCfg()},
        {"AdcbufCfg", config.adcbufCfg()},
        {"LowPower", config.lowPower()},
        {"ProfileCfg", config.profileCfg()},
        {"FrameCfg", config.frameCfg()},
        {"BpmCfg", config.bpmCfg()},
        {"DynamicRangeAngleCfg", config.dynamicRangeAngleCfg()},
        {"DynamicRACfarCfg", config.dynamicRACfarCfg()},
        {"Dynamic2DAngleCfg", config.dynamic2DAngleCfg()},
        {"StaticRangeAngleCfg", config.staticRangeAngleCfg()},
        {"StaticRACfarCfg", config.staticRACfarCfg()},
        {"FineMotionCfg", config.fineMotionCfg()},
        {"AntGeometry0", config.antGeometry0()},
        {"AntGeometry1", config.antGeometry1()},
        {"AntPhaseRot", config.antPhaseRot()},
        {"FovCfg", config.fovCfg()},
        {"CompRangeBiasAndRxChanPhase", config.compRangeBiasAndRxChanPhase()},
        {"StaticBoundaryBox", config.staticBoundaryBox()},
        {"BoundaryBox", config.boundaryBox()},
        {"PresenceBoundaryBox", config.presenceBoundaryBox()},
        {"SensorPosition", config.sensorPosition()},
        {"GatingParam", config.gatingParam()},
        {"StateParam", config.stateParam()},
        {"AllocationParam", config.allocationParam()},
        {"MaxAcceleration", config.maxAcceleration()},
        {"TrackingCfg", config.trackingCfg()},
    };
    for (const auto& [name, p] : props) {
        if (allProps || p) P(name, p);
    }

    auto chirps = config.chirpCfgs();
    if (allProps || !chirps.empty()) {
        o << "  .ChirpCfgs count=" << chirps.size() << "\n";
        for (auto* c : chirps) o << "    " << E(c->toString()) << "\n";
    }
    auto raws = config.unrecognised();
    if (allProps || !raws.empty()) {
        o << "  .Unrecognised count=" << raws.size() << "\n";
        for (auto* r : raws) o << "    " << E(r->toString()) << "\n";
    }
    o << "  text=" << E(config.toString()) << "\n";
    o << "  crlf=" << E(config.toString("\r\n")) << "\n";
}

static void Case(const std::string& label, const std::string& text, bool allProps = false) {
    o << "CASE " << label << " <" << E(text) << ">\n";
    try {
        Dump(PeopleTrackingConfigReader::parse(text), allProps);
    } catch (const std::exception& ex) {
        Exc(ex);
    }
}

static std::string readAll(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

static std::uint32_t state = 2463534242u;
static std::uint32_t Next() {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

std::string configParityRun(const std::vector<std::string>& args) {
    o.str({});
    o.clear();
    auto sep = std::find(args.begin(), args.end(), "--") - args.begin();
    std::string corpusPath = args[0];
    std::vector<std::string> cfgDirs(args.begin() + 1, args.begin() + sep);
    std::vector<std::string> filePaths(args.begin() + sep + 1, args.end());

    // ---- A: sample files
    for (const auto& dir : cfgDirs) {
        std::vector<std::filesystem::path> files;
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.path().extension() == ".cfg") files.push_back(entry.path());
        }
        std::ranges::sort(files, {}, [](const auto& p) { return p.filename().string(); });
        for (const auto& path : files) {
            o << "FILE " << path.filename().string() << "\n";
            Dump(PeopleTrackingConfigReader::readFile(path), true);
            // File.ReadAllText strips a BOM and decodes; the samples are plain ASCII.
            Case("file-text " + path.filename().string(), readAll(path));
        }
    }

    // ---- B: corpus
    int n = 0;
    {
        std::ifstream corpus(corpusPath, std::ios::binary);
        std::string raw;
        while (std::getline(corpus, raw)) {
            if (raw.starts_with('#')) continue;
            n++;
            if (raw.starts_with("@num ")) {
                std::string token = Unescape(raw.substr(5));
                Case(std::to_string(n) + "i", "stateParam " + token + " 0 0 0 0 0");
                Case(std::to_string(n) + "f", "maxAcceleration " + token + " 0 0");
                Case(std::to_string(n) + "e", "adcCfg 2 " + token);
                Case(std::to_string(n) + "b", "bpmCfg -1 " + token + " 0 1");
                continue;
            }
            Case(std::to_string(n), Unescape(raw));
        }
    }

    // ---- C: random floats
    for (int i = 0; i < 5000; i++) {
        std::uint32_t bits = Next();
        float f = std::bit_cast<float>(bits);
        std::string s = ConfigText::line("v", f);
        std::string back;
        try {
            auto c = PeopleTrackingConfigReader::parse("maxAcceleration " + s.substr(2) + " 0 0");
            back = std::format("{:08x}", std::bit_cast<std::uint32_t>(c.maxAcceleration()->x));
        } catch (const std::exception& ex) {
            back = std::string("EXC ") + ex.what();
        }
        o << std::format("RB {:08x} {} {}\n", bits, s, back);
    }

    for (int i = 0; i < 5000; i++) {
        std::string t;
        if (Next() % 2 == 0) t += '-';
        int digits = 1 + int(Next() % 10);
        int point = int(Next() % std::uint32_t(digits + 1));
        for (int k = 0; k < digits; k++) {
            if (k == point && k > 0) t += '.';
            t += char('0' + Next() % 10);
        }
        if (Next() % 3 == 0) {
            t += Next() % 2 == 0 ? 'e' : 'E';
            int e = int(Next() % 101) - 50;
            t += std::to_string(e);
        }
        try {
            auto c = PeopleTrackingConfigReader::parse("maxAcceleration " + t + " 0 0");
            auto c2 = PeopleTrackingConfigReader::parse("stateParam " + t + " 0 0 0 0 0");
            o << std::format("RD {} {:08x} {} | {}\n", t,
                             std::bit_cast<std::uint32_t>(c.maxAcceleration()->x),
                             E(c.maxAcceleration()->toString()), E(c2.stateParam()->toString()));
        } catch (const std::exception& ex) {
            o << "RD " << t << " EXC " << E(msg(ex)) << "\n";
        }
    }

    // ---- D: reading files
    for (const auto& path : filePaths) {
        o << "READ <" << E(path) << ">\n";
        try {
            Dump(PeopleTrackingConfigReader::readFile(path), false);
        } catch (const std::exception& ex) {
            Exc(ex);
        }
    }

    // ---- E: API
    o << "API\n";
    o << "  aop0 " << AopBoard::antGeometry0()->toString() << "\n";
    o << "  aop1 " << AopBoard::antGeometry1()->toString() << "\n";
    o << "  aopRot " << AopBoard::antPhaseRot()->toString() << "\n";
    o << "  blank <" << Comment::blank()->toString() << "> " << B(Comment::blank()->isBlank())
      << " " << Comment::blank()->paramName() << " " << B(!Comment::blank()->text()) << "\n";
    o << "  comment <" << Comment(" x").toString() << "> " << B(Comment(" x").isBlank()) << " <"
      << Comment("").toString() << "> " << B(Comment("").isBlank()) << "\n";
    o << "  raw <" << RawParameter("foo", {}).toString() << "> <"
      << RawParameter("foo", {"a", "b"}).toString() << "> <" << RawParameter("foo", {""}).toString()
      << ">\n";
    auto& reg = PeopleTrackingConfigReader::registry();
    o << "  knows " << B(reg.knows("sensorStop")) << " " << B(reg.knows("SensorStop")) << " "
      << B(reg.knows("guiMonitor")) << " " << B(reg.knows("dynamic2DAngleCfg")) << " "
      << B(reg.knows("compRangeBiasAndRxChanPhase")) << "\n";
    ConfigParameterRegistry baseOnly;
    BaseConfigParameters::addBaseParameters(baseOnly);
    o << "  baseKnows " << B(baseOnly.knows("profileCfg")) << " "
      << B(baseOnly.knows("trackingCfg")) << "\n";
    o << "  baseParse " << typeName(*baseOnly.parse("trackingCfg", {"1", "2"})) << " "
      << baseOnly.parse("channelCfg", {"15", "7", "0"})->toString() << "\n";
    o << "  line <"
      << ConfigText::line("x", 1, 1.5f, true, false, AdcCfg::OutputFormat::Complex1x, "str", -0.0f,
                          PT::AntPhaseRot::Rotation::Inverted)
      << ">\n";
    o << "  list <" << ConfigText::list("x", std::vector<int>{1, -2, 3}) << "> <"
      << ConfigText::list("x", std::vector<float>{0.5f, 1e-5f}) << "> <"
      << ConfigText::list("x", std::vector<int>{}) << ">\n";
    std::string sb = "y";
    ConfigText::append(sb, 2.5f);
    ConfigText::append(sb, static_cast<ChannelCfg::Rx>(static_cast<int>(ChannelCfg::Rx::Rx0) |
                                                       static_cast<int>(ChannelCfg::Rx::Rx3)));
    o << "  append <" << sb << ">\n";
    auto layout = PeopleTrackingConfigReader::parse("% a\n\nsensorStop\nfoo 1");
    o << "  toString <" << E(layout.toString("")) << "> <" << E(layout.toString("\r\n")) << "> <"
      << E(layout.toString("||")) << ">\n";
    ConfigFormatException e1("msg");
    ConfigFormatException e2("msg", 5);
    o << "  cfe <" << e1.what() << "> " << e1.lineNumber() << " <" << e2.what() << "> "
      << e2.lineNumber() << " " << B(std::is_base_of_v<FormatException, ConfigFormatException>)
      << "\n";
    std::vector<std::string> v1{"1", "x", "2"};
    ConfigArguments args1("p", v1);
    {
        auto a = args1.paramName();
        auto b = args1.count();
        auto c = args1.remaining();
        auto d = args1.intValue("a");
        auto e = args1.remaining();
        o << "  args " << a << " " << b << " " << c << " " << d << " " << e << "\n";
    }
    try {
        args1.intValue("b");
    } catch (const std::exception& ex) {
        o << "  args EXC " << ex.what() << "\n";
    }
    {
        auto a = args1.remaining();
        auto b = args1.floatValue("c");
        auto c = args1.remaining();
        o << "  args " << a << " " << Sarwave::Core::MMWave::DotNet::formatSingle(b) << " " << c
          << "\n";
    }
    try {
        args1.boolValue("d");
    } catch (const std::exception& ex) {
        o << "  args EXC " << ex.what() << "\n";
    }
    std::vector<std::string> v2{"1", "-1", "1"};
    ConfigArguments args2("q", v2);
    {
        auto values = args2.remainingEnums<PT::AntPhaseRot::Rotation>("r");
        std::string joined;
        for (std::size_t k = 0; k < values.size(); k++)
            joined += (k ? "," : "") + std::to_string(static_cast<int>(values[k]));
        o << "  enums " << joined << " " << args2.remaining() << "\n";
    }
    std::vector<std::string> v3{"1", "5"};
    ConfigArguments args3("q", v3);
    {
        auto values = args3.remainingInts("r");
        std::string joined;
        for (std::size_t k = 0; k < values.size(); k++)
            joined += (k ? "," : "") + std::to_string(values[k]);
        auto rem = args3.remaining();
        auto len = args3.remainingInts("s").size();
        o << "  ints " << joined << " " << rem << " " << len << "\n";
    }
    std::vector<std::string> v4{"3", "16"};
    ConfigArguments args4("w", v4);
    o << "  flags " << static_cast<int>(args4.enumValue<ChannelCfg::Rx>("rx")) << "\n";
    try {
        args4.enumValue<ChannelCfg::Rx>("rx2");
    } catch (const std::exception& ex) {
        o << "  flags EXC " << ex.what() << "\n";
    }
    auto p1 = PeopleTrackingConfigReader::parse(
        "profileCfg 0 60.75 30.00 25.00 59.10 394758 0 54.71 1 96 2950.00 2 1 36");
    auto p2 = PeopleTrackingConfigReader::parse(
        "profileCfg 0 60.75 30 25 59.1 394758 0 54.71 1 96 2950 2 1 36");
    auto p3 = PeopleTrackingConfigReader::parse(
        "profileCfg 0 60.75 30 25 59.1 394758 0 54.71 1 96 2950 2 1 37");
    o << "  eq " << B(*p1.profileCfg() == *p2.profileCfg()) << " "
      << B(*p1.profileCfg() == *p3.profileCfg()) << " " << B(*p1.profileCfg() == *p2.profileCfg())
      << "\n";
    o << "  eq2 " << B(SensorStart() == SensorStart(std::nullopt)) << " "
      << B(SensorStart(0) == SensorStart()) << " " << B(SensorStop() == SensorStop()) << " "
      << B(FlushCfg() == FlushCfg()) << "\n";
    PT::TrackingCfg t1(true, 2, 800, 30, 46, 96, 55);
    o << "  eq3 " << B(t1 == PT::TrackingCfg(true, 2, 800, 30, 46, 96, 55, std::nullopt)) << " "
      << B(t1 == PT::TrackingCfg(true, 2, 800, 30, 46, 96, 55, false)) << " "
      << B(Comment("x") == Comment("x")) << " " << B(*Comment::blank() == Comment(std::nullopt))
      << "\n";
    PT::Dynamic2DAngleZoomInCfg d1(-1, 4, 1, 1, 1.0f, 30.0f,
                                   PT::Dynamic2DAngleZoomInCfg::LocalMax::Elevation);
    auto d2 = d1;
    d2.zoominFactor = 5;
    o << "  eq4 "
      << B(d1 == PT::Dynamic2DAngleZoomInCfg(-1, 4, 1, 1, 1.0f, 30.0f,
                                             PT::Dynamic2DAngleZoomInCfg::LocalMax::Elevation))
      << " " << B(d1 == d2) << " " << d1.subFrameIdx << " " << d1.paramName() << "\n";
    o << "  eq5 " << B(PT::MaxAcceleration(0.0f, 1, 1) == PT::MaxAcceleration(-0.0f, 1, 1)) << "\n";
    o << "  ctor " << SensorStart(3).toString() << " "
      << PT::TrackingCfg(false, 2, 800, 30, 46, 96, 55, true).toString() << " "
      << CompRangeBiasAndRxChanPhase(0.25f, {{1.0f, 0.0f}, {-1.0f, 0.5f}}).toString() << " "
      << CompRangeBiasAndRxChanPhase(0.0f, {}).toString() << "\n";
    o << "  ctor2 "
      << PT::Dynamic2DAngleElevationCfg(-1, 3, 0.03f, 1, 0, true, 0.3f, 0.85f, 8).toString() << " "
      << AdcbufCfg(-1, AdcbufCfg::OutputFormat::Complex, AdcbufCfg::IqOrder::QLsbIMsb,
                   AdcbufCfg::ChannelLayout::NonInterleaved, 1)
             .toString()
      << "\n";
    o << "  undefinedEnum "
      << AdcCfg(static_cast<AdcCfg::BitDepth>(9), static_cast<AdcCfg::OutputFormat>(-3)).toString()
      << " "
      << ChannelCfg(static_cast<ChannelCfg::Rx>(255), static_cast<ChannelCfg::Tx>(5), 0).toString()
      << "\n";

    return o.str();
}

#ifndef SARWAVE_CONFIG_PARITY_NO_MAIN
int main(int argc, char** argv) {
    const std::string out = configParityRun(std::vector<std::string>(argv + 1, argv + argc));
    std::fwrite(out.data(), 1, out.size(), stdout);
}
#endif

using System.Text;
using Sarwave.Core.MMWave.Config;
using Sarwave.Core.MMWave.Config.Parameters;
using Sarwave.Core.MMWave.Configured.PeopleTracking.Config.IWR6843;
using Sarwave.Core.MMWave.Configured.PeopleTracking.Config.IWR6843.Parameters;

// Usage: Harness <corpus.txt> <cfg dir>... -- <file path>...
var o = new StreamWriter(Console.OpenStandardOutput(), new UTF8Encoding(false)) { NewLine = "\n" };
int sep = Array.IndexOf(args, "--");
string corpusPath = args[0];
string[] cfgDirs = args[1..sep];
string[] filePaths = args[(sep + 1)..];

static string E(string s)
{
    var b = new StringBuilder();
    foreach (char c in s)
    {
        switch (c)
        {
            case '\\': b.Append("\\\\"); break;
            case '\n': b.Append("\\n"); break;
            case '\r': b.Append("\\r"); break;
            case '\t': b.Append("\\t"); break;
            default:
                if (c < 0x20 || c == 0x7f) b.Append("\\x").Append(((int)c).ToString("x2"));
                else b.Append(c);
                break;
        }
    }
    return b.ToString();
}

static string Unescape(string s)
{
    var b = new StringBuilder();
    for (int i = 0; i < s.Length; i++)
    {
        if (s[i] != '\\' || i + 1 >= s.Length) { b.Append(s[i]); continue; }
        char n = s[++i];
        switch (n)
        {
            case 'n': b.Append('\n'); break;
            case 'r': b.Append('\r'); break;
            case 't': b.Append('\t'); break;
            case '0': b.Append('\0'); break;
            case '\\': b.Append('\\'); break;
            case 'u': b.Append((char)Convert.ToInt32(s.Substring(i + 1, 4), 16)); i += 4; break;
            default: b.Append('\\').Append(n); break;
        }
    }
    return b.ToString();
}

static string B(bool v) => v ? "True" : "False";

void Exc(Exception ex)
{
    int line = ex is ConfigFormatException c ? c.LineNumber : -1;
    o.WriteLine($"  EXC {ex.GetType().Name} line={line}: {E(ex.Message)}");
    if (ex.InnerException is { } inner)
        o.WriteLine($"    inner {inner.GetType().Name}: {E(inner.Message)}");
}

void P(string name, IConfigParameter? p) =>
    o.WriteLine(p is null ? $"  .{name} = null" : $"  .{name} = {p.GetType().Name} {E(p.ToString())}");

void Dump(PeopleTrackingConfig config, bool allProps)
{
    o.WriteLine($"  lines={config.Parameters.Count}");
    int i = 0;
    foreach (IConfigParameter p in config.Parameters)
    {
        string extra = p switch
        {
            Comment c => $" blank={B(c.IsBlank)} text={(c.Text is null ? "null" : E(c.Text))}",
            RawParameter r => $" args={r.Arguments.Count} [{string.Join("|", r.Arguments.Select(E))}]",
            _ => ""
        };
        o.WriteLine($"  [{i++}] {p.GetType().Name} {E(p.ParamName)} | {E(p.ToString())}{extra}");
    }

    var props = new (string, IConfigParameter?)[]
    {
        ("DfeDataOutputMode", config.DfeDataOutputMode), ("ChannelCfg", config.ChannelCfg),
        ("AdcCfg", config.AdcCfg), ("AdcbufCfg", config.AdcbufCfg), ("LowPower", config.LowPower),
        ("ProfileCfg", config.ProfileCfg), ("FrameCfg", config.FrameCfg), ("BpmCfg", config.BpmCfg),
        ("DynamicRangeAngleCfg", config.DynamicRangeAngleCfg), ("DynamicRACfarCfg", config.DynamicRACfarCfg),
        ("Dynamic2DAngleCfg", config.Dynamic2DAngleCfg), ("StaticRangeAngleCfg", config.StaticRangeAngleCfg),
        ("StaticRACfarCfg", config.StaticRACfarCfg), ("FineMotionCfg", config.FineMotionCfg),
        ("AntGeometry0", config.AntGeometry0), ("AntGeometry1", config.AntGeometry1),
        ("AntPhaseRot", config.AntPhaseRot), ("FovCfg", config.FovCfg),
        ("CompRangeBiasAndRxChanPhase", config.CompRangeBiasAndRxChanPhase),
        ("StaticBoundaryBox", config.StaticBoundaryBox), ("BoundaryBox", config.BoundaryBox),
        ("PresenceBoundaryBox", config.PresenceBoundaryBox), ("SensorPosition", config.SensorPosition),
        ("GatingParam", config.GatingParam), ("StateParam", config.StateParam),
        ("AllocationParam", config.AllocationParam), ("MaxAcceleration", config.MaxAcceleration),
        ("TrackingCfg", config.TrackingCfg),
    };
    foreach ((string name, IConfigParameter? p) in props)
    {
        if (allProps || p is not null) P(name, p);
    }

    var chirps = config.ChirpCfgs.ToList();
    if (allProps || chirps.Count > 0)
    {
        o.WriteLine($"  .ChirpCfgs count={chirps.Count}");
        foreach (ChirpCfg c in chirps) o.WriteLine($"    {E(c.ToString())}");
    }
    var raws = config.Unrecognised.ToList();
    if (allProps || raws.Count > 0)
    {
        o.WriteLine($"  .Unrecognised count={raws.Count}");
        foreach (RawParameter r in raws) o.WriteLine($"    {E(r.ToString())}");
    }
    o.WriteLine($"  text={E(config.ToString())}");
    o.WriteLine($"  crlf={E(config.ToString("\r\n"))}");
}

void Case(string label, string text, bool allProps = false)
{
    o.WriteLine($"CASE {label} <{E(text)}>");
    try
    {
        Dump(PeopleTrackingConfigReader.Parse(text), allProps);
    }
    catch (Exception ex)
    {
        Exc(ex);
    }
}

// ---- A: sample files
foreach (string dir in cfgDirs)
{
    foreach (string path in Directory.GetFiles(dir, "*.cfg").OrderBy(p => Path.GetFileName(p), StringComparer.Ordinal))
    {
        o.WriteLine($"FILE {Path.GetFileName(path)}");
        Dump(PeopleTrackingConfigReader.ReadFile(path), true);
        Case("file-text " + Path.GetFileName(path), File.ReadAllText(path));
    }
}

// ---- B: corpus
int n = 0;
foreach (string raw in File.ReadAllLines(corpusPath))
{
    if (raw.StartsWith('#')) continue;
    n++;
    if (raw.StartsWith("@num "))
    {
        string token = Unescape(raw[5..]);
        Case($"{n}i", $"stateParam {token} 0 0 0 0 0");
        Case($"{n}f", $"maxAcceleration {token} 0 0");
        Case($"{n}e", $"adcCfg 2 {token}");
        Case($"{n}b", $"bpmCfg -1 {token} 0 1");
        continue;
    }
    Case(n.ToString(), Unescape(raw));
}

// ---- C: random floats
uint state = 2463534242;
uint Next()
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

for (int i = 0; i < 5000; i++)
{
    uint bits = Next();
    float f = BitConverter.UInt32BitsToSingle(bits);
    string s = ConfigText.Line("v", f);
    string back;
    try
    {
        var c = PeopleTrackingConfigReader.Parse("maxAcceleration " + s[2..] + " 0 0");
        back = BitConverter.SingleToUInt32Bits(c.MaxAcceleration!.X).ToString("x8");
    }
    catch (Exception ex) { back = "EXC " + ex.Message; }
    o.WriteLine($"RB {bits:x8} {s} {back}");
}

for (int i = 0; i < 5000; i++)
{
    var t = new StringBuilder();
    if (Next() % 2 == 0) t.Append('-');
    int digits = 1 + (int)(Next() % 10);
    int point = (int)(Next() % (uint)(digits + 1));
    for (int k = 0; k < digits; k++)
    {
        if (k == point && k > 0) t.Append('.');
        t.Append((char)('0' + Next() % 10));
    }
    if (Next() % 3 == 0)
    {
        t.Append(Next() % 2 == 0 ? 'e' : 'E');
        int e = (int)(Next() % 101) - 50;
        t.Append(e);
    }
    string text = t.ToString();
    try
    {
        var c = PeopleTrackingConfigReader.Parse("maxAcceleration " + text + " 0 0");
        var c2 = PeopleTrackingConfigReader.Parse("stateParam " + text + " 0 0 0 0 0");
        o.WriteLine($"RD {text} {BitConverter.SingleToUInt32Bits(c.MaxAcceleration!.X):x8} {E(c.MaxAcceleration!.ToString())} | {E(c2.StateParam!.ToString())}");
    }
    catch (Exception ex) { o.WriteLine($"RD {text} EXC {E(ex.Message)}"); }
}

// ---- D: reading files
foreach (string path in filePaths)
{
    o.WriteLine($"READ <{E(path)}>");
    try
    {
        Dump(PeopleTrackingConfigReader.ReadFile(path), false);
    }
    catch (Exception ex)
    {
        Exc(ex);
    }
}

// ---- E: API
o.WriteLine("API");
o.WriteLine($"  aop0 {AopBoard.AntGeometry0}");
o.WriteLine($"  aop1 {AopBoard.AntGeometry1}");
o.WriteLine($"  aopRot {AopBoard.AntPhaseRot}");
o.WriteLine($"  blank <{Comment.Blank}> {B(Comment.Blank.IsBlank)} {Comment.Blank.ParamName} {B(Comment.Blank.Text is null)}");
o.WriteLine($"  comment <{new Comment(" x")}> {B(new Comment(" x").IsBlank)} <{new Comment("")}> {B(new Comment("").IsBlank)}");
o.WriteLine($"  raw <{new RawParameter("foo", [])}> <{new RawParameter("foo", ["a", "b"])}> <{new RawParameter("foo", [""])}>");
o.WriteLine($"  knows {B(PeopleTrackingConfigReader.Registry.Knows("sensorStop"))} {B(PeopleTrackingConfigReader.Registry.Knows("SensorStop"))} {B(PeopleTrackingConfigReader.Registry.Knows("guiMonitor"))} {B(PeopleTrackingConfigReader.Registry.Knows("dynamic2DAngleCfg"))} {B(PeopleTrackingConfigReader.Registry.Knows("compRangeBiasAndRxChanPhase"))}");
var baseOnly = new ConfigParameterRegistry().AddBaseParameters();
o.WriteLine($"  baseKnows {B(baseOnly.Knows("profileCfg"))} {B(baseOnly.Knows("trackingCfg"))}");
o.WriteLine($"  baseParse {baseOnly.Parse("trackingCfg", ["1", "2"]).GetType().Name} {baseOnly.Parse("channelCfg", ["15", "7", "0"])}");
o.WriteLine($"  line <{ConfigText.Line("x", 1, 1.5f, true, false, AdcCfg.OutputFormat.Complex1x, "str", -0f, AntPhaseRot.Rotation.Inverted)}>");
o.WriteLine($"  list <{ConfigText.List("x", new[] { 1, -2, 3 })}> <{ConfigText.List("x", new[] { 0.5f, 1e-5f })}> <{ConfigText.List("x", Array.Empty<int>())}>");
var sb = new StringBuilder("y");
ConfigText.Append(sb, 2.5f);
ConfigText.Append(sb, ChannelCfg.Rx.Rx0 | ChannelCfg.Rx.Rx3);
o.WriteLine($"  append <{sb}>");
var layout = PeopleTrackingConfigReader.Parse("% a\n\nsensorStop\nfoo 1");
o.WriteLine($"  toString <{E(layout.ToString(""))}> <{E(layout.ToString("\r\n"))}> <{E(layout.ToString("||"))}>");
var e1 = new ConfigFormatException("msg");
var e2 = new ConfigFormatException("msg", 5);
o.WriteLine($"  cfe <{e1.Message}> {e1.LineNumber} <{e2.Message}> {e2.LineNumber} {B(e1 is FormatException)}");
var args1 = new ConfigArguments("p", ["1", "x", "2"]);
o.WriteLine($"  args {args1.ParamName} {args1.Count} {args1.Remaining} {args1.Int("a")} {args1.Remaining}");
try { args1.Int("b"); } catch (Exception ex) { o.WriteLine($"  args EXC {ex.Message}"); }
o.WriteLine($"  args {args1.Remaining} {args1.Float("c")} {args1.Remaining}");
try { args1.Bool("d"); } catch (Exception ex) { o.WriteLine($"  args EXC {ex.Message}"); }
var args2 = new ConfigArguments("q", ["1", "-1", "1"]);
o.WriteLine($"  enums {string.Join(",", args2.RemainingEnums<AntPhaseRot.Rotation>("r").Select(v => v.ToString("D")))} {args2.Remaining}");
var args3 = new ConfigArguments("q", ["1", "5"]);
o.WriteLine($"  ints {string.Join(",", args3.RemainingInts("r"))} {args3.Remaining} {args3.RemainingInts("s").Length}");
var args4 = new ConfigArguments("w", ["3", "16"]);
o.WriteLine($"  flags {args4.Enum<ChannelCfg.Rx>("rx"):D}");
try { args4.Enum<ChannelCfg.Rx>("rx2"); } catch (Exception ex) { o.WriteLine($"  flags EXC {ex.Message}"); }
var p1 = PeopleTrackingConfigReader.Parse("profileCfg 0 60.75 30.00 25.00 59.10 394758 0 54.71 1 96 2950.00 2 1 36");
var p2 = PeopleTrackingConfigReader.Parse("profileCfg 0 60.75 30 25 59.1 394758 0 54.71 1 96 2950 2 1 36");
var p3 = PeopleTrackingConfigReader.Parse("profileCfg 0 60.75 30 25 59.1 394758 0 54.71 1 96 2950 2 1 37");
o.WriteLine($"  eq {B(p1.ProfileCfg == p2.ProfileCfg)} {B(p1.ProfileCfg == p3.ProfileCfg)} {B(p1.ProfileCfg!.Equals(p2.ProfileCfg))}");
o.WriteLine($"  eq2 {B(new SensorStart() == new SensorStart(null))} {B(new SensorStart(0) == new SensorStart())} {B(new SensorStop() == new SensorStop())} {B(new FlushCfg() == new FlushCfg())}");
var t1 = new TrackingCfg(true, 2, 800, 30, 46, 96, 55);
o.WriteLine($"  eq3 {B(t1 == new TrackingCfg(true, 2, 800, 30, 46, 96, 55, null))} {B(t1 == new TrackingCfg(true, 2, 800, 30, 46, 96, 55, false))} {B(new Comment("x") == new Comment("x"))} {B(Comment.Blank == new Comment((string?)null))}");
var d1 = new Dynamic2DAngleZoomInCfg(-1, 4, 1, 1, 1f, 30f, Dynamic2DAngleZoomInCfg.LocalMax.Elevation);
o.WriteLine($"  eq4 {B(d1 == new Dynamic2DAngleZoomInCfg(-1, 4, 1, 1, 1f, 30f, Dynamic2DAngleZoomInCfg.LocalMax.Elevation))} {B(d1 == d1 with { ZoominFactor = 5 })} {d1.SubFrameIdx} {d1.ParamName}");
o.WriteLine($"  eq5 {B(new MaxAcceleration(0f, 1, 1) == new MaxAcceleration(-0f, 1, 1))}");
o.WriteLine($"  ctor {new SensorStart(3)} {new TrackingCfg(false, 2, 800, 30, 46, 96, 55, true)} {new CompRangeBiasAndRxChanPhase(0.25f, [(1f, 0f), (-1f, 0.5f)])} {new CompRangeBiasAndRxChanPhase(0f, [])}");
o.WriteLine($"  ctor2 {new Dynamic2DAngleElevationCfg(-1, 3, 0.03f, 1, 0, true, 0.3f, 0.85f, 8)} {new AdcbufCfg(-1, AdcbufCfg.OutputFormat.Complex, AdcbufCfg.IqOrder.QLsbIMsb, AdcbufCfg.ChannelLayout.NonInterleaved, 1)}");
o.WriteLine($"  undefinedEnum {new AdcCfg((AdcCfg.BitDepth)9, (AdcCfg.OutputFormat)(-3))} {new ChannelCfg((ChannelCfg.Rx)255, ChannelCfg.Tx.Tx0 | ChannelCfg.Tx.Tx2, 0)}");
o.Flush();

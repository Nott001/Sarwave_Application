using System.Globalization;
using System.Reflection;
using System.Text;
using Sarwave.Core.MMWave.Configured.PeopleTracking;
using Sarwave.Core.MMWave.Configured.PeopleTracking.Association;
using Sarwave.Core.MMWave.Prompting;
using Sarwave.Core.MMWave.Recording;
using Sarwave.Core.MMWave.Streaming;

CultureInfo.DefaultThreadCurrentCulture = CultureInfo.InvariantCulture;
CultureInfo.CurrentCulture = CultureInfo.InvariantCulture;

static string F(float v) => v.ToString();
static string V(System.Numerics.Vector3 v) => $"{F(v.X)},{F(v.Y)},{F(v.Z)}";
static string Ex(Exception e) => $"EXCEPTION {e.GetType().FullName}: {e.Message}";

string cmd = args[0];
switch (cmd)
{
    case "log":
    {
        using var rec = FrameRecording.Load(args[1]);
        var logger = new DebugLogger { Conf = new() { HideBytes = args[2] == "1" } };
        for (int i = 0; i < rec.Count; i++)
        {
            try { logger.PrintFrameDataVerbose(in rec.Frames[i], rec.Timestamps[i]); }
            catch (Exception e) { Console.WriteLine(Ex(e)); break; }
        }
        break;
    }
    case "manager":
    {
        using var rec = FrameRecording.Load(args[1]);
        var manager = new CombinedManager(TimeSpan.FromMilliseconds(double.Parse(args[2])));
        var sb = new StringBuilder();
        for (int i = 0; i < rec.Count; i++)
        {
            try { manager.Update(in rec.Frames[i], rec.Timestamps[i]); }
            catch (Exception e) { sb.Append(Ex(e)).Append('\n'); break; }
            sb.Append($"F {i} presence={manager.Presence}\n");
            foreach (TargetObject t in manager.AllTargetObjects)
            {
                sb.Append($" T {t.ObjectId} {t.State} pos={V(t.Pos)} vel={V(t.Vel)} acc={V(t.Acc)} z={F(t.MinZ)}~{F(t.MaxZ)} g={F(t.G)} c={F(t.ConfidenceLevel)} ec={F(t.Ec.M11)},{F(t.Ec.M23)},{F(t.Ec.M44)}\n");
            }
            foreach (PointEntry e in manager.PointClouds)
            {
                sb.Append($" C {e.Timestamp.Ticks} assoc={e.IsAssociated} n={e.PointValues.Length}\n");
                if (e.IsAssociated)
                {
                    foreach (int id in e.ObjectIdsUsed(false))
                    {
                        var pts = e.GetPointsOf(id);
                        // Order within a group is unspecified (introsort); print a sorted multiset.
                        var items = new List<string>();
                        foreach (var p in pts) items.Add($"{F(p.Cartesian.X)},{F(p.Cartesian.Y)},{F(p.Cartesian.Z)},{F(p.Doppler)},{F(p.Snr)},{F(p.Radial.Rannge)}");
                        items.Sort(StringComparer.Ordinal);
                        sb.Append($"  G {id} {pts.Length}: {string.Join(";", items)}\n");
                    }
                    sb.Append($"  X {string.Join(",", e.ObjectIdsUsed(true))}\n");
                }
                else
                {
                    var items = new List<string>();
                    foreach (var p in e.PointValues) items.Add($"{F(p.Cartesian.X)},{F(p.Cartesian.Y)},{F(p.Cartesian.Z)},{F(p.Doppler)},{F(p.Snr)}");
                    sb.Append($"  U {string.Join(";", items)}\n");
                }
            }
        }
        Console.Write(sb.ToString());
        break;
    }
    case "reader":
    {
        byte[] raw = File.ReadAllBytes(args[1]);
        uint seed = uint.Parse(args[2]);
        var reader = new FrameReader(null!);
        var list = (List<byte>)typeof(FrameReader).GetField("_buffer", BindingFlags.NonPublic | BindingFlags.Instance)!.GetValue(reader)!;
        var tryExtract = typeof(FrameReader).GetMethod("TryExtractFrame", BindingFlags.NonPublic | BindingFlags.Instance)!;
        int pos = 0, count = 0;
        while (true)
        {
            object[] a = { null };
            if ((bool)tryExtract.Invoke(reader, a)!)
            {
                Frame f = (Frame)a[0];
                var b = f.Bytes;
                uint sum = 0; foreach (byte x in b) sum = sum * 31 + x;
                Console.WriteLine($"frame {count++} len={b.Length} sum={sum}");
                f.Dispose();
                continue;
            }
            if (pos >= raw.Length) break;
            seed = seed * 1103515245u + 12345u;
            int chunk = 1 + (int)((seed >> 8) % 700u);
            chunk = Math.Min(chunk, raw.Length - pos);
            list.AddRange(new ArraySegment<byte>(raw, pos, chunk));
            pos += chunk;
        }
        Console.WriteLine($"end buffered={list.Count}");
        break;
    }
    case "load":
    {
        try
        {
            using var rec = FrameRecording.Load(args[1]);
            Console.WriteLine($"count={rec.Count} truncated={rec.IsTruncated} duration={rec.Duration.Ticks}");
        }
        catch (Exception e) { Console.WriteLine(Ex(e)); }
        break;
    }
    case "rewrite":
    {
        using var rec = FrameRecording.Load(args[1]);
        using (var w = new FrameRecorder(args[2]))
        {
            for (int i = 0; i < rec.Count; i++) w.Record(in rec.Frames[i], rec.Timestamps[i]);
            Console.WriteLine($"frames={w.FrameCount}");
        }
        break;
    }
    case "playback":
    {
        using var rec = FrameRecording.Load(args[1]);
        int n = int.Parse(args[3]);
        var seen = new List<string>();
        var done = new ManualResetEventSlim();
        var pb = new FramePlayback(rec) { Speed = double.Parse(args[2]), Loop = true };
        pb.Begin((in Frame frame, TimeSpan ts) =>
        {
            lock (seen)
            {
                if (seen.Count < n) seen.Add($"{ts.Ticks} {frame.Bytes.Length}");
                if (seen.Count == n) done.Set();
            }
        });
        done.Wait();
        try { pb.Stop(); } catch (OperationCanceledException) { }
        lock (seen) foreach (var s in seen) Console.WriteLine(s);
        break;
    }
    case "prompt":
    {
        foreach (string input in new[] { "0", "5", " 7 ", "+3", "-0", "-1", "abc", "", "2147483647", "2147483648", "1.0", "0x10", "٣" })
        {
            try { Console.WriteLine($"'{input}' -> {General.CheckStrArgument(input)}"); }
            catch (Exception e) { Console.WriteLine($"'{input}' -> {Ex(e)}"); }
        }
        Console.WriteLine(General.CreatePath(0, 1).Cli + " " + General.CreatePath(0, 1).Data);
        break;
    }
    case "fixed":
    {
        var vals = new List<double> { 0, -0.0, 0.5, 1.5, 2.5, -0.5, 0.125, 0.135, 1.005, 2.675, 0.001, -0.001, -0.004, -0.005, 0.045, 12, 12.5, 99.995, 999.9999, 1e-7, 123456.785, 1.25, -1.25, 0.05, 0.95, 0.995, 3.14159, 2.5e10, 1234567.89, 0.12499999 };
        uint seed = 9;
        for (int i = 0; i < 3000; i++) { seed = seed * 1103515245u + 12345u; double r = ((seed >> 8) % 2000001) / 1000.0 - 1000.0; if (i % 3 == 0) r /= 977.0; vals.Add(r); }
        string[] fmts = { "0.00", "0.##", "0.#", "0", "0.000", "0.0" };
        foreach (double v in vals)
        {
            var parts = new List<string>();
            foreach (var f in fmts) parts.Add(v.ToString(f));
            foreach (var f in fmts) parts.Add(((float)v).ToString(f));
            Console.WriteLine(string.Join(" ", parts));
        }
        break;
    }
    case "random":
    {
        var r = new Random(1);
        var parts = new List<string>();
        for (int i = 0; i < 12; i++) parts.Add(r.NextSingle().ToString("R"));
        Console.WriteLine(string.Join(" ", parts));
        var r2 = new Random(42);
        Console.WriteLine($"{r2.Next()} {r2.Next()} {r2.NextDouble():R}");
        break;
    }
    case "timespan":
    {
        foreach (long t in new long[] { 0, 1, 9, 10, 12345678, 599999999, 600000000, 36000000000, 36123456789, 863999999999 })
            Console.WriteLine($@"{t} {new TimeSpan(t):mm\:ss\.ffffff}");
        break;
    }
}

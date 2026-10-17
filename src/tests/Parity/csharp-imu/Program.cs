// C# twin of ../ImuParityHarness.cpp: the same commands, the same text.
using System.Globalization;
using System.Numerics;
using Sarwave.Core.Imu;
using Sarwave.Core.Imu.Fusion;
using Sarwave.Core.Imu.Gating;
using Sarwave.Core.Imu.Recording;

CultureInfo.DefaultThreadCurrentCulture = CultureInfo.InvariantCulture;
CultureInfo.CurrentCulture = CultureInfo.InvariantCulture;

static string F(float v) => v.ToString();
static string V(Vector3 v) => $"{F(v.X)},{F(v.Y)},{F(v.Z)}";
static string Q(Quaternion q) => $"{F(q.X)},{F(q.Y)},{F(q.Z)},{F(q.W)}";

switch (args[0])
{
    case "process":
    {
        ImuRecording recording = ImuRecording.Load(args[1]);
        int every = int.Parse(args[2]);
        ImuProcessor processor = new(ImuMounting.Identity);
        int i = 0;
        foreach (ImuSample sample in recording.Samples)
        {
            ImuState s = processor.Process(in sample);
            if (i++ % every != 0) continue;
            Console.WriteLine($"{s.Timestamp.Ticks} a={V(s.Acceleration)} w={V(s.AngularRate)} up={V(s.Up)} bias={V(processor.GyroBias)} still={processor.IsStill}");
        }
        Console.WriteLine($"count={recording.Count} truncated={recording.IsTruncated} rate={F(recording.SampleRate)}");
        break;
    }
    case "gate":
    {
        ImuRecording recording = ImuRecording.Load(args[1]);
        MotionGate gate = new(recording.ToTimeline(ImuMounting.Identity));
        TimeSpan first = recording.Samples[0].Timestamp;
        TimeSpan last = recording.Samples[^1].Timestamp;
        TimeSpan period = TimeSpan.FromSeconds(double.Parse(args[2]) / 1000.0);
        for (TimeSpan t = first + TimeSpan.FromSeconds(0.2); t <= last; t += period)
        {
            ImuFrameState s = gate.Evaluate(t);
            var w = s.Window;
            Console.WriteLine($"{t.Ticks} {s.State} accepted={s.Accepted} settle={s.SettleRemaining.Ticks} n={w.SampleCount} peak={F(w.PeakAngularRate)},{F(w.PeakLinearAcceleration)} mean={F(w.MeanAngularRate)},{F(w.MeanLinearAcceleration)} up={V(w.MeanUp)} gap={w.MaxGap.Ticks},{w.TrailingGap.Ticks} pitch={F(s.Tilt.PitchDegrees)} roll={F(s.Tilt.RollDegrees)} q={Q(s.Tilt.Rotation)}");
        }
        Console.WriteLine($"accepted={gate.AcceptedFrames} rejected={gate.RejectedFrames}");
        break;
    }
    case "rewrite":
    {
        ImuRecording recording = ImuRecording.Load(args[1]);
        using ImuRecorder recorder = new(args[2], recording.SampleRate);
        recorder.Record(recording.Samples);
        Console.WriteLine($"samples={recorder.SampleCount}");
        break;
    }
    case "mount":
    {
        for (int a = 0; a < 6; a++)
        for (int b = 0; b < 6; b++)
        for (int c = 0; c < 6; c++)
        {
            Console.Write($"{a}{b}{c} ");
            try
            {
                ImuMounting m = ImuMounting.FromAxes((ImuAxis)a, (ImuAxis)b, (ImuAxis)c);
                ImuMounting trimmed = m.WithLevelCalibration(new Vector3(0.03f, -0.05f, 1));
                Console.WriteLine($"{Q(m.Rotation)} x={V(m.ToRadar(Vector3.UnitX))} t={Q(trimmed.Rotation)}");
            }
            catch (Exception e)
            {
                Console.WriteLine($"EXCEPTION {e.GetType().FullName}: {e.Message}");
            }
        }
        break;
    }
}

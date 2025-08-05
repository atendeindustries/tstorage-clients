
using System.Globalization;
using TStorage.Interfaces;

namespace TStorage.Tests.FunctionalTests.Utilities
{
    public class TestDataProvider
    {
        public static RecordsSet<byte[]> LoadTestData(string path)
        {
            var recordsSet = new RecordsSet<byte[]>();
            foreach (var line in File.ReadLines(path))
            {
                if (string.IsNullOrWhiteSpace(line)) { continue; }

                var data = line.Split(' ', StringSplitOptions.RemoveEmptyEntries);
                if (data.Length != 6) continue;

                int cid = int.Parse(data[0]);
                long mid = long.Parse(data[1]);
                int moid = int.Parse(data[2]);
                long cap = long.Parse(data[3]);
                long acq = long.Parse(data[4]);
                byte[] payload = HexToBytes(data[5]);

                recordsSet.Append(new(new Key(cid, mid, moid, cap, acq), payload));
            }

            return recordsSet;
        }

        private static byte[] HexToBytes(string hex)
        {
            if (hex.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
            {
                hex = hex[2..];
            }

            byte[] bytes = new byte[hex.Length / 2];

            for (int i = 0; i < bytes.Length; i++)
                bytes[i] = byte.Parse(hex.Substring(i * 2, 2), NumberStyles.HexNumber);

            return bytes;
        }
    }
}

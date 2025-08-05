/*
 * Copyright 2025 Atende Industries
 */

using System.Globalization;
using TStorage.Interfaces;
using System.Text;

namespace ExampleApp
{
    public class Helper
    {
        public class PayloadTypeBytes : IPayloadType<byte[]>
        {
            public byte[]? FromBytes(byte[] bytes)
            {
                return bytes;
            }

            public byte[] ToBytes(byte[] value)
            {
                return value;
            }
        }

        public static RecordsSet<byte[]> LoadRecordsSetFromFile(string filePath, char sep = ',')
        {
            RecordsSet<byte[]> recordsSet = new();
            foreach (var line in File.ReadLines(filePath))
            {
                if (string.IsNullOrWhiteSpace(line)) { continue; }

                var data = line.Split(sep);
                if (data.Length != 5)
                {
                    throw new FormatException($"Invalid number of fields, expected 5, got {data.Length}) in line: {line}");
                }

                int cid = int.Parse(data[0]);
                long mid = long.Parse(data[1]);
                int moid = int.Parse(data[2]);
                long cap = long.Parse(data[3]);
                long acq = -1;
                byte[] payload = HexToBytes(data[4]);

                recordsSet.Append(new(new Key(cid, mid, moid, cap, acq), payload));
            }

            return recordsSet;
        }

        public static void WriteRecordToStdOut(Record<byte[]> record, char sep = ',')
        {

            Console.WriteLine($"{record.Key.Cid}{sep}{record.Key.Mid}{sep}{record.Key.Moid}{sep}{record.Key.Cap}{sep}{record.Key.Acq}{sep}{BytesToHex(record.Value)}");
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

        private static string BytesToHex(byte[] bytes)
        {
            if (bytes == null || bytes.Length == 0)
            {
                return "";
            }

            StringBuilder stringBuilder = new(bytes.Length * 2);
            foreach (byte b in bytes)
                stringBuilder.AppendFormat("{0:x2}", b);
            return stringBuilder.ToString();
        }
    }
}
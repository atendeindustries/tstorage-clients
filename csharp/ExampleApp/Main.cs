/*
 * Copyright 2025 Atende Industries
 */

using TStorage.Interfaces;
using TStorage.Main;

namespace ExampleApp
{
    internal class Program
    {
        // Usage:
        // ExampleApp <host> <port>
        // <cid1> <mid1> <moid1> <cap1> <acq1>
        // <cid2> <mid2> <moid2> <cap2> <acq2>
        // <csvFilePath>
        static int Main(string[] args)
        {
            string host = args[0];
            int port = int.Parse(args[1]);
            Key keyRangeMin = new(cid: int.Parse(args[2]), mid: long.Parse(args[3]), moid: int.Parse(args[4]), cap: long.Parse(args[5]), acq: long.Parse(args[6]));
            Key keyRangeMax = new(cid: int.Parse(args[7]), mid: long.Parse(args[8]), moid: int.Parse(args[9]), cap: long.Parse(args[10]), acq: long.Parse(args[11]));
            RecordsSet<byte[]> inputData = Helper.LoadRecordsSetFromFile(filePath: args[12]);

            using Channel<byte[]> channel = new(host, port, new Helper.PayloadTypeBytes());

            Response connectResult = channel.Connect();
            if (connectResult.Status != ResponseStatus.OK)
            {
                Console.Error.WriteLine($"Connection went wrong. Error: {connectResult.Status}");
                return -1;
            }

            Response putResult = channel.Put(inputData);
            if (putResult.Status != ResponseStatus.OK)
            {
                Console.Error.WriteLine($"Put went wrong. Error: {putResult.Status}");
                return -1;
            }

            ResponseAcq result = channel.GetStream(keyRangeMin, keyRangeMax, records =>
            {
                foreach (var record in records)
                {
                    Helper.WriteRecordToStdOut(record);
                }
            });
            if (result.Status != ResponseStatus.OK)
            {
                Console.Error.WriteLine($"Get went wrong. Error: {result.Status}");
                return -1;
            }

            return 0;
        }
    }
}

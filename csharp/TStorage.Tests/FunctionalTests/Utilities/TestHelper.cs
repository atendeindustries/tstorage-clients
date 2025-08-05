using System.Diagnostics.CodeAnalysis;
using System.Net.Http.Headers;
using TStorage.Interfaces;

namespace TStorage.Tests.FunctionalTests.Utilities;

public class TestHelper
{
    public static void StartTStorage(ServerManager serverManager)
    {
        serverManager.StartServers();
        serverManager.WaitUntilServersReady();
        Thread.Sleep(TestConfig.NetworkToleranceIntervalMs);
    }

    public class CompareWithoutAcq : IEqualityComparer<RecordsSet<byte[]>>
    {
        public bool Equals(RecordsSet<byte[]>? a, RecordsSet<byte[]>? b)
        {
            if (a is null || b is null || (a.Size != b.Size)) return false;

            var aSorted = a.OrderBy(o => (o.Key.Cid, o.Key.Mid, o.Key.Moid, o.Key.Cap)).ToList();
            var bSorted = b.OrderBy(o => (o.Key.Cid, o.Key.Mid, o.Key.Moid, o.Key.Cap)).ToList();

            for (int i = 0; i < aSorted.Count; i++)
            {
                if (aSorted[i].Key.Cid != bSorted[i].Key.Cid
                || aSorted[i].Key.Mid != bSorted[i].Key.Mid
                || aSorted[i].Key.Moid != bSorted[i].Key.Moid
                || aSorted[i].Key.Cap != bSorted[i].Key.Cap
                || !aSorted[i].Value.SequenceEqual(bSorted[i].Value))
                {
                    return false;
                }
            }
            return true;
        }

        public int GetHashCode(RecordsSet<byte[]> obj)
        {
            var hash = new HashCode();
            foreach (var record in obj)
            {
                hash.Add(record.Key.Cid);
                hash.Add(record.Key.Mid);
                hash.Add(record.Key.Moid);
                hash.Add(record.Key.Cap);
                hash.Add(record.Value);
            }
            return hash.ToHashCode();
        }
    }

    public class CompareAll : IEqualityComparer<RecordsSet<byte[]>>
    {
        public bool Equals(RecordsSet<byte[]>? a, RecordsSet<byte[]>? b)
        {
            if (a is null || b is null || (a.Size != b.Size)) return false;

            var aSorted = a.OrderBy(o => o.Key).ToList();
            var bSorted = b.OrderBy(o => o.Key).ToList();

            for (int i = 0; i < aSorted.Count; i++)
            {
                if (!aSorted[i].Key.Equals(bSorted[i].Key)
                || !aSorted[i].Value.SequenceEqual(bSorted[i].Value))
                {
                    return false;
                }
            }
            return true;
        }

        public int GetHashCode(RecordsSet<byte[]> obj)
        {
            var hash = new HashCode();
            foreach (var record in obj)
            {
                hash.Add(record.Key);
                hash.Add(record.Value);
            }
            return hash.ToHashCode();
        }
    }
}

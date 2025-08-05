using TStorage.Interfaces;
using TStorage.Main;
using TStorage.Tests.FunctionalTests.Utilities;
using TStorage.Utilities;

namespace TStorage.Tests.FunctionalTests
{
    [Collection("SynchronisedTests")]
    public class GetStreamTests
    {
        [Fact]
        public void GetStream_BasicTest()
        {
            // Arrange
            var testRecords = Task.Run(() => TestDataProvider.LoadTestData("FunctionalTests/Utilities/1K-test-records.txt"));
            using var serverManager = new ServerManager();
            TestHelper.StartTStorage(serverManager);
            using Channel<byte[]> channel = new(serverManager.ListenAddress, serverManager.ListenPort, new PayloadTypeBytes());
            channel.Connect();

            var putResult = channel.Puta(testRecords.Result);
            Assert.True(putResult.Status == ResponseStatus.OK);

            // Act
            RecordsSet<byte[]> resultRecords = new();
            ResponseAcq responseAcq = channel.GetStream(Key.Min(), Key.Max(), records =>
            {
                foreach (var record in records)
                {
                    resultRecords.Append(new(record.Key, record.Value));
                }
            });

            // Assert
            Assert.Equal(ResponseStatus.OK, responseAcq.Status);
            Assert.Equal(testRecords.Result, resultRecords, new TestHelper.CompareAll());
        }
    }
}
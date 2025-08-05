using TStorage.Interfaces;
using TStorage.Main;
using TStorage.Tests.FunctionalTests.Utilities;
using TStorage.Utilities;

namespace TStorage.Tests.FunctionalTests
{
    [Collection("SynchronisedTests")]
    public class PutGetTests
    {
        [Fact]
        public void PutGet_BasicTest()
        {
            // Arrange
            var testRecords = Task.Run(() => TestDataProvider.LoadTestData("FunctionalTests/Utilities/1K-test-records.txt"));
            using var serverManager = new ServerManager();
            TestHelper.StartTStorage(serverManager);
            using Channel<byte[]> channel = new(serverManager.ListenAddress, serverManager.ListenPort, new PayloadTypeBytes());
            channel.Connect();

            // Act and assert
            var putResult = channel.Put(testRecords.Result);
            Assert.True(putResult.Status == ResponseStatus.OK);

            var resultRecords = channel.Get(Key.Min(), Key.Max());
            Assert.Equal(testRecords.Result, resultRecords.Data, new TestHelper.CompareWithoutAcq());
        }
    }
}
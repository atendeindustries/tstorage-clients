/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Tests.FunctionalTests.Utilities
{
    public class ServerManager : IDisposable
    {
        public void StartServers()
        {
            _dTimestamp.Start();
            Thread.Sleep(millisecondsTimeout: 100);
            _usConnector.Start();
            _usTracker.Start();
            _usContainer.Start();
        }

        public void StopServers()
        {
            _dTimestamp.Stop();
            _usConnector.Stop();
            _usTracker.Stop();
            _usContainer.Stop();
        }

        public void WaitUntilServersReady()
        {
            BaseServer[] baseServers = [_dTimestamp, _usConnector, _usContainer, _usTracker];
            while (true)
            {
                if (baseServers.All(server => server.IsReady()))
                {
                    break;
                }
                Thread.Sleep(100);
            }
        }

        public async Task WaitUntilServersStopped()
        {
            BaseServer[] baseServers = [_dTimestamp, _usConnector, _usContainer, _usTracker];
            while (true)
            {
                var statuses = await Task.WhenAll(baseServers.Select(s => s.WaitForStop()));
                if (statuses.All(status => status))
                {
                    break;
                }
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_isDisposed)
            {
                _isDisposed = true;
                if (disposing)
                {
                    _usConnector.Dispose();
                    _usContainer.Dispose();
                    _usTracker.Dispose();
                    _dTimestamp.Dispose();
                }
            }
        }
        private bool _isDisposed = false;


        public string ListenAddress { get => TestConfig.USConnectorListenAddres; }
        public int ListenPort { get => TestConfig.USConnectorListenPort; }
        private readonly USConnector _usConnector = new();
        private readonly USContainer _usContainer = new();
        private readonly USTracker _usTracker = new();
        private readonly DTimestamp _dTimestamp = new();
    }
}

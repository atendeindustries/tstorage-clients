/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Tests.FunctionalTests.Utilities
{
    public class BaseServer(string binName, string logPath, string arguments = "") : IDisposable
    {
        public virtual void Start()
        {
            CleanLogs();
            _ = _process.RunAsync();
        }

        public void Stop()
        {
            _process.Stop();
            AssertCorrectEnd();
        }

        public void CleanLogs()
        {
            var logPath = LogPath;
            if (File.Exists(logPath))
            {
                File.Delete(logPath);
            }
        }

        public bool IsReady()
        {
            return File.Exists(_logPath);
        }

        public async Task<bool> WaitForStop()
        {
            await _process!.WaitForStop();
            return true;
        }

        public string LogPath
        {
            get
            {
                if (File.Exists(_logPath))
                {
                    return _logPath;
                }

                return _logPath + "_error";
            }
        }
        private readonly string _logPath = logPath;

        private void AssertCorrectEnd()
        {
            // TODO ExitCode is equal 137 after .Kill()
            // if (_process.ExitCode != 0)
            // {
            //     throw new InvalidOperationException($"Process terminated with return code: {_process.ExitCode}");
            // }
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
                    Stop();
                    _process?.Dispose();
                }
            }
        }
        private bool _isDisposed = false;

        private readonly ProcessWrapper _process = new(binName, arguments);
    }
}

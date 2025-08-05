/*
 * Copyright 2025 Atende Industries
 */

using System.Diagnostics;
using System.Text;

namespace TStorage.Tests.FunctionalTests.Utilities
{
    public class ProcessWrapper(string binName, string arguments = "") : IDisposable
    {
        public async Task RunAsync()
        {
            CreateProcess();

            _outputBuilder ??= new StringBuilder();
            _errorBuilder ??= new StringBuilder();
            _outputBuilder.Clear();
            _errorBuilder.Clear();

            if (RedirectStandardOutput)
            {
                _process!.OutputDataReceived += (_, e) => { if (e.Data != null) { _outputBuilder.AppendLine(e.Data); } };
            }
            if (RedirectStandardError)
            {
                _process!.ErrorDataReceived += (_, e) => { if (e.Data != null) { _errorBuilder.AppendLine(e.Data); } };
            }

            lock (_lock)
            {
                _process!.Start();
            }

            if (RedirectStandardOutput)
            {
                _process!.BeginOutputReadLine();

            }
            if (RedirectStandardError)
            {
                _process!.BeginErrorReadLine();
            }

            await _process!.WaitForExitAsync();

            ExitCode = _process!.ExitCode;
            Output = _outputBuilder.ToString().Trim();
            Error = _errorBuilder.ToString().Trim();
        }

        public void Stop()
        {
            lock (_lock)
            {
                if (_process is not null && !_process.HasExited)
                {
                    _process.Kill();
                }
            }
        }

        public async Task WaitForStop()
        {
            await _process!.WaitForExitAsync();
        }

        private void CreateProcess()
        {
            var processStartInfo = new ProcessStartInfo
            {
                FileName = BinName,
                Arguments = Arguments,
                RedirectStandardOutput = RedirectStandardOutput,
                RedirectStandardError = RedirectStandardError,
                UseShellExecute = UseShellExecute,
                CreateNoWindow = CreateNoWindow
            };

            _process = new Process { StartInfo = processStartInfo };
            _process.Exited += OnExit;

        }

        protected void OnExit(object? sender, EventArgs e)
        {
            ExitCode = _process!.ExitCode;
            Output = _outputBuilder.ToString().Trim();
            Error = _errorBuilder.ToString().Trim();
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
                    _process = null;
                }
            }
        }
        private bool _isDisposed = false;

        public string BinName { get; set; } = binName;
        public string Arguments { get; set; } = arguments;
        public bool UseShellExecute { get; set; } = false;
        public bool CreateNoWindow { get; set; } = false;
        public bool RedirectStandardOutput { get; set; } = true;
        public bool RedirectStandardError { get; set; } = true;
        public int ExitCode { get; set; } = -1;
        public string Output { get; set; } = string.Empty;
        public string Error { get; set; } = string.Empty;

        private Process? _process;
        private readonly object _lock = new();

        StringBuilder _outputBuilder = new();
        StringBuilder _errorBuilder = new();
    }
}

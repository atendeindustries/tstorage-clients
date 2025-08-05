# TStorage Database C# Client

A dedicated C# client for communication with TStorage database.
Built to simplify integration of TStorage with .NET-based projects, enabling developers to communicate with the database efficiently and with minimal setup.


## Table of Contents

- [Features](#features)  
- [Requirements](#requirements)  
- [Installation](#installation)  
- [Usage](#usage)  
- [Project Structure](#project-structure)
- [Documentation](#documentation)
- [License](#license)


## Features

- Generic and self-contained `Channel<T>` module for simple communication and configuration.
- Supports TStorage's `GET`, `GET_ACQ`, `GET_STREAM`, `PUT` and `PUTA` commands.
- Allows creating custom serialization via `PayloadType<T>` interface.
- Provides reliable methods to read data from TStorage with built-in error handling.
- Ensures all operations are safe and exceptions are properly caught and managed.

Supported requests:
 - `PUT` - add new records to TStorage where acq value is provided by TStorage.
 - `PUTA` - add new records to TStorage where acq value is provided by a user.
 - `GET_ACQ` - get last acq timestamp.
 - `GET` - get records.
 - `GET_STREAM` - get records in batches. Usable when records would use more memory than available on the system.


## Requirements

- .NET SDK 8.0 or higher.
- Compatible operating systems systems Linux, Windows and macOS.
- No external dependencies required.


## Installation

Since this library is not yet published as a NuGet package, you can use it in your project by following the steps below:

1. Clone the repository.
```bash
git clone https://github.com/...
```

2. Add the library project to your solution.
```bash
dotnet sln add path/to/library/TStorage.csproj
```

3. Add the library project reference to your project.
```bash
dotnet add reference path/to/library/TStorage.csproj
```

4. Build the solution.
```bash
dotnet build
```


## Usage

Correct integration and example usage of the library can be checked in the example project located at: `ExampleApp`.


## Project Structure
```
├── csharp.sln
├── ExampleApp
│   ├── ExampleApp.csproj
│   ├── Helper.cs
│   ├── Main.cs
├── LICENSE
├── README.md
├── TStorage
│   ├── Doxyfile
│   ├── Interfaces/
│   ├── Main/
│   ├── TStorage.csproj
│   └── Utilities/
└── TStorage.Tests
    ├── FunctionalTests/
    ├── GlobalUsings.cs
    ├── TStorage.Tests.csproj
    └── UnitTests/
```


## Documentation

Documentation is generated using [Doxygen](https://www.doxygen.nl/), based on XML-style comments in the source code.

### Generating Documentation

1. Install Doxygen.
2. Run doxygen in TStorage project.
```bash
doxygen Doxyfile
```
3. The generated documentation will be located in the `TStorage/docs/html/index.html`


## License

Apache License Version 2.0, January 2004 - see the [`LICENSE`](LICENSE) file for details.

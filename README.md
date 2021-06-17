<p><img align="right" width="130"src="https://github.com/Becavalier/TWVM/blob/master/assets/brand-300x300.png?raw=true"></p>

# TWVM

#### *TWVM is now under construction privately, will update the source code and documentations ASAP.*

<img alt="GitHub Actions status" src="https://github.com/Becavalier/TWVM/workflows/Build CI/badge.svg">

> A tiny, lightweight and efficient WebAssembly virtual machine.

## Getting Started

### Status

Not production-ready yet!

### Compile

Please install the following softwares before compilation:

* [cmake v3.11 (or above)](https://cmake.org/install/)
* [nodejs v12.13.0 LTS (or above)](https://nodejs.org/en/download/)

Then, run the following command to compile:

```
npm run build
```

### Others

* **Code Lint**:

We use "*cpplint*" to check the code style, you can install it by follow command:

`pip install cpplint`

And lint the source code by:

`npm run lint`

* **Memory Check**:

Install "*valgrind*" on MacOS according to the following articles first:

*[How to Install Valgrind on macOS High Sierra](https://www.gungorbudak.com/blog/2018/04/28/how-to-install-valgrind-on-macos-high-sierra/)*

*[Valgrind on macOs 10.14/10.15](https://github.com/sowson/valgrind)*


And then, run the below command to check the memory leak of the program:

`npm run memcheck`

## Roadmap

- [x] Architecture refactoring.
- [x] Run simple fibonacci function successfully.
- [x] Full Wasm spec version 1.0 support.
- [ ] Pass all testsuites.
- [ ] Simple DCT optimization.


## Copyright and License

Licensed under the MIT License;

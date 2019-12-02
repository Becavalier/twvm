<p align="center"><img width="130"src="https://github.com/Becavalier/TWVM/blob/master/arts/brand-300x300.png?raw=true"></p>

<img alt="GitHub Actions status" src="https://github.com/Becavalier/TWVM/workflows/Build CI/badge.svg">

> A tiny, lightweight and efficient WebAssembly virtual machine.

## Getting Started

### Status

Not production-ready yet, feel free to keep tracking the project status by clicking "Watch".

### Compile

If you want to compile and use this project, please install the following softwares in advance:

* [cmake v3.11 (or above)](https://cmake.org/install/)
* [nodejs v12.13.0 LTS (or above)](https://nodejs.org/en/download/)

then, run the following command to compile:

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


Then run the following command to detect the memory leak of the binary version program:

`npm run memcheck`

## Roadmap

- [x] Basic Wasm interpreter on fibonacci;
- [ ] Full Wasm MVP proposals support;
- [ ] Stage based full-path type validation;
- [ ] JIT engine based optimization;
- [ ] Experimental SIMD instructions;
- [ ] Basic WASI standard libraries;


## Copyright and License

Licensed under the MIT License;

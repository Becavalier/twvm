# TWVM

[![experimental](http://badges.github.io/stability-badges/dist/experimental.svg)](http://github.com/badges/stability-badges)

> A tiny and efficient WebAssembly virtual machine.

## Getting Started

### Compile

If you want to compile and use this project, please install the following software in advance:

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

Install "*valgrind*" on MacOS according to the following article first:

*[How to Install Valgrind on macOS High Sierra](https://www.gungorbudak.com/blog/2018/04/28/how-to-install-valgrind-on-macos-high-sierra/)*


Then run the following command to detect the memory leak of the binary version program:

`npm run memcheck`

## Roadmap

- [ ] Support basic Wasm functionalities;
- [ ] Support full Wasm MVP proposals;
- [ ] Support lightweight validation;
- [ ] Support thread independant execution;
- [ ] Support experimental SIMD;
- [ ] Support WASI;


## Copyright and License

Licensed under the MIT License;

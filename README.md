<p><img align="right" width="130"src="https://github.com/Becavalier/TWVM/blob/master/assets/brand-300x300.png?raw=true"></p>

# TWVM

#### *TWVM is still under continuously construction, not production-ready yet!*

<img alt="GitHub Actions status" src="https://github.com/Becavalier/TWVM/workflows/Build CI/badge.svg">

> A tiny, lightweight, and efficient WebAssembly virtual machine.

## Getting Started

### Compile & Install

Please install the following softwares beforehand:

* [cmake v3.11 (or above)](https://cmake.org/install/)
* [nodejs v12.13.0 LTS (or above)](https://nodejs.org/en/download/)

Then, run the following command to compile and install:

```bash
npm run build
```

### How to use?

You can use the below command to invoke an **exported function** from a WebAssemly binary module:

```bash
# Invoke function `fib` with value 10.
twvm ./tests/modules/fibonacci.wasm -i=fib,10  # 55.
```

For further information, please run the below command for help:

```bash
twvm -h
```

### Other Information

* **Limitations**:

Currently, TWVM can only run standard WebAssembly binary modules with the instructions in MVP standard. Also, please make sure the module doesn't require any import objects (.e.g *memory*, *function*, *table*, and etc).

* **Memory Check**:

Install "*valgrind*" on MacOS according to the following articles first:

*[How to Install Valgrind on macOS High Sierra](https://www.gungorbudak.com/blog/2018/04/28/how-to-install-valgrind-on-macos-high-sierra/)*

*[Valgrind on macOS 10.14/10.15](https://github.com/sowson/valgrind)*


And then, run the below command to check the memory leak of the program:

`npm run memcheck`

## Roadmap

- [x] Architecture refactoring.
- [x] Run simple fibonacci function successfully.
- [x] Full Wasm spec version 1.0 support.
- [x] Pass all basic testcases.
- [ ] DCT optimization.
- [ ] WAT support.


## Copyright and License

Licensed under the MIT License.

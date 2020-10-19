# matsuyoshi30's mini C Compiler

![ci](https://github.com/matsuyoshi30/mmcc/workflows/ci/badge.svg)

Based on [compilerbook](https://www.sigbus.info/compilerbook), and using Docker.

```
$ docker build -t compilerbook .

$ ./docker-run
```

`mmcc` can now build itself, except the C preprocessor.

## Build

```
$ make             # build the first gen compiler
$ make mmcc-stage2 # build the second gen compiler
```

## Test

```
$ make test        # test the first gen compiler
$ make test-stage2 # test the second gen compiler
```

## License

[MIT](./LICENSE)

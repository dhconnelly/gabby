# gabby

an inference server from scratch in c++

## status

- [x] multithreaded web server
- [x] openai-compatible chat completion api
- [x] test framework
- [ ] json parser
- [ ] model and weights loader
- [ ] tokenizer
- [ ] llama3.2 in cuda
- [ ] profiling and optimization
- [ ] docker image

## prerequisites

- x86_64
- ubuntu 24.04
- gcc 13.2
- cmake 3.22
- nvidia gpu with at least 3gb vram
- nvidia cuda toolkit: https://developer.nvidia.com/cuda-toolkit

## getting started

    cmake -S . -B build     # prepare build
    cmake --build build     # build everything
    ./build/gabby_test      # run tests
    ./build/gabby --info --port 8080 --workers 7 &

this will start the server running on localhost at port 8080 with
seven worker threads and INFO level logging. note that it supports
graceful shutdown via SIGINT and SIGTERM.

while it's runnning, you can call the chat completion api:

    curl -v localhost:8080/v1/chat/completion

# gabby

an inference server from scratch in c++

## status

- [x] multithreaded web server
- [x] test framework
- [x] json parser
- [x] openai-compatible chat completion api
- [ ] model and weights loader
- [ ] tokenizer
- [ ] llama3.2 in cuda
- [ ] profiling and optimization
- [ ] docker image

other improvements:

- [ ] backpressure w/http 529
- [ ] streaming w/server-side events
- [ ] add /statusz with metrics etc.

## prerequisites

- x86_64
- ubuntu 24.04
- gcc 13.2
- cmake 3.22
- nvidia gpu with at least 3gb vram
- nvidia cuda toolkit: https://developer.nvidia.com/cuda-toolkit
- Llama3.2-1B-Instruct weights from Meta: 
    + install the `llama` tool ([guide](https://llama-stack.readthedocs.io/en/latest/references/llama_cli_reference/index.html)): `pip install llama-stack`
    + fill out the access form [here](https://www.llama.com/llama-downloads/) and get your private download url
    + run `llama download --source meta --model-id Llama3.2-1B-Instruct --meta-url 'META_URL'` with `META_URL` replaced by your private download url you got in the previous step
    + this should download the model parameters and weights to `$HOME/.llama/checkpoints/Llama3.2-1B-Instruct/`. `gabby` will assume this is where the weights are located; if you have a different location, specify this with the env var `MODEL_DIR`.
    + note: this isn't using quantization at the moment :)

## getting started

```bash
cmake -S . -B build     # prepare build
cmake --build build     # build everything
./build/gabby_test      # run tests
./build/gabby --info --port 8080 --workers 7
```

this will start the server running on localhost at port 8080 with
seven worker threads and INFO level logging. note that it supports
graceful shutdown via SIGINT and SIGTERM.

while it's runnning, you can call the chat completion api:

```bash
curl localhost:8080/v1/chat/completions -d '{
    "model": "gabby-1",
    "messages": [{
        "role": "system",
        "content": "You are a helpful assistant."
    },{
        "role": "user",
        "content": "Hello!"
    }]
}'
```

or use an openai-compatible chat app (like boltai for mac).

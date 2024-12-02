# gabby

a zero-dependency llama 3 inference server from scratch in c++

currently under active development, see status below

## status

- [x] multithreaded web server
- [x] test framework
- [x] json parser
- [x] openai-compatible chat completion api
- [x] parse safetensors, params, tokenizer configs
- [ ] tokenizer
- [ ] llama3.2 in cuda
- [ ] profiling and optimization
- [ ] docker image

other improvements:

- [ ] backpressure w/http 529
- [ ] streaming w/server-side events
- [ ] add /statusz with metrics etc.
- [ ] revisit concurrency

## prerequisites

- x86_64
- ubuntu 24.04
- gcc 13.2
- cmake 3.22
- nvidia gpu with at least 3gb vram
- nvidia cuda toolkit: https://developer.nvidia.com/cuda-toolkit
- Llama3.2-1B-Instruct SafeTensors from Hugging Face:
    + Create a Hugging Face account at https://huggingface.co/
    + Agree to the license agreement at https://huggingface.co/meta-llama/Llama-3.2-1B-Instruct
    + Wait for approval; check status at https://huggingface.co/settings/gated-repos
    + Install the Hugging Face CLI: `pip install -U "huggingface_hub[cli]"`
    + Log in via the CLI: `huggingface-cli login`. You'll have to paste a token (or create one with at least read access if it doesn't exist) at https://huggingface.co/settings/tokens
    + Download the repository: `huggingface-cli download meta-llama/Llama-3.2-1B-Instruct`
    + We assume the files are downloaded and stored using the default Hugging Face cache, i.e. at `$HOME/.cache/huggingface/hub/models--meta-llama--Llama-3.2-1B-Instruct/snapshots/$SHA/`. The server will look for this directory (using the first `SHA` encountered) at startup; if you want to override this path, use the flag `--model-dir`.

## getting started

```bash
cmake -S . -B build     # prepare build
cmake --build build     # build everything
./build/gabby_test      # run tests
./build/gabby --debug --port 8080 --workers 7
```

this will start the server running on localhost at port 8080 with
seven worker threads and DEBUG level logging. note that it supports
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

## license

MIT

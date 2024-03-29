# hangman-rc

Hangman implementation in a multi-client server context, using C++.

## Compilation

In order to compile the project, `c++17` (available since GCC 9) is required.
There are two ways to compile the project:

- Using `make prod`, which will generate the server's binary according to the project
  statement's specifications - the word-list reading order is **randomized**.
- Using `make`, the project version utilized to run tests and, in a general
  manner, debug the server. The word-list reading order is
  **sequential** (in order for the tests to be deterministic).

On compilation, `build/` folders will be generated inside `client/`, `server/`
and `lib/`, containing the generated object files.

There's also the `make clean` command, which will remove the generated binaries
and asset folders created during the server's execution.

Note that, regarding the `server/assets/hints/` folder, the images are not included
in the compressed archive (as suggested by the teacher). **All hints should, however,
be placed in this folder**.

## Usage

After compiling the project, the **server** can be executed by running its binary,
as specified in the project's statement. The following command should be run
from the project's root directory:

```bash
server/bin/GS server/assets/word_eng.txt [-v] [-p <port>]
```

Operating in verbose mode (with the `-v` flag) will print the server's log (including the peer's IP address and port, plus the message sent by them) to the standard output. Moreover, the server's port can be specified with the `-p` flag. If no port is specified, the default port `58045` will be used.

The **client** can be executed in a very similar manner (once again, run the
command from the project's root directory):

```bash
client/bin/player [-p <port>] [-n <host>]
```

Here, the host's port and IP(v4) address/hostname can be specified with the `-p` and `-n`, respectively. If no port is specified, the default port `58045` will be used. If no IP address is specified, the default address `localhost` will be used.

## Project structure

The project is structured as follows:

- `client/`: contains the client's source code, split into `src/` and `include/`
  folders (the latter containing the client's header files, while the former contains
  the client's source files). The `bin/` folder contains the client's binary, `player`.
  The `assets` folder will hold any temporary files created during the client's execution.

- `server/`: structured in a rather similar manner to the `client`, with an analogous logic
  for the`src`, `include` and `bin` folders. Here, the `assets` folder holds not only
  any temporary files created during the server's execution, but also the word-list
  file, `word_eng.txt`, which is used to generate the game's associated word, and the hint
  files (images), which are, of course, kept between executions (and never deleted via
  `make clean`).

- `lib/`: includes the common source code for both client and server, ranging from
  the GameState class, to socket-handling functions and general-purpose methods.

## Teacher's note

Regarding changes that the lab's teacher may want to do in order to further test the project, altering the timeout's associated waiting time can be done via the `SOCKET_TIMEOUT` constant in `lib/common.h` (which is, by default, set to 5 seconds) - if set to 0 seconds, the timer is deactivated (according to the manpage).

# SHROOM

## Group Members
* Syed Rumman Naser (2405394) - server + client + concurrency
* Shreekant Singh (24051740) - cipher + file transfer

## How to Build
Compile the source files using GCC with the POSIX threads library linked:
* `gcc server.c -o server -pthread`
* `gcc client.c -o client -pthread`

## How to Run
* Server: `./server <port>` (e.g., `./server 8080`)
* Client: `./client <server_ip> <port>` (e.g., `./client 127.0.0.1 8080`)

## Cipher Choice
We chose <XOR / Vigenère / substitution-permutation> because <reason>.
Known weakness: <weakness details>.

## Design Notes
* **Encryption:** This application uses hop-by-hop encryption rather than true end-to-end encryption[cite: 1]. The server temporarily decrypts incoming ciphertexts into memory and re-encrypts them using the target client's key[cite: 1].
* **Concurrency:** The server utilizes a `pthreads` concurrency model[cite: 1]. This isolated thread-per-client approach ensures that if one user abruptly disconnects or sends malformed garbage bytes, the server smoothly cleans up the socket without blocking or crashing other active connections[cite: 1]. 
* **File Transfer:** File transfers have a strict size cap of 1 MB to eliminate the need for chunked or streaming data transfers over the network[cite: 1].

## Known Limitations
* True end-to-end encryption is not supported; clients do not establish shared keys directly with one another[cite: 1].

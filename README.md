# SHROOM

## Group Members
* Syed Rumman Naser (2405394) - server + client + concurrency + registration setup[cite: 8]
* Shreekant Singh (24051740) - cipher + file transfer[cite: 8]

## How to Build
Compile the source files using GCC with the POSIX threads library linked[cite: 8]:
* `gcc server.c cipher.c file_utils.c -o server -pthread`
* `gcc client.c cipher.c file_utils.c -o client -pthread`

## How to Run
* Server: `./server <port>` (e.g., `./server 8080`)[cite: 8]
* Client: `./client <server_ip> <port>` (e.g., `./client 127.0.0.1 8080`)[cite: 8]

## Cipher Choice
We chose a **Repeating-Key XOR Cipher** because it uses the exact same mathematical operation to symmetrically encrypt and decrypt data without relying on external libraries.
* **Known weakness:** It is highly vulnerable to frequency analysis and known-plaintext attacks. If the key length is shorter than the message, the repeating pattern easily exposes the key.

## Design Notes
* **Encryption:** This application uses hop-by-hop encryption rather than true end-to-end encryption[cite: 8]. The server temporarily decrypts incoming ciphertexts into memory using the sender's key and re-encrypts them using the target client's key before routing[cite: 8].
* **Registration Paradox:** To allow the server to securely store symmetric keys, the initial `REGISTER` command is transmitted in plaintext. All subsequent `SEND TO` and `SENDFILE TO` communication is encrypted.
* **Concurrency:** The server utilizes a `pthreads` concurrency model[cite: 8]. This isolated thread-per-client approach ensures that if one user abruptly disconnects or sends malformed garbage bytes, the server smoothly cleans up the socket without blocking or crashing other active connections[cite: 8]. 
* **File Transfer:** File transfers have a strict size cap of 1 MB to eliminate the need for chunked or streaming data transfers over the network, and the application strictly validates `.txt` extensions[cite: 8].

## Known Limitations
* True end-to-end encryption is not supported; clients do not establish shared keys directly with one another[cite: 8].

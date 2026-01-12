# Chip-8 Emulator

This is my personal Chip-8 emulator project written in C++.  
I am currently learning C++ and decided to build this emulator from scratch.  
The project uses **C++ 17 (GCC 15.2.0)** and **SDL3** for graphics and input.

---

## Installation

Right now the provided installation only works on MacOS system, I figured Ubuntu/Linux user can find the away to install g++ , make, and sdl3 on the internet. 

Sorry windows users, it will get buggy.

Install Homebrew (if not already installed):

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Install SDL3:


```bash
brew install sdl3
```


Install g++ and make (if not already installed):


```bash
brew install gcc make
```


Clone github

```bash
git clone https://github.com/nhantran1711/chip8emulator.git
cd chip8emulator
```


## Running application

Making sure that you are on the **same folder** as ./chip8, or else it won't work

Making sure that when run _make_, it shouldn't scream any issue, if it does, please make an issue ticket

otherwise, run ./chip8

```bash
make
./chip8
```



---

## Documentation & References

A huge thanks to the following resources that helped me a lot:

- **SDL Wiki (SDL3 Documentation)**  
  [https://wiki.libsdl.org/SDL3/FrontPage](https://wiki.libsdl.org/SDL3/FrontPage)

- **How to Write an Emulator (Chip-8 Interpreter)**  
  [https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/](https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/)

- **Codeslinger – Chip-8 Projects**  
  [http://www.codeslinger.co.uk/pages/projects/chip8.html](http://www.codeslinger.co.uk/pages/projects/chip8.html)

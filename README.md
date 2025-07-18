# Rayboy-Emulator
🕹️ A game boy emulator being developed in the C programming language and Raylib. Once it's finished it's planned to support Game Boy, Game Boy Color and Game Boy Advance.

<img src="https://github.com/user-attachments/assets/13f67c2a-4452-4f55-b4e5-52102cf1ca9f" width="1000"/>


## Achievements

| Feature                     | Status   |
|-----------------------------|----------|
| Input handling              | ✅       |
| Flags handling              | ✅       |
| ROM loading                 | ✅       |
| CPU (512/512 instructions)  | 🚧(Debugging)|
| Graphics                    | ❌       |
| Audio                       | ❌       |
| Cheats & Settings           | ❌       |
| Game Boy Color              | ❌       |

---

# Sources
I would like to thank this sources that were and are a big help during the development of Rayboy.
- https://gbdev.io/pandocs
- https://gbdev.io/gb-opcodes/optables/#standard
- https://rgbds.gbdev.io/docs/v0.9.3/gbz80.7

## Build Instructions

You need to have [Raylib](https://www.raylib.com/) installed. 
```bash
sudo apt update
sudo apt install libraylib-dev
```
Then, compile using:

```bash
bash build.bash
